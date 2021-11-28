

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <main.h>
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
#define SCREEN_HEIGHT 780
#define SCREEN_WIDTH 1280

#include "utils.cpp"
#include "main.h"
#include "ng_math.cpp"
#include "engine.cpp"

int main(int argc, char **argv)
{
    lg->startup();
    rd->startup();

    lg->writeLog("%s number: %d", "test logger", 5);

// TODO move to gameManager
    bool isRunning = true;

    size_t memorySize = megabyte(128);
    ng::gameMemory = (ng::GameMemory *)malloc(memorySize);
    if (ng::gameMemory == NULL)
    {
        perror("Could not allocate game memory\n");
    }

    ng::gameMemory->workingMemoryCur = ng::gameMemory->workingMemory;
    ng::gameMemory->size = memorySize;
    ng::gameMemory->isInit = true;

    ng::Sprite enemySprite = ng::loadSprite("EnemySprite.txt", "enemy");
    for (int i = 0; i < 20; i++)
    {
        ng::addEntityToWorld("enemy", ng::Vector2d(i * 2, i * 3));
    }

    ng::QueryEntitiesResult batmanList = ng::getEntitiesByType("batman");

    ng::Vector2d framePos = ng::Vector2d(10.0f, 20.0f);
    ng::Vector2d framePos2 = ng::Vector2d(26.0f, 20.0f);
    ng::Vector2d framePos3 = ng::Vector2d(200.0f, 20.0f);
    ng::Vector2d framePos4 = ng::Vector2d(200.0f, 120.0f);
    ng::Vector2d framePos5 = ng::Vector2d(200.0f, 220.0f);
    ng::Vector2d positions[] = {framePos, framePos2, framePos3, framePos4, framePos5};



    while (isRunning)
    {
        SDL_RenderClear(rd->renderer);

        ng::drawFrames(rd, enemySprite.frames, positions, enemySprite.frameCount);

        SDL_RenderPresent(rd->renderer);
        SDL_Delay(33);
    }

    free(ng::gameMemory);

    return 0;
}