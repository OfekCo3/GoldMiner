 #include "breakoutGame/breakout_game.h"
 #include "bagel.h"
 #include "SDL3_image/SDL_image.h"
 #include "SDL3/SDL.h"
 #include <iostream>
 /**
  * @brief Initializes SDL, creates a window and renderer, and loads the texture sheet.
  *
  * @param window Reference to SDL_Window* pointer to be initialized.
  * @param renderer Reference to SDL_Renderer* pointer to be initialized.
  * @param sheet Reference to SDL_Texture* pointer to be initialized.
  * @return true if initialization succeeds, false otherwise.
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
  * @brief Frees SDL resources including texture, renderer, and window, and shuts down SDL.
  *
  * @param window SDL_Window* to destroy.
  * @param renderer SDL_Renderer* to destroy.
  * @param sheet SDL_Texture* to destroy.
  */
 void cleanUp(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* sheet) {
     SDL_DestroyTexture(sheet);
     SDL_DestroyRenderer(renderer);
     SDL_DestroyWindow(window);
     SDL_Quit();
 }


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



 /*int main() {
     // Create entities
     // Position the ball to hit the wall and then the floor
     ballPos = {400, 100};// near bottom-right corner
     auto& ballVel = bagel::World::getComponent<Velocity>({ballID});
     ballVel = {0, 2}; // moving right

     // Brick & paddle placed where the ball can hit them in early frames
     brickPos = {400, 250};

     paddlePos = {400, 400};

     auto& floorPos = bagel::World::getComponent<Position>({floorID});
     floorPos = {400, 590}; // y=590, matches bottom edge
     for (int i = 0; i < 10; ++i) {
         std::cout << "\n--- Frame " << i << " ---\n";

         PlayerControlSystem();
         MovementSystem();   // 1. update position
         CollisionSystem();  // 2. detect collisions
         DestroySystem();    // 3. remove destroyed entities
         // Check ball status
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
 }*/

int main() {
     SDL_Window* window = nullptr;
     SDL_Renderer* renderer = nullptr;
     SDL_Texture* sheet = nullptr;

     if (!init(window, renderer, sheet)) return -1;

     // ✅ בדיקה: יצירת לבנים עם צבעים שונים
     breakout::CreateBrick(2, breakout::SpriteID::BRICK_BLUE,   100.0f, 100.0f);
     breakout::CreateBrick(1, breakout::SpriteID::BRICK_PURPLE, 300.0f, 150.0f);
     breakout::CreateBrick(3, breakout::SpriteID::BRICK_YELLOW, 500.0f, 200.0f);

     // 🎮 לולאת רינדור
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

         breakout::RenderSystem(renderer, sheet);  // ציור לפי SpriteID

         SDL_RenderPresent(renderer);
         SDL_Delay(16); // ~60 FPS
     }

     cleanUp(window, renderer, sheet);
     return 0;
 }