#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

#include "goldMiner/gold_miner_ecs.h"
#include "goldMiner/sprite_manager.h"
#include "bagel.h"

#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main() {
    std::cout << "Starting Gold Miner ECS...\n";

    SDL_Window* window = SDL_CreateWindow("Gold Miner ECS", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    goldminer::initBox2DWorld();
    // Load sprite textures from "res" folder
    LoadAllSprites(renderer);
    // Create some initial entities
    goldminer::CreatePlayer(1);
    goldminer::CreateRope(1);
    goldminer::CreateGold(100.0f, 500.0f);
    goldminer::CreateDiamond(600.0f, 520.0f);
    goldminer::CreateRock(1000.0f, 530.0f);
//    goldminer::CreateMysteryBag(300.0f, 510.0f);
    goldminer::CreateTreasureChest(300.0f, 510.0f);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Step the Box2D world at fixed time step (60 FPS)
        constexpr float timeStep = 1.0f / 60.0f;
        constexpr int velocityIterations = 8;
        constexpr int positionIterations = 3;
        b2World_Step(goldminer::gWorld, timeStep, velocityIterations);

        goldminer::PhysicsSyncSystem();
        goldminer::CollisionSystem();
        goldminer::DebugCollisionSystem();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw background
        SDL_FRect bg = {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
        SDL_RenderTexture(renderer, GetSpriteTexture(SPRITE_BACKGROUND), nullptr, &bg);

        // Render ECS entities
        goldminer::RenderSystem(renderer);
        goldminer::RopeRenderSystem(renderer);


        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Approximate 60 FPS
    }

    // Cleanup
    UnloadAllSprites();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
