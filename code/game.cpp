

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <main.h>
#include <math.h>
#include <algorithm>
#include <stdint.h>
#include <time.h>
#include <stdarg.h>
#include <SDL.h>
#include <SDL_ttf.h>

typedef uint32_t uint32;
typedef uint8_t uint8;

#define megabyte(value) ((value) * (1024) * (1024))
#define lg ng::Logger::getInstance()
#define rd ng::Renderer::getInstance()
#define gm ng::GameManager::getInstance()
#define SCREEN_HEIGHT 780
#define SCREEN_WIDTH 1280

#include "main.h"
#include "engine.h"

#include "utils.cpp"
#include "ng_math.cpp"
#include "renderer.cpp"
#include "engine.cpp"


struct MissileManager
{
    ng::QueryEntitiesResult missileList = {};
    uint32 currentMissile = 0;
    uint32 shouldReloadCount = 100;
} missileManager;

void generateWorld()
{
    ng::Sprite enemySprite = ng::loadSprite("EnemySprite.txt", "enemy");
    ng::Sprite heroSprite = ng::loadSprite("heroSprite.txt", "hero");
    ng::Sprite missileSprite = ng::loadSprite("missileSprite.txt", "missile");

    ng::addHeroToWorld(ng::Vector2d(20, SCREEN_HEIGHT / 2), heroSprite);

    for (int i = 0; i < 20; i++)
    {
        ng::addEntityToWorld(ng::EntityTypes::missileInactive, ng::Vector2d(), missileSprite);
    }

    ng::Randomizer xGenerator = {};
    ng::Randomizer yGenerator = {};
    xGenerator = ng::getRandomReal((float)SCREEN_WIDTH * 0.3, (float)SCREEN_WIDTH * 0.4f);
    yGenerator = ng::getRandomReal(0.0f, (float)SCREEN_HEIGHT * 0.5f);
    for (int i = 0; i < 3; i++)
    {
        float x = xGenerator.distribution.realD(xGenerator.generator);
        float y = yGenerator.distribution.realD(yGenerator.generator);
        float newXLow = x + enemySprite.width * ng::FONT_SIZE_PIXEL;
        xGenerator = ng::getRandomReal(newXLow, newXLow * 1.2);

        ng::addEntityToWorld("enemy", ng::Vector2d(x, y), enemySprite);
    }

    xGenerator = ng::getRandomReal((float)SCREEN_WIDTH * 0.3, (float)SCREEN_WIDTH * 0.4f);
    yGenerator = ng::getRandomReal((float)SCREEN_HEIGHT * 0.5f, (float)SCREEN_HEIGHT);
    for (int i = 0; i < 3; i++)
    {
        float x = xGenerator.distribution.realD(xGenerator.generator);
        float y = yGenerator.distribution.realD(yGenerator.generator);
        float newXLow = x + enemySprite.width * ng::FONT_SIZE_PIXEL;
        xGenerator = ng::getRandomReal(newXLow, newXLow * 1.2);

        ng::addEntityToWorld("enemy", ng::Vector2d(x, y), enemySprite);
    }
}

void handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            if (event.key.state == SDL_PRESSED)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_Q)
                {
                    gm->terminateGame();
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_UP)
                {
                    ng::movePlayer(ng::MoveDirection::UP);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
                {
                    ng::movePlayer(ng::MoveDirection::DOWN);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
                {
                    ng::movePlayer(ng::MoveDirection::LEFT);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                {
                    ng::movePlayer(ng::MoveDirection::RIGHT);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
                {
                    if (missileManager.shouldReloadCount != 100)
                        return;

                    ng::launchMissile(missileManager.missileList.entities[missileManager.currentMissile]);
                    missileManager.currentMissile++;

                    if (missileManager.currentMissile >= 20)
                    {
                        missileManager.shouldReloadCount = 99;
                    }
                }
            }
        }
    }
}

void drawMissiles()
{
    for (int i = 0; i < missileManager.currentMissile; i++)
    {
        if (strcmp(missileManager.missileList.entities[i]->type.c_str(), ng::EntityTypes::missile) == 0)
        {
            ng::Entity *current = missileManager.missileList.entities[i];
            current->pos.x += current->velocity.x;
            if (current->pos.x > SCREEN_WIDTH)
            {
                current->type = ng::EntityTypes::missileInactive;
            }
            ng::drawFrames(rd, current, 1);

            uint32 width = ng::getEntityWidthOnDraw(rd, current);
            ng::drawColliderBox(rd, current, 1, width);
        }
    }
}

//TODO
void destroyPlayer()
{
    printf("you die");
}

void destroyEnemy()
{
}

void checkColissionsFor()
{
    ng::QueryEntitiesResult enemyList = ng::getEntitiesByType("enemy");
    ng::Entity *hero = ng::gameMemory->hero;

    for (int i = 0; i < enemyList.count; i++)
    {
        if (ng::areEntitiesOverlapping(hero, enemyList.entities[i]))
        {
            destroyPlayer();
        }
    }

    for (int i = 0; i < enemyList.count; i++)
    {
        for (int j = 0; j < missileManager.missileList.count; j++)
        {
            if (ng::areEntitiesOverlapping(missileManager.missileList.entities[j], enemyList.entities[i]))
            {
                ng::Entity *missile = missileManager.missileList.entities[j];
                missile->type = ng::EntityTypes::missileInactive;

                ng::Entity *enemy = enemyList.entities[i];
                enemy->type = ng::EntityTypes::enemyInactive;

                //TODO update game score
            }
        }
    }
}

int main(int argc, char **argv)
{
    int initResult = 0;
    initResult += lg->startup();
    initResult += rd->startup();
    initResult += gm->startup();

    if (initResult > 0)
    {
        lg->writeLog("Systems initialization failed");
        exit(initResult);
    }

    size_t memorySize = megabyte(128);
    ng::gameMemory = (ng::GameMemory *)malloc(memorySize);
    if (ng::gameMemory == NULL)
    {
        perror("Could not allocate game memory\n");
    }

    ng::gameMemory->workingMemoryCur = ng::gameMemory->workingMemory;
    ng::gameMemory->size = memorySize;
    ng::gameMemory->isInit = true;

    generateWorld();

    // manage missiles
    missileManager.missileList = ng::getEntitiesByType(ng::EntityTypes::missileInactive);

    // reload/respawn windows
    uint32 shouldRespawnCount = 15;

    while (gm->isRunning)
    {
        SDL_SetRenderDrawColor(rd->renderer, 255, 255, 255, 255);
        SDL_RenderClear(rd->renderer);

        //respawns
        if (shouldRespawnCount == 0)
        {
            ng::respawnEnemies();
            shouldRespawnCount = 15;
        }
        shouldRespawnCount--;

        if (missileManager.shouldReloadCount != 100)
        {
            missileManager.shouldReloadCount--;
            if (missileManager.shouldReloadCount <= 0)
            {
                missileManager.shouldReloadCount = 100;
                missileManager.currentMissile = 0;
            }
        }

        //update
        ng::moveEnemies();

        handleEvents();

        checkColissionsFor();


        ng::QueryEntitiesResult enemyList = ng::getEntitiesByType("enemy");
        ng::drawFrames(rd, *enemyList.entities, enemyList.count);
        uint32 widthEnemy = ng::getEntityWidthOnDraw(rd, *enemyList.entities);
        ng::drawColliderBox(rd, *enemyList.entities, enemyList.count, widthEnemy);
        ng::freeWorkingMemory(enemyList.freeTag);

        //draw hero
        ng::drawFrames(rd, ng::gameMemory->hero, 1);
        uint32 width = ng::getEntityWidthOnDraw(rd, ng::gameMemory->hero);
        ng::drawColliderBox(rd, ng::gameMemory->hero, 1, width);

        drawMissiles();

        SDL_RenderPresent(rd->renderer);
        //TODO should run atleast in 33 ms
        SDL_Delay(16);
    }

    lg->shutdown();
    rd->shutdown();
    gm->shutdown();

    free(ng::gameMemory);

    return 0;
}