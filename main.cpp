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


 int main() {
     using namespace breakout;

     // Create all needed entities
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

     // Position all entities near each other for collision testing
     auto& ballPos = bagel::World::getComponent<Position>({ballID});
     ballPos = {790, 10}; // near right wall
     auto& ballVel = bagel::World::getComponent<Velocity>({ballID});
     ballVel = {5, -3}; // fast toward wall and up

     auto& brickPos = bagel::World::getComponent<Position>({brickID});
     brickPos = {100, 100};

     auto& paddlePos = bagel::World::getComponent<Position>({paddleID});
     paddlePos = {100, 100};

     auto& floorPos = bagel::World::getComponent<Position>({floorID});
     floorPos = {400, 590};

     std::cout << "\nRunning systems...\n";

     // Simulate a few frames
     for (int i = 0; i < 5; ++i) {
         std::cout << "Frame " << i << ":\n";
         MovementSystem();   // Apply velocity
         CollisionSystem();  // Handle hits (brick/paddle/floor)

         auto& pos = bagel::World::getComponent<Position>({ballID});
         auto& vel = bagel::World::getComponent<Velocity>({ballID});
         std::cout << "Ball Position: (" << pos.x << ", " << pos.y << ") ";
         std::cout << "Velocity: (" << vel.dx << ", " << vel.dy << ")\n";
     }

     std::cout << "Done.\n";
     return 0;
 }
