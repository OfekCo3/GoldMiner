#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "goldMiner/gold_miner_ecs.h"
#include "goldMiner/sprite_manager.h"
#include "bagel.h"
#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Gold Miner ECS", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load all textures
    LoadAllSprites(renderer);

    // Create sample entities
    goldminer::CreatePlayer(1);
    goldminer::CreateRope(1);
    goldminer::CreateGold(400.0f, 500.0f);
    goldminer::CreateDiamond(600.0f, 520.0f);
    goldminer::CreateRock(800.0f, 530.0f);
    goldminer::CreateMysteryBag(300.0f, 510.0f);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render background
        SDL_FRect bg = {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
        SDL_RenderTexture(renderer, GetSpriteTexture(goldminer::SPRITE_BACKGROUND), nullptr, &bg);

        // Render ECS entities
        goldminer::RenderSystem(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }

    UnloadAllSprites();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
