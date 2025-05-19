 #include "breakoutGame/breakout_game.h"
 #include "bagel.h"
 #include "SDL3/SDL.h"
 #include "SDL3_image/SDL_image.h"


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
     using namespace breakout;

     // יצירת ישויות
     int ballID = CreateBall();
     int paddleID = CreatePaddle(1, 2);
     int brickID = CreateBrick(2);
     int floorID = CreateFloor();
     int uiID = CreateUIManager();

     std::cout << "Entities created:\n";
     std::cout << "Ball ID: " << ballID << "\n";
     std::cout << "Paddle ID: " << paddleID << "\n";
     std::cout << "Brick ID: " << brickID << "\n";
     std::cout << "Floor ID: " << floorID << "\n";
     std::cout << "UIManager ID: " << uiID << "\n";

     // מיקום לצורך בדיקת התנגשות - נשנה את כולם לאותו מקום
     auto& ballPos = bagel::World::getComponent<Position>({ballID});
     ballPos = {100, 100};
     auto& brickPos = bagel::World::getComponent<Position>({brickID});
     brickPos = {100, 100};
     auto& paddlePos = bagel::World::getComponent<Position>({paddleID});
     paddlePos = {100, 100};

     std::cout << "\nRunning systems...\n";

     // הרצת מערכת התנגשות פעמיים כדי לראות שהלבנה מתרסקת
     CollisionSystem();
     CollisionSystem();




     std::cout << "Done.\n";
     return 0;
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