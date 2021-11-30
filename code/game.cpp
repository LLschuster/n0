

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

void generateWorld()
{
    ng::Sprite enemySprite = ng::loadSprite("EnemySprite.txt", "enemy");
    ng::Randomizer xGenerator = {};
    ng::Randomizer yGenerator = {};
    xGenerator = ng::getRandomReal((float)SCREEN_WIDTH * 0.3, (float)SCREEN_WIDTH * 0.4f);
    yGenerator = ng::getRandomReal(0.0f, (float)SCREEN_HEIGHT * 0.5f);
    for (int i = 0; i < 10; i++)
    {
        float x = xGenerator.distribution.realD(xGenerator.generator);
        float y = yGenerator.distribution.realD(yGenerator.generator);
        float newXLow = x + enemySprite.width * ng::FONT_SIZE_PIXEL;
        xGenerator = ng::getRandomReal(newXLow, newXLow * 1.2);

        ng::addEntityToWorld("enemy", ng::Vector2d(x, y), enemySprite);
    }

    xGenerator = ng::getRandomReal((float)SCREEN_WIDTH * 0.3, (float)SCREEN_WIDTH * 0.4f);
    yGenerator = ng::getRandomReal((float)SCREEN_HEIGHT * 0.5f, (float)SCREEN_HEIGHT);
    for (int i = 0; i < 10; i++)
    {
        float x = xGenerator.distribution.realD(xGenerator.generator);
        float y = yGenerator.distribution.realD(yGenerator.generator);
        float newXLow = x + enemySprite.width * ng::FONT_SIZE_PIXEL;
        xGenerator = ng::getRandomReal(newXLow, newXLow * 1.2);

        ng::addEntityToWorld("enemy", ng::Vector2d(x, y), enemySprite);
    }
}

int main(int argc, char **argv)
{
    int initResult = 0;
    initResult += lg->startup();
    initResult += rd->startup();

    if (initResult > 0)
    {
        lg->writeLog("Systems initialization failed");
        exit(initResult);
    }

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

    generateWorld();

    while (isRunning)
    {
        SDL_RenderClear(rd->renderer);

        ng::QueryEntitiesResult enemyList = ng::getEntitiesByType("enemy");
        ng::drawFrames(rd, *enemyList.entities, enemyList.count);

        SDL_RenderPresent(rd->renderer);
        SDL_Delay(33);
    }

    lg->shutdown();
    rd->shutdown();

    free(ng::gameMemory);

    return 0;
}