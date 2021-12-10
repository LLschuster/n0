

namespace ng
{
    /*
     We need to make this instances static because we only need one of this things at any given time
     and not use constructors because static things get allocated before the main function runs, and constructors get executed
     in some random order we can't predict.
    */
    class Logger : public EngineSystem
    {
    public:
        FILE *logFile;
        static Logger *getInstance()
        {
            // this var gets created and initialize only on the first getInstance call
            static Logger *instance = nullptr;
            if (!instance)
            {
                instance = new Logger(); // todo get memory from memorypool
            }
            return instance;
        }

        uint32 startup()
        {
            time_t rawTime;
            struct tm *timeInfo;
            time(&rawTime);
            timeInfo = localtime(&rawTime);

            char fileName[50];
            strftime(fileName, sizeof(fileName), "%H-%M-%S-%F.txt", timeInfo);

            char filePath[150];
            strcat(filePath, "logs/");
            strcat(filePath, fileName);

            logFile = fopen(filePath, "a");
            if (logFile == NULL)
            {
                fputs("ERROR while opening log file\n", stderr);
                perror("Could not open logFile\n");
                return 1;
            }
            return 0;
        }

        void shutdown()
        {
            fclose(logFile);
        }

        void writeLog(const char *fmt, ...)
        {
            if (logFile == NULL)
            {
                printf("Log file is not open, did you forget to initialize it?");
                return;
            }
            char buffer[256];
            va_list args;
            va_start(args, fmt);
            uint32 total = vsprintf(buffer, fmt, args);

            if (fwrite(buffer, sizeof(char), total, logFile) != total)
            {
                fputs("Could not write the log\n", stderr);
                perror("Could not write the log\n");
            }

            fflush(logFile);
            va_end(args);
        }

    private:
        Logger(){};
        ~Logger(){};
    };


    class ResourceManager : public EngineSystem
    {
        ResourceManager() {}
        ~ResourceManager() {}
        uint32 startup()
        {
            return 0;
        }
        void shutdown() {}
    };

    class GameManager : public EngineSystem
    {
    public:
        bool isRunning;
        GameManager() {}
        ~GameManager() {}

        static GameManager *getInstance()
        {
            static GameManager *instance = nullptr;
            if (!instance)
            {
                instance = new GameManager();
            }
            return instance;
        }
        uint32 startup()
        {
            isRunning = true;
            return 0;
        }
        void shutdown()
        {
        }

        void terminateGame()
        {
            isRunning = false;
        }
    };

    struct Frame
    {
        float width;
        float height;
        std::string src;
        void operator=(Frame &other)
        {
            width = other.width;
            height = other.height;
            src = std::string(other.src);
        }
    };

    struct Sprite
    {
        Frame frames[MAX_FRAMES_PER_SPRITE];
        std::string label;
        uint32 width;
        uint32 height;
        uint32 frameCount;
        uint32 slowdown;
        char *color; //TODO add color type
    };

    class Entity
    {
    public:
        std::string type;
        Vector2d pos;
        Vector2d velocity;
        Sprite sprite;
        Entity(std::string _type, Vector2d _pos, Vector2d _velocity) : type(_type), pos(_pos), velocity(_velocity){};
        Entity(){};
    };

    struct WorldEntityList
    {
        Entity entities[100000];
        Entity solidEntities[100000];
        uint32 entityIndex;
        uint32 solidEntityIndex;
    };

    struct GameMemory
    {
        WorldEntityList worldEntityList;
        uint8 workingMemory[MAX_WORKING_MEMORY];
        uint8 *workingMemoryCur = nullptr;
        Entity *hero = nullptr;
        bool isInit = false;
        size_t size;
    };

    struct QueryEntitiesResult
    {
        Entity **entities;
        uint32 count;
        uint8 *freeTag;
    };

    GameMemory *gameMemory = {};

    QueryEntitiesResult getEntitiesByType(const std::string &type)
    {
        QueryEntitiesResult result = {};
        if (!gameMemory->isInit)
        {
            perror("World entity list pointer is not set");
            return result;
        }

        if (gameMemory->workingMemoryCur + sizeof(Entity *) * gameMemory->worldEntityList.entityIndex > gameMemory->workingMemory + MAX_WORKING_MEMORY - 1)
        {
            printf("Your are out of memory idiot\n");
        }

        result.freeTag = gameMemory->workingMemoryCur; // use this to free the memory

        Entity **filteredEntities = (Entity **)gameMemory->workingMemoryCur;
        uint32 count = 0;
        for (int i = 0; i < gameMemory->worldEntityList.entityIndex; i++)
        {
            if (strcmp(gameMemory->worldEntityList.entities[i].type.c_str(), type.c_str()) == 0)
            {
                filteredEntities[count] = &(gameMemory->worldEntityList.entities[i]);
                count++;
            }
        }

        gameMemory->workingMemoryCur += count * sizeof(Entity *);

        result.count = count;
        result.entities = filteredEntities;

        return result;
    }

    void freeWorkingMemory(uint8 *freeTag)
    {
        if (gameMemory->workingMemoryCur < freeTag)
        {
            printf("Memory already freed, \n");
            return;
        }

        for (uint8 *i = gameMemory->workingMemoryCur; i != freeTag; i--)
        {
            *i = 0;
        }

        gameMemory->workingMemoryCur = freeTag;
    }

    uint32 addEntityToWorld(const std::string &type, Vector2d pos, Sprite sprite, Vector2d velocity = Vector2d(10.0f, 10.0f), bool isSolid = true)
    {
        if (!gameMemory->isInit)
        {
            return 0;
        }

        uint32 currentIndex = gameMemory->worldEntityList.entityIndex;
        Entity *current = &(gameMemory->worldEntityList.entities[currentIndex]);
        current->pos = pos;
        current->type = std::string(type);
        current->sprite = sprite;
        current->velocity = velocity;
        gameMemory->worldEntityList.entityIndex++;

        if (isSolid)
        {
            uint32 solidIndex = gameMemory->worldEntityList.solidEntityIndex;
            gameMemory->worldEntityList.solidEntities[solidIndex] = *current;
            gameMemory->worldEntityList.solidEntityIndex++;
        }

        return currentIndex;
    }

    void addHeroToWorld(Vector2d pos, Sprite sprite)
    {
        uint32 heroIndex = addEntityToWorld("hero", pos, sprite, Vector2d(20.0f, 20.0f));
        gameMemory->hero = &(gameMemory->worldEntityList.entities[heroIndex]);
    }

    void launchMissile(Entity *missile)
    {
        Entity *hero = gameMemory->hero;
        missile->pos.x = hero->pos.x;
        missile->pos.y = hero->pos.y;
        missile->type = EntityTypes::missile;
    }

    void movePlayer(MoveDirection move)
    {
        Entity *hero = gameMemory->hero;
        switch (move)
        {
        case MoveDirection::UP:
            if ((hero->pos.y - (hero->sprite.height / 2) * FONT_SIZE_PIXEL) <= 0)
                return;
            hero->pos.y -= hero->velocity.y;
            break;
        case MoveDirection::DOWN:
            if ((hero->pos.y + (hero->sprite.height / 2) * FONT_SIZE_PIXEL) >= SCREEN_HEIGHT)
                return;
            hero->pos.y += hero->velocity.y;
            break;
        case MoveDirection::LEFT:
            if ((hero->pos.x - (hero->sprite.width / 2) * FONT_SIZE_PIXEL) <= 0)
                return;
            hero->pos.x -= hero->velocity.x;
            break;
        case MoveDirection::RIGHT:
            if ((hero->pos.x + (hero->sprite.width / 2) * FONT_SIZE_PIXEL) >= SCREEN_WIDTH)
                return;
            hero->pos.x += hero->velocity.x;
            break;
        }
    }

    void moveEnemies()
    {
        QueryEntitiesResult enemyList = getEntitiesByType("enemy");
        for (int i = 0; i < enemyList.count; i++)
        {
            enemyList.entities[i]->pos.x -= enemyList.entities[i]->velocity.x;
            if (enemyList.entities[i]->pos.x < -150)
            {
                enemyList.entities[i]->type = EntityTypes::enemyInactive;
            }
        }
        freeWorkingMemory(enemyList.freeTag);
    }

    void respawnEnemies()
    {

        QueryEntitiesResult enemyList = getEntitiesByType(EntityTypes::enemyInactive);

        ng::Randomizer xGenerator = {};
        ng::Randomizer yGenerator = {};
        xGenerator = ng::getRandomReal((float)SCREEN_WIDTH * 0.3, (float)SCREEN_WIDTH);
        yGenerator = ng::getRandomReal(0.0f, (float)SCREEN_HEIGHT * 0.5f);

        for (int i = 0; i < enemyList.count / 2; i++)
        {

            float x = xGenerator.distribution.realD(xGenerator.generator);
            float y = yGenerator.distribution.realD(yGenerator.generator);
            float newXLow = x + enemyList.entities[i]->sprite.width * ng::FONT_SIZE_PIXEL;
            xGenerator = ng::getRandomReal(newXLow, newXLow * 1.2);

            enemyList.entities[i]->pos.x = x;
            enemyList.entities[i]->pos.y = y;
            enemyList.entities[i]->type = EntityTypes::enemy;
        }

        xGenerator = ng::getRandomReal((float)SCREEN_WIDTH * 0.3, (float)SCREEN_WIDTH);
        yGenerator = ng::getRandomReal((float)SCREEN_HEIGHT * 0.5f, (float)SCREEN_HEIGHT);
        for (int i = enemyList.count / 2; i < enemyList.count; i++)
        {
            float x = xGenerator.distribution.realD(xGenerator.generator);
            float y = yGenerator.distribution.realD(yGenerator.generator);
            float newXLow = x + enemyList.entities[i]->sprite.width * ng::FONT_SIZE_PIXEL;
            xGenerator = ng::getRandomReal(newXLow, newXLow * 1.2);

            enemyList.entities[i]->pos.x = x;
            enemyList.entities[i]->pos.y = y;
            enemyList.entities[i]->type = EntityTypes::enemy;
        }

        freeWorkingMemory(enemyList.freeTag);
    }

    void
    drawFrames(Renderer *renderer, Entity *entities, uint32 count)
    {
        char line[50];
        for (uint32 i = 0; i < count; i++)
        {
            // offset because we draw with position as the middle point
            Entity current = entities[i];
            Frame frameToDraw = current.sprite.frames[0];
            Vector2d posToDraw = current.pos;

            float offsetX = frameToDraw.width / 2;
            float offsetY = frameToDraw.height / 2;

            for (
                uint32 y = 0; y < frameToDraw.height; y++)
            {
                for (uint32 x = 0; x < frameToDraw.width; x++)
                {
                    line[x] = frameToDraw.src[y * frameToDraw.width + x];
                }
                line[(uint32)frameToDraw.width] = '\0';
                renderer->drawText(Vector2d(posToDraw.x - offsetX, posToDraw.y - ((offsetY - y) * FONT_SIZE_PIXEL)), FONT_SIZE_PIXEL, line);
            }
        }
    }

    uint32
    getEntityWidthOnDraw(Renderer *renderer, Entity *entity)
    {
        char line[50];
        // offset because we draw with position as the middle point
        Frame frameToDraw = entity->sprite.frames[0];
        uint32 maxWidth = 0;

        for (
            uint32 y = 0; y < frameToDraw.height; y++)
        {
            for (uint32 x = 0; x < frameToDraw.width; x++)
            {
                line[x] = frameToDraw.src[y * frameToDraw.width + x];
            }
            line[(uint32)frameToDraw.width] = '\0';
            Size size = renderer->getDrawRect(line);
            maxWidth = std::max(maxWidth, (uint32)size.w);
        }
        return maxWidth;
    }

    void drawColliderBox(Renderer *renderer, Entity *entities, uint32 count, int width)
    {
        for (uint32 i = 0; i < count; i++)
        {
            Entity current = entities[i];
            float x = current.pos.x; //- ((current.sprite.width / 2) * FONT_SIZE_PIXEL);
            float y = current.pos.y - (ceil((float)(current.sprite.height) / 2) * FONT_SIZE_PIXEL);

            renderer->drawRect(x, y, width, current.sprite.height * FONT_SIZE_PIXEL);
        }
    }

    namespace SpriteFileKeys
    {
        const char frames[] = "frames";
        const char width[] = "width";
        const char height[] = "height";
        const char color[] = "color";
        const char slowdown[] = "slowdown";
    };

    void extractHeaderValue(char *line, Sprite *sprite)
    {
        char *value;
        if ((value = strstr(line, SpriteFileKeys::frames)) != NULL)
        {
            sprite->frameCount = atoi(value + sizeof(SpriteFileKeys::frames));
            return;
        }
        if ((value = strstr(line, SpriteFileKeys::width)) != NULL)
        {
            sprite->width = atoi(value + sizeof(SpriteFileKeys::width));
            return;
        }
        if ((value = strstr(line, SpriteFileKeys::height)) != NULL)
        {
            sprite->height = atoi(value + sizeof(SpriteFileKeys::height));
            return;
        }
        if ((value = strstr(line, SpriteFileKeys::color)) != NULL)
        {
            sprite->color = value + sizeof(SpriteFileKeys::color);
            return;
        }
        if ((value = strstr(line, SpriteFileKeys::slowdown)) != NULL)
        {
            sprite->slowdown = atoi(value + sizeof(SpriteFileKeys::slowdown));
            return;
        }
    }

    void addFrameToSprite(std::string &source, Sprite *sprite, uint32 count)
    {
        Frame frame;
        frame.src = source;
        frame.height = sprite->height;
        frame.width = sprite->width;
        sprite->frames[count] = frame;
    }

    Sprite loadSprite(char *pathname, char *label)
    {
        char filePath[200] = "";
        char line[256];
        uint8 readFlag = 99; // 0 for header, 1 for body, 2 for footer
        strcat(filePath, DATA_PATH);
        strcat(filePath, pathname);
        FILE *file = fopen(filePath, "r");

        if (file == NULL)
        {
            perror("Could not open Sprite file");
            printf("%s", filePath);
            exit(9);
        }

        Sprite sprite;
        sprite.label = label;

        uint32 frameCount = 0;
        std::string frameSrc = "";
        while (fgets(line, sizeof(line), file) != NULL)
        {
            if (readFlag == 0)
            {
                extractHeaderValue(line, &sprite);
                if (strstr(line, "</HEADER>"))
                {
                    readFlag = 99;
                }
                continue;
            }
            if (readFlag == 1)
            {

                if (strstr(line, "end"))
                {
                    addFrameToSprite(frameSrc, &sprite, frameCount);
                    frameCount++;
                    frameSrc = "";
                }
                else
                {
                    for (size_t s = strlen(line) - 1; s < sprite.width; s++)
                    {
                        line[s] = ' ';
                    }
                    line[sprite.width] = '\0';
                    frameSrc += line;
                }
                if (strstr(line, "</BODY>"))
                {
                    readFlag = 99;
                }
                continue;
            }
            if (readFlag == 2)
            {
                if (strstr(line, "</FOOTER>"))
                {
                    readFlag = 99;
                }
                continue;
            }
            if (strstr(line, "<HEADER>"))
            {
                readFlag = 0;
            }
            if (strstr(line, "<BODY>"))
            {
                readFlag = 1;
            }
            if (strstr(line, "<FOOTER>"))
            {
                readFlag = 2;
            }
        }

        return sprite;
    }

    bool areEntitiesOverlapping(Entity *one, Entity *two)
    {
        bool isXOverlapping = false;
        bool isYOverlapping = false;
        float oneRightBound = one->pos.x + (one->sprite.width * FONT_SIZE_PIXEL) / 2;
        float oneLeftBound = one->pos.x - (one->sprite.width * FONT_SIZE_PIXEL) / 2;
        float oneBottomBound = one->pos.y + (one->sprite.height * FONT_SIZE_PIXEL) / 2;
        float oneTopBound = one->pos.y - (one->sprite.height * FONT_SIZE_PIXEL) / 2;

        float twoRightBound = two->pos.x + (two->sprite.width * FONT_SIZE_PIXEL) / 2;
        float twoLeftBound = two->pos.x - (two->sprite.width * FONT_SIZE_PIXEL) / 2;
        float twoBottomBound = two->pos.y + (two->sprite.height * FONT_SIZE_PIXEL) / 2;
        float twoTopBound = two->pos.y - (two->sprite.height * FONT_SIZE_PIXEL) / 2;

        if (oneRightBound >= twoLeftBound && oneRightBound <= twoRightBound)
        {
            isXOverlapping = true;
        }

        if (oneLeftBound >= twoLeftBound && oneLeftBound <= twoRightBound)
        {
            isXOverlapping = true;
        }

        if (oneTopBound <= twoBottomBound && oneTopBound >= twoTopBound)
        {
            return isYOverlapping = true;
        }

        if (oneBottomBound >= twoTopBound && oneBottomBound <= twoBottomBound)
        {
            return isYOverlapping = true;
        }

        return isXOverlapping && isYOverlapping;
    }
};
/*
 ____ 
\____\ 
*/