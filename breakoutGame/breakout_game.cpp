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

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (bagel::World::mask(ent).test(required)) {
                // Movement logic would go here
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
        bagel::Mask mask;
        mask.set(bagel::Component<Position>::Bit);
        mask.set(bagel::Component<Collider>::Bit);

        for (id_type id1 = 0; id1 <= bagel::World::maxId().id; ++id1) {
            bagel::ent_type e1{id1};
            if (!bagel::World::mask(e1).test(mask)) continue;

            for (id_type id2 = id1 + 1; id2 <= bagel::World::maxId().id; ++id2) {
                bagel::ent_type e2{id2};
                if (!bagel::World::mask(e2).test(mask)) continue;

                auto& p1 = bagel::World::getComponent<Position>(e1);
                auto& c1 = bagel::World::getComponent<Collider>(e1);
                auto& p2 = bagel::World::getComponent<Position>(e2);
                auto& c2 = bagel::World::getComponent<Collider>(e2);

                if (!isColliding(p1, c1, p2, c2)) continue;

                bool ball1 = bagel::World::mask(e1).test(bagel::Component<BallTag>::Bit);
                bool ball2 = bagel::World::mask(e2).test(bagel::Component<BallTag>::Bit);

                // כדור פוגע בלבנה
                if (ball1 && bagel::World::mask(e2).test(bagel::Component<BrickHealth>::Bit)) {
                    auto& brick = bagel::World::getComponent<BrickHealth>(e2);
                    std::cout << "Ball hit brick! Remaining hits: " << brick.hits << "\n";
                    brick.hits--;
                    if (brick.hits <= 0) {
                        bagel::World::addComponent<DestroyedTag>(e2, {});
                    }
                }
                else if (ball2 && bagel::World::mask(e1).test(bagel::Component<BrickHealth>::Bit)) {
                    auto& brick = bagel::World::getComponent<BrickHealth>(e1);
                    brick.hits--;
                    if (brick.hits <= 0) {
                        bagel::World::addComponent<DestroyedTag>(e1, {});
                    }
                }

                // כדור פוגע בפדל — שנה את כיוון הכדור
                if (ball1 && bagel::World::mask(e2).test(bagel::Component<PaddleControl>::Bit)) {
                    std::cout << "Ball hit paddle! Inverting Y velocity.\n";

                    auto& vel = bagel::World::getComponent<Velocity>(e1);
                    vel.dy *= -1;
                }
                else if (ball2 && bagel::World::mask(e1).test(bagel::Component<PaddleControl>::Bit)) {
                    auto& vel = bagel::World::getComponent<Velocity>(e2);
                    vel.dy *= -1;
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

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (bagel::World::mask(ent).test(required)) {
                // Paddle movement based on input would go here
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
            if (bagel::World::mask(ent).test(required)) {
                // Destruction/removal logic would go here
            }
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

    /**
     * @brief Creates a brick entity with a specified amount of health.
     * @param health Number of hits required to break the brick
     * @return Unique entity ID
     */
    id_type CreateBrick(int health) {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Sprite{}, Collider{}, BrickHealth{health});
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
    void run(SDL_Renderer* ren, SDL_Texture* tex) {
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
    }


}




// namespace breakout
