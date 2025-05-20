/**
 * @file breakout_game.cpp
 * @brief Implementation of systems and entity creation functions using BAGEL ECS.
 *
 * This file contains the definitions of all systems and entity constructors
 * used in the Breakout-style game, implemented based on the ECS model with BAGEL.
 * All logic is placeholder only; the focus is on structure and mask-based iteration.
 */

#include "breakout_game.h"
#include "../bagel.h"
#include "SDL3_image/SDL_image.h"
#include <iostream>
#include <unordered_map>

namespace std {
    template <>
    struct hash<breakout::SpriteID> {
        size_t operator()(const breakout::SpriteID& s) const {
            return hash<int>()(static_cast<int>(s));
        }
    };
}

namespace breakout {

    //----------------------------------
    // System Implementations
    //----------------------------------

    /**
     * @brief Updates positions of entities that have both Position and Velocity components.
     */
    void MovementSystem() {
        bagel::Mask required;
        required.set(bagel::Component<Position>::Bit);
        required.set(bagel::Component<Velocity>::Bit);

        constexpr float SCREEN_WIDTH = 800.0f;
        constexpr float SCREEN_HEIGHT = 600.0f;

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (!bagel::World::mask(ent).test(required)) continue;

            auto& pos = bagel::World::getComponent<Position>(ent);
            auto& vel = bagel::World::getComponent<Velocity>(ent);
            auto& collider = bagel::World::getComponent<Collider>(ent);

            pos.x += vel.dx;
            pos.y += vel.dy;

            if (pos.x < 0 || pos.x + collider.width > SCREEN_WIDTH) {
                std::cout << "Entity " << id << " hit horizontal wall\n";
                pos.x = std::clamp(pos.x, 0.0f, SCREEN_WIDTH - collider.width);
                vel.dx *= -1;
            }

            if (pos.y < 0) {
                std::cout << "Entity " << id << " hit top wall\n";
                pos.y = 0;
                vel.dy *= -1;
            }
        }
    }

    bool isColliding(const Position& a, const Collider& ca, const Position& b, const Collider& cb) {
        return (
                a.x < b.x + cb.width &&
                a.x + ca.width > b.x &&
                a.y < b.y + cb.height &&
                a.y + ca.height > b.y
        );
    }

    /**
     * @brief Detects and handles collisions between entities that have Position and Collider components.
     *        Also checks optional components like BallTag and BrickHealth for special behavior.
     */
    void CollisionSystem() {
        bagel::Mask requiredMask;
        requiredMask.set(bagel::Component<Position>::Bit);
        requiredMask.set(bagel::Component<Collider>::Bit);

        for (bagel::id_type id1 = 0; id1 <= bagel::World::maxId().id; ++id1) {
            bagel::ent_type e1{id1};
            if (!bagel::World::mask(e1).test(requiredMask)) continue;
            // We only want e1 to be the ball
            if (!bagel::World::mask(e1).test(bagel::Component<BallTag>::Bit)) continue;

            for (bagel::id_type id2 = 0; id2 <= bagel::World::maxId().id; ++id2) {
                if (id1 == id2) continue;
                bagel::ent_type e2{id2};
                if (!bagel::World::mask(e2).test(requiredMask)) continue;
                if (bagel::World::mask(e2).test(bagel::Component<DestroyedTag>::Bit)) continue;

                auto& p1 = bagel::World::getComponent<Position>(e1);
                auto& c1 = bagel::World::getComponent<Collider>(e1);
                auto& p2 = bagel::World::getComponent<Position>(e2);
                auto& c2 = bagel::World::getComponent<Collider>(e2);

                if (!isColliding(p1, c1, p2, c2)) continue;

                // Ball hits a brick
                if (bagel::World::mask(e2).test(bagel::Component<BrickHealth>::Bit)) {
                    auto& brick = bagel::World::getComponent<BrickHealth>(e2);
                    std::cout << "Ball hit brick! Remaining hits: " << brick.hits << "\n";
                    brick.hits--;
                    auto& vel = bagel::World::getComponent<Velocity>(e1);
                    vel.dy *= -1;

                    if (brick.hits <= 0) {
                        auto& sprite = bagel::World::getComponent<Sprite>(e2);
                        sprite.spriteID = static_cast<SpriteID>(static_cast<int>(sprite.spriteID) + 1);
                        bagel::World::addComponent<BreakAnimation>(e2, {0.1});

                    }
                    break;
                }



                // Ball hits paddle → reverse vertical direction
                else if (bagel::World::mask(e2).test(bagel::Component<PaddleControl>::Bit)) {
                    std::cout << "Ball hit paddle! Inverting Y velocity.\n";
                    auto& vel = bagel::World::getComponent<Velocity>(e1);
                    vel.dy *= -1;
                    break;
                }

                // Ball hits floor → (later: reduce life count)
                else if (bagel::World::mask(e2).test(bagel::Component<FloorTag>::Bit)) {
                    std::cout << "Ball hit the floor!\n";
                    bagel::World::addComponent<DestroyedTag>(e1, {});
                    break;
                }
            }
        }
    }

    /**
     * @brief Reads player input and updates paddle position accordingly.
     */
    void PlayerControlSystem() {
        bagel::Mask required;
        required.set(bagel::Component<Position>::Bit);
        required.set(bagel::Component<PaddleControl>::Bit);

        // Poll keyboard state
        SDL_PumpEvents();
        int numKeys = 0;
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (!bagel::World::mask(ent).test(required)) continue;

            auto& pos = bagel::World::getComponent<Position>(ent);
            const auto& control = bagel::World::getComponent<PaddleControl>(ent);

            const float speed = 5.0f;

            if (keys[control.keyLeft]) {
                pos.x -= speed;
            }
            if (keys[control.keyRight]) {
                pos.x += speed;
            }
            // Reflect from left/right walls
            pos.x = std::clamp(pos.x, 0.0f, 800.0f - (161.0f * 0.7f));
        }

    }

    /**
     * @brief Applies power-up effects to entities, checking optional components like PaddleControl and BallTag.
     */
    void PowerUpSystem() {
        bagel::Mask required;
        required.set(bagel::Component<PowerUpType>::Bit);
        required.set(bagel::Component<TimedEffect>::Bit);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (bagel::World::mask(ent).test(required)) {
                bool hasPaddle = bagel::World::mask(ent).test(bagel::Component<PaddleControl>::Bit);
                bool hasBall = bagel::World::mask(ent).test(bagel::Component<BallTag>::Bit);
                (void)hasPaddle; (void)hasBall;
            }
        }
    }

    /**
     * @brief Removes entities that are marked for deletion using the DestroyedTag component.
     */
    void DestroySystem() {
        bagel::Mask required;
        required.set(bagel::Component<DestroyedTag>::Bit);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (!bagel::World::mask(ent).test(required)) continue;

            std::cout << "Destroying entity: " << id << "\n";
            bagel::World::destroyEntity(ent);
        }
    }

    /**
     * @brief Displays UI-related data such as score and lives for entities that hold those components.
     */
    void UISystem() {
        bagel::Mask required;
        required.set(bagel::Component<Score>::Bit);
        required.set(bagel::Component<LifeCount>::Bit);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (bagel::World::mask(ent).test(required)) {
                // Rendering of score/lives would go here
            }
        }
    }

    //----------------------------------
    // Entity Creation Functions
    //----------------------------------

    /**
     * @brief Creates a new ball entity with position, velocity, sprite, and collision.
     *        The ball starts near the center of the screen and moves upward.
     *
     * @return Unique entity ID
     */
    id_type CreateBall() {
        bagel::Entity e = bagel::Entity::create();

        Position pos{400.0f, 450.0f};
        Velocity vel{1.2f, 1.5f};
        Sprite sprite{SpriteID::BALL};
        float spriteW = 87.0f * 0.4f;
        float spriteH = 77.0f * 0.4f;
        Collider collider{spriteW, spriteH};
        BallTag tag;

        e.addAll(pos, vel, sprite, collider, tag);
        return e.entity().id;
    }

    /**
     * @brief Creates a new brick entity with position, brickHealth sprite, and collision.
     *        The ball starts near the center of the screen and moves upward.
     *
     * @return Unique entity ID
     */
    id_type CreateBrick(int health, SpriteID color, float x, float y) {
        bagel::Entity e = bagel::Entity::create();

        e.addAll(
            Position{x, y},
            Sprite{color},
            Collider{171.0f * 0.7f, 59.0f * 0.7f},
            BrickHealth{health}
        );

        return e.entity().id;
    }

     /**
     * @brief Creates a paddle entity with position, sprite, collision, and input controls.
     *
     * The paddle is placed near the bottom of the screen with a default sprite and
     * a collider for ball interaction. It responds to the provided keyboard keys.
     *
     * @param left Key code (SDL_Scancode) for moving left
     * @param right Key code (SDL_Scancode) for moving right
     * @return Unique entity ID
     */
    id_type CreatePaddle(int left, int right) {
        bagel::Entity e = bagel::Entity::create();
        Position pos{320.0f, 560.0f};
        Sprite sprite{SpriteID::PADDLE};
        Collider collider{161.0f * 0.7f, 55.0f * 0.7f};
        PaddleControl control{left, right};

        e.addAll(pos, sprite, collider, control);
        return e.entity().id;
    }

    /**
     * @brief Creates a falling power-up with a defined type and timed effect.
     * @param type Identifier for the power-up effect
     * @return Unique entity ID
     */
    id_type CreatePowerUp(int type) {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Velocity{}, Sprite{}, Collider{}, PowerUpType{type}, TimedEffect{}, DestroyedTag{});
        return e.entity().id;
    }

    /**
     * @brief Creates a UI manager entity to display score and life count.
     * @return Unique entity ID
     */
    id_type CreateUIManager() {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Score{}, LifeCount{});
        return e.entity().id;
    }

    /**
     * @brief Creates a floor entity to check if ball hit the floor - game over.
     * @return Unique entity ID
     */
    id_type CreateFloor() {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{0.0f, 590.0f}, Collider{800.0f, 10.0f}, FloorTag{});
        return e.entity().id;
    }

    /**
  * @brief Main game loop for the Breakout ECS-style game.
  *
  * Initializes all core entities (UI, paddle, ball, bricks),
  * runs input, system updates, and rendering each frame.
  *
  * @param ren SDL renderer for drawing.
  * @param tex Texture sheet for all game sprites.
  */
    void run(SDL_Renderer* ren, SDL_Texture* tex) {
        using namespace bagel;

        CreateUIManager();
        CreatePaddle(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT);
        CreateBall();
        CreateFloor();
        CreateBrickGrid(4, 6, 1); // 4 rows × 6 cols, health = 1

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            Uint32 frameStart = SDL_GetTicks();

            // === Input handling ===
            SDL_PumpEvents();

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT ||
                    (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_ESCAPE)) {
                    quit = true;
                    }
            }

            // === System updates ===
            PlayerControlSystem();  // Moves paddle
            MovementSystem();       // Moves ball
            CollisionSystem();      // Handles ball collisions
            PowerUpSystem();        // Future logic
            DestroySystem();        // Removes destroyed entities

            World::step();          // Apply component changes

            // === Rendering ===
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);

            RenderSystem(ren, tex); // Uses SpriteID and Position

            SDL_RenderPresent(ren);

            // Frame limiter (~60 FPS)
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < 16) SDL_Delay(16 - frameTime);
        }
    }

    static const std::unordered_map<SpriteID, SDL_FRect> SPRITE_ATLAS = {
        {SpriteID::BALL,            {800, 548, 87, 77}},
        {SpriteID::PADDLE,          {392, 9, 161, 55}},
        {SpriteID::BRICK_BLUE,      {21, 17, 171, 59}},
        {SpriteID::BRICK_BLUE_DMG,  {209, 16, 171, 60}},
        {SpriteID::BRICK_PURPLE,    {20, 169, 168, 57}},
        {SpriteID::BRICK_PURPLE_DMG,{208, 168, 170, 58}},
        {SpriteID::BRICK_YELLOW,    {20, 469, 169, 59}},
        {SpriteID::BRICK_YELLOW_DMG,{210, 470, 166, 63}},
        {SpriteID::BRICK_ORANGE,    {17, 319, 175, 57}},
        {SpriteID::BRICK_ORANGE_DMG,{206, 318, 175, 58}},
        {SpriteID::LASER,           {837, 643, 11, 22}}
    };


    void RenderSystem(SDL_Renderer* ren, SDL_Texture* tex) {
        using namespace bagel;

        Mask required;
        required.set(Component<Position>::Bit);
        required.set(Component<Sprite>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (World::mask(ent).test(required)) {
                const auto& pos = World::getComponent<Position>(ent);
                const auto& sprite = World::getComponent<Sprite>(ent);

                SDL_FRect dst = {pos.x, pos.y, 40, 40};  // default
                auto it = SPRITE_ATLAS.find(sprite.spriteID);
                if (it != SPRITE_ATLAS.end()) {
                    const SDL_FRect& src = it->second;
                    dst.w = src.w * 0.7f;  // scale down width
                    dst.h = src.h * 0.7f;  // scale down height
                    if (sprite.spriteID==SpriteID::BALL) {
                        dst.w = src.w * 0.4f;  // scale down width
                        dst.h = src.h * 0.4f;  // scale down height
                    }
                    SDL_RenderTexture(ren, tex, &src, &dst);
                }
            }
        }
    }

    void CreateBrickGrid(int rows, int cols, int health) {
        const float brickW = 120.0f, brickH = 40.0f;
        const float spacingX = 5.0f, spacingY = 5.0f;
        float totalWidth = cols * brickW + (cols - 1) * spacingX;
        float startX = (800.0f - totalWidth) / 2.0f;
        float startY = 80.0f;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                float x = startX + col * (brickW + spacingX);
                float y = startY + row * (brickH + spacingY);
                SpriteID color = static_cast<SpriteID>(2 + (row % 4) * 2); // AMAL: BRICK_* enums
                CreateBrick(health, color, x, y);
            }
        }
    }

    void BreakAnimationSystem(float deltaTime) {
        bagel::Mask breakMask;
        breakMask.set(bagel::Component<BreakAnimation>::Bit);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type entity{id};
            if (!bagel::World::mask(entity).test(breakMask)) continue;

            auto& anim = bagel::World::getComponent<BreakAnimation>(entity);
            anim.timer += deltaTime;

            if (anim.timer >= 1.0f) {
                bagel::World::addComponent<DestroyedTag>(entity, {});

            }
        }
    }
} //namespace breakout;