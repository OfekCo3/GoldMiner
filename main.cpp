#include "breakoutGame/breakout_game.h"
#include "bagel.h"

#include "lib/SDL/include/SDL3/SDL.h"
#include "lib/SDL_image/include/SDL3_image/SDL_image.h"
#include <iostream>

/**
 * @brief Initializes SDL, creates a window and renderer, and loads the texture sheet.
 */
bool init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& sheet) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (SDL_CreateWindowAndRenderer("Breakout ECS", 800, 600, 0, &window, &renderer) < 0) {
        std::cerr << "Failed to create window or renderer: " << SDL_GetError() << "\n";
        return false;
    }

    sheet = IMG_LoadTexture(renderer, "res/breakout.png");
    if (!sheet) {
        std::cerr << "Failed to load texture: " << SDL_GetError() << "\n";
        return false;
    }

    return true;
}

/**
 * @brief Frees SDL resources and shuts down SDL.
 */
void cleanUp(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* sheet) {
    SDL_DestroyTexture(sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* sheet = nullptr;

    if (!init(window, renderer, sheet)) return -1;

    // ✅ Run the full ECS-based game
    breakout::run(renderer, sheet);

    cleanUp(window, renderer, sheet);
    return 0;
}

/*
=====================
🧪 Alternate test versions (commented)
=====================

// Version that runs ECS manually without full game loop:
/*
int main() {
    // Create entities manually for testing
    ballPos = {400, 100};
    auto& ballVel = bagel::World::getComponent<Velocity>({ballID});
    ballVel = {0, 2};
    brickPos = {400, 250};
    paddlePos = {400, 400};
    auto& floorPos = bagel::World::getComponent<Position>({floorID});
    floorPos = {400, 590};

    for (int i = 0; i < 10; ++i) {
        std::cout << "\n--- Frame " << i << " ---\n";
        PlayerControlSystem();
        MovementSystem();
        CollisionSystem();
        DestroySystem();

        const auto& mask = bagel::World::mask({ballID});
        if (mask.ctz() == -1) {
            std::cout << "Ball entity was destroyed successfully. Exiting loop.\n";
            break;
        }

        auto& pos = bagel::World::getComponent<Position>({ballID});
        auto& vel = bagel::World::getComponent<Velocity>({ballID});
        std::cout << "Ball Position: (" << pos.x << ", " << pos.y << ") ";
        std::cout << "Velocity: (" << vel.dx << ", " << vel.dy << ")\n";
    }

    std::cout << "\nAll systems tested.\n";
}
*/

// Version that renders static bricks without ECS loop:
/*
int main() {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* sheet = nullptr;

    if (!init(window, renderer, sheet)) return -1;

    breakout::CreateBrick(2, breakout::SpriteID::BRICK_BLUE,   100.0f, 100.0f);
    breakout::CreateBrick(1, breakout::SpriteID::BRICK_PURPLE, 300.0f, 150.0f);
    breakout::CreateBrick(3, breakout::SpriteID::BRICK_YELLOW, 500.0f, 200.0f);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        SDL_PumpEvents();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT ||
                (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_ESCAPE)) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        breakout::RenderSystem(renderer, sheet);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    cleanUp(window, renderer, sheet);
    return 0;
}
*/
