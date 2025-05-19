// #include "breakoutGame/breakout_game.h"
// #include "SDL3_image/SDL_image.h"
// #include "SDL3/SDL.h"
// #include <iostream>
// /**
//  * @brief Initializes SDL, creates a window and renderer, and loads the texture sheet.
//  *
//  * @param window Reference to SDL_Window* pointer to be initialized.
//  * @param renderer Reference to SDL_Renderer* pointer to be initialized.
//  * @param sheet Reference to SDL_Texture* pointer to be initialized.
//  * @return true if initialization succeeds, false otherwise.
//  */
// bool init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& sheet) {
//     if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//         std::cerr << "SDL Init failed: " << SDL_GetError() << "\n";
//         return false;
//     }
//
//     if (SDL_CreateWindowAndRenderer("Breakout ECS", 800, 600, 0, &window, &renderer) < 0) {
//         std::cerr << "Failed to create window or renderer: " << SDL_GetError() << "\n";
//         return false;
//     }
//
//     sheet = IMG_LoadTexture(renderer, "res/breakout.png");
//     if (!sheet) {
//         std::cerr << "Failed to load texture: " << SDL_GetError() << "\n";
//         return false;
//     }
//
//     return true;
// }
//
// /**
//  * @brief Frees SDL resources including texture, renderer, and window, and shuts down SDL.
//  *
//  * @param window SDL_Window* to destroy.
//  * @param renderer SDL_Renderer* to destroy.
//  * @param sheet SDL_Texture* to destroy.
//  */
// void cleanUp(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* sheet) {
//     SDL_DestroyTexture(sheet);
//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
// }
//
//
// int main() {
//     SDL_Window* window = nullptr;
//     SDL_Renderer* renderer = nullptr;
//     SDL_Texture* sheet = nullptr;
//
//     if (!init(window, renderer, sheet)) return -1;
//
//     breakout::run(renderer, sheet);
//
//     cleanUp(window, renderer, sheet);
//     return 0;
//}