/**
 * @file breakout_game.cpp
 * @brief Implementation of systems and entity creation functions using BAGEL ECS.
 *
 * This file contains the definitions of all systems and entity constructors
 * used in the Breakout-style game, implemented based on the ECS model with BAGEL.
 */

#include "breakout_game.h"
#include "../bagel.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <unordered_map>
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
                auto& pos = bagel::World::getComponent<Position>(ent);
                const auto& vel = bagel::World::getComponent<Velocity>(ent);
                pos.x += vel.dx;
                pos.y += vel.dy;
            }
        }
    }

    /**
     * @brief Detects and handles collisions between entities that have Position and Collider components.
     *        Also checks optional components like BallTag and BrickHealth for special behavior.
     */
    void CollisionSystem() {
        bagel::Mask required;
        required.set(bagel::Component<Position>::Bit);
        required.set(bagel::Component<Collider>::Bit);

        for (bagel::id_type id1 = 0; id1 <= bagel::World::maxId().id; ++id1) {
            bagel::ent_type e1{id1};
            if (!bagel::World::mask(e1).test(required)) continue;

            for (bagel::id_type id2 = 0; id2 <= bagel::World::maxId().id; ++id2) {
                if (id1 == id2) continue;
                bagel::ent_type e2{id2};
                if (!bagel::World::mask(e2).test(required)) continue;

                // Optional checks
                bool isBall = bagel::World::mask(e1).test(bagel::Component<BallTag>::Bit);
                bool hasHealth = bagel::World::mask(e2).test(bagel::Component<BrickHealth>::Bit);
                (void)isBall; (void)hasHealth;
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

        SDL_PumpEvents();
        auto keys = SDL_GetKeyboardState(nullptr);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (bagel::World::mask(ent).test(required)) {
                auto& pos = bagel::World::getComponent<Position>(ent);
                const auto& control = bagel::World::getComponent<PaddleControl>(ent);

                if (keys[control.keyLeft]) {
                    pos.x -= 4.0f;
                }
                if (keys[control.keyRight]) {
                    pos.x += 4.0f;
                }
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
                bagel::World::destroyEntity(ent);
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

    /**
     * @brief Draws all entities with Position and Sprite components onto the screen.
     *
     * Uses spriteID from the Sprite component to locate the subtexture to draw from the sprite sheet.
     * Assumes the SDL_Renderer and SDL_Texture (sheet) are passed externally.
     *
     * @param renderer The SDL_Renderer used for drawing.
     * @param sheet The SDL_Texture containing the sprite sheet.
     */
    void RenderSystem(SDL_Renderer* renderer, SDL_Texture* sheet) {
        using namespace bagel;

        static const std::unordered_map<int, SDL_FRect> SPRITE_MAP = {
            {0, {800, 548, 87, 77}},     // Ball
            {1, {392, 9, 161, 55}},      // Paddle
            {2, {21, 17, 171, 59}},      // Brick Blue
            {3, {209, 16, 171, 60}},     // Brick Blue Damaged
            {4, {20, 169, 168, 57}},     // Brick Purple
            {5, {208, 168, 170, 58}},    // Brick Purple Damaged
            {6, {20, 469, 169, 59}},     // Brick Yellow
            {7, {210, 470, 166, 63}},    // Brick Yellow Damaged
            {8, {17, 319, 175, 57}},     // Brick Orange
            {9, {206, 318, 175, 58}},    // Brick Orange Damaged
            {10, {837, 643, 11, 22}},    // Laser
        };

        Mask required;
        required.set(Component<Position>::Bit);
        required.set(Component<Sprite>::Bit);

        SDL_RenderClear(renderer);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(required)) continue;

            const auto& pos = World::getComponent<Position>(ent);
            const auto& sprite = World::getComponent<Sprite>(ent);

            auto it = SPRITE_MAP.find(sprite.spriteID);
            if (it == SPRITE_MAP.end()) continue;

            SDL_FRect dst = {
                pos.x,
                pos.y,
                it->second.w,
                it->second.h
            };

            SDL_RenderTexture(renderer, sheet, &it->second, &dst);
        }

        SDL_RenderPresent(renderer);
    }

    //----------------------------------
    // Entity Creation Functions
    //----------------------------------

    id_type CreateBall() {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Velocity{}, Sprite{}, Collider{}, BallTag{});
        return e.entity().id;
    }

    id_type CreateBrick(int health) {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Sprite{}, Collider{}, BrickHealth{health});
        return e.entity().id;
    }

    id_type CreatePaddle(int left, int right) {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Sprite{}, Collider{}, PaddleControl{left, right});
        return e.entity().id;
    }

    id_type CreatePowerUp(int type) {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Position{}, Velocity{}, Sprite{}, Collider{}, PowerUpType{type}, TimedEffect{}, DestroyedTag{});
        return e.entity().id;
    }

    id_type CreateUIManager() {
        bagel::Entity e = bagel::Entity::create();
        e.addAll(Score{}, LifeCount{});
        return e.entity().id;
    }

} // namespace breakout
