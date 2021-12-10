#ifndef ENGINE_H
#define ENGINE_H
namespace ng
{
    static const uint32 MAX_WORKING_MEMORY = megabyte(10);
    static const uint32 MAX_FRAMES_PER_SPRITE = 10;
    static const uint32 FONT_SIZE_PIXEL = 16;
    static const char *DATA_PATH = "..//data//";

    struct Size
    {
        int h;
        int w;
    };

    enum MoveDirection
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
    };

    namespace EntityTypes
    {
        const char hero[] = "hero";
        const char enemy[] = "enemy";
        const char enemyInactive[] = "enemyInactive";
        const char missile[] = "missile";
        const char missileInactive[] = "missileInactive";
    };

    class EngineSystem
    {
        std::string type;
        virtual uint32 startup() { return 0; };
        virtual void shutdown(){};
    };
}

#endif
