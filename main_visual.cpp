#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "goldMiner/gold_miner_ecs.h"
#include "goldMiner/sprite_manager.h"
#include "bagel.h"
#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {
    std::cout << "Starting program...\n";

    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "SDL Init success.\n";

    SDL_Window* window = SDL_CreateWindow("Gold Miner ECS", 1280, 720, 0);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "Window created.\n";

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "Renderer created.\n";

    std::cout << "Loading textures...\n";
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

        SDL_FRect bg = {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
        SDL_RenderTexture(renderer, GetSpriteTexture(SPRITE_BACKGROUND), nullptr, &bg);

        goldminer::RenderSystem(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }

    UnloadAllSprites();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
