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

            // Apply velocity to position
            pos.x += vel.dx;
            pos.y += vel.dy;

            // Reflect from left/right walls
            if (pos.x < 0 || pos.x > SCREEN_WIDTH) {
                std::cout << "Entity " << id << " hit horizontal wall\n";
                pos.x = std::clamp(pos.x, 0.0f, SCREEN_WIDTH);
                vel.dx *= -1;
            }

            // Reflect from top wall only (bottom is handled by FloorTag in CollisionSystem)
            if (pos.y < 0) {
                std::cout << "Entity " << id << " hit top wall\n";
                pos.y = 0;
                vel.dy *= -1;
            }
        }
    }

    bool isColliding(const Position& a, const Collider& ca, const Position& b, const Collider& cb) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float distSq = dx * dx + dy * dy;
        float radiusSum = ca.radius + cb.radius;
        return distSq <= radiusSum * radiusSum;
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
                    if (brick.hits <= 0) {
                        bagel::World::addComponent<DestroyedTag>(e2, {});
                    }
                }

                // Ball hits paddle → reverse vertical direction
                else if (bagel::World::mask(e2).test(bagel::Component<PaddleControl>::Bit)) {
                    std::cout << "Ball hit paddle! Inverting Y velocity.\n";
                    auto& vel = bagel::World::getComponent<Velocity>(e1);
                    vel.dy *= -1;
                }

                // Ball hits floor → (later: reduce life count)
                else if (bagel::World::mask(e2).test(bagel::Component<FloorTag>::Bit)) {
                    std::cout << "Ball hit the floor!\n";
                    bagel::World::addComponent<DestroyedTag>(e1, {});
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
     * @brief Creates a new ball entity with basic motion and collision components.
     * @return Unique entity ID
     */
    id_type CreateBall() {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Velocity{}, Sprite{}, Collider{}, BallTag{});
        return e.entity().id;
    }

    id_type CreateBrick(int health, SpriteID color, float x, float y) {
        bagel::Entity e = bagel::Entity::create();

        e.addAll(
            Position{x, y},
            Sprite{color},
            Collider{10},        //fix r
            BrickHealth{health}
        );

        return e.entity().id;
    }

    /**
     * @brief Creates a paddle entity with assigned movement key controls.
     * @param left Key code for left movement
     * @param right Key code for right movement
     * @return Unique entity ID
     */
    id_type CreatePaddle(int left, int right) {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Sprite{}, Collider{}, PaddleControl{left, right});
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

    id_type CreateFloor() {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{400.0f, 590.0f}, Collider{1000}, FloorTag{});
        return e.entity().id;
    }

    /**
     * @brief Main game loop for the Breakout ECS-style game.
     *
     * Initializes entities, handles player input, updates systems,
     * and renders all game entities with Position and Sprite components.
     *
     * @param ren SDL renderer for drawing.
     * @param tex Texture sheet for all game sprites.
     */
    /*void run(SDL_Renderer* ren, SDL_Texture* tex) {
        using namespace bagel;

        // === Create initial entities ===
        CreateUIManager();
        CreatePaddle(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT);
        CreateBall();

        for (int i = 0; i < 10; ++i)
            CreateBrick(1 + (i % 3));  // Bricks with different durability

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            Uint32 frameStart = SDL_GetTicks();

            // === Input handling ===
            SDL_PumpEvents();
            const bool* keys = SDL_GetKeyboardState(nullptr);  // SDL3 returns bool*

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT)
                    quit = true;
                else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_ESCAPE)
                    quit = true;
            }

            // Move paddle based on keyboard input
            for (bagel::id_type id = 0; id <= World::maxId().id; ++id) {
                ent_type ent{id};
                if (World::mask(ent).test(Component<PaddleControl>::Bit) &&
                    World::mask(ent).test(Component<Position>::Bit)) {

                    auto& pos = World::getComponent<Position>(ent);
                    const auto& control = World::getComponent<PaddleControl>(ent);

                    if (keys[control.keyLeft])  pos.x -= 5.0f;
                    if (keys[control.keyRight]) pos.x += 5.0f;
                }
            }

            // === System logic ===
            MovementSystem();
            CollisionSystem();
            PowerUpSystem();
            DestroySystem();

            World::step();  // Clears internal state after changes

            // === Rendering ===
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);

            for (bagel::id_type id = 0; id <= World::maxId().id; ++id) {
                ent_type ent{id};
                if (World::mask(ent).test(Component<Position>::Bit) &&
                    World::mask(ent).test(Component<Sprite>::Bit)) {

                    const auto& pos = World::getComponent<Position>(ent);

                    SDL_FRect dst = {pos.x, pos.y, 40, 40};  // Default size
                    SDL_FRect src = {0, 0, 87, 77};          // Placeholder sprite from sheet

                    SDL_RenderTexture(ren, tex, &src, &dst);
                }
            }

            SDL_RenderPresent(ren);

            // Frame limiting
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < 16) SDL_Delay(16 - frameTime);
        }
    }*/



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
                    dst.w = src.w;
                    dst.h = src.h;
                    SDL_RenderTexture(ren, tex, &src, &dst);
                }
            }
        }
    }



}




// namespace breakout
