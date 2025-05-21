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
#include <vector>
#include <box2d/box2d.h>

/**
 * @brief Specialization of std::hash for breakout::eSpriteID.
 *
 * This allows breakout::eSpriteID to be used as a key in unordered_map or other hash-based containers.
 * It works by casting the enum value to an int and hashing that.
 */
namespace std {
    template <>
    struct hash<breakout::eSpriteID> {
        size_t operator()(const breakout::eSpriteID& s) const {
            return hash<int>()(static_cast<int>(s));
        }
    };
}

namespace breakout {

    b2WorldId boxWorld = b2_nullWorldId;

    /**
     * @brief Initializes the Box2D physics world with zero gravity.
     *
     * This function creates a new Box2D world and assigns it to the global `boxWorld` variable.
     * The gravity is set to (0, 0) since movement is manually controlled.
     * */
    void PrepareBoxWorld() {
        b2WorldDef def = b2DefaultWorldDef();
        def.gravity = {0.0f, 0.0f};
        boxWorld = b2CreateWorld(&def);
    }

    //----------------------------------
    /// @section Initialization Helpers
    //----------------------------------

    /**
     * @brief Creates a full grid of bricks arranged in rows and columns.
     *        In the center of the top row, a star power-up is placed instead of a brick.
     *
     * @param rows Number of brick rows
     * @param cols Number of bricks per row
     * @param health Health value assigned to each brick
     */
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
                eSpriteID color = static_cast<eSpriteID>(2 + (row % 4) * 2); // Choose color by row

                // Place the star and the heart
                if (row == 1 && col == 1) {
                    CreateStar(x, y);
                }
                else if (row == 2 && col == cols - 2) {
                    CreateHeart(x, y);
                }
                else {
                    CreateBrick(health, color, x, y);
                }
            }
        }
    }

    /**
     * @brief Sprite atlas mapping each eSpriteID to its rectangle in the sprite sheet.
     *
     * Each entry defines the source rectangle (x, y, width, height) of the sprite
     * within the shared texture atlas. These coordinates are used during rendering
     * to extract the correct portion of the image.
     *
     * Units are in pixels and refer to positions inside the texture image.
     */
    static const std::unordered_map<eSpriteID, SDL_FRect> SPRITE_ATLAS = {
            {eSpriteID::BALL, {800, 548, 87, 77}},
            {eSpriteID::PADDLE, {392, 9, 161, 55}},
            {eSpriteID::BRICK_BLUE, {21, 17, 171, 59}},
            {eSpriteID::BRICK_BLUE_DMG, {209, 16, 171, 60}},
            {eSpriteID::BRICK_PURPLE, {20, 169, 168, 57}},
            {eSpriteID::BRICK_PURPLE_DMG, {208, 168, 170, 58}},
            {eSpriteID::BRICK_YELLOW, {20, 469, 169, 59}},
            {eSpriteID::BRICK_YELLOW_DMG, {210, 470, 166, 63}},
            {eSpriteID::BRICK_ORANGE, {17, 319, 175, 57}},
            {eSpriteID::BRICK_ORANGE_DMG, {206, 318, 175, 58}},
            {eSpriteID::LASER, {837, 643, 11, 22}},
            {eSpriteID::STAR, {798, 372, 84, 73}},
            {eSpriteID::HEART, {804, 461, 79, 70}},
    };

    /**
     * @brief Returns the "damaged" version of a brick sprite.
     *
     * This function is used when a brick is hit and its health reaches zero.
     * It maps the original intact brick sprite to its corresponding damaged version.
     *
     * @param spriteID The original brick sprite ID.
     * @return The damaged version of the sprite, or the original if not a known brick type.
     */
    eSpriteID getBrokenVersion(eSpriteID spriteID) {
        switch (spriteID) {
            case eSpriteID::BRICK_BLUE:
                return eSpriteID::BRICK_BLUE_DMG;
            case eSpriteID::BRICK_PURPLE:
                return eSpriteID::BRICK_PURPLE_DMG;
            case eSpriteID::BRICK_YELLOW:
                return eSpriteID::BRICK_YELLOW_DMG;
            case eSpriteID::BRICK_ORANGE:
                return eSpriteID::BRICK_ORANGE_DMG;
            default:
                return spriteID;
        }
    }

    /**
     * @brief Checks for axis-aligned bounding box (AABB) collision between two entities.
     *
     * This function uses the positions and colliders of two entities to determine
     * if they are overlapping on both X and Y axes.
     *
     * @param a Position of the first entity.
     * @param ca Collider (width and height) of the first entity.
     * @param b Position of the second entity.
     * @param cb Collider of the second entity.
     * @return true if the entities are colliding, false otherwise.
     */
    bool isColliding(const Position& a, const Collider& ca, const Position& b, const Collider& cb) {
        return (
                a.x < b.x + cb.width &&
                a.x + ca.width > b.x &&
                a.y < b.y + cb.height &&
                a.y + ca.height > b.y
        );
    }

    //----------------------------------
    /// @section System Implementations
    //----------------------------------

    /**
    * @brief Handles timing of break animations and destroys the entity when the animation is complete.
    *
    * Any entity with a BreakAnimation component will have its timer incremented each frame.
    * Once the timer exceeds 0.555 seconds, the entity is marked for destruction (if not already).
    *
    * @param deltaTime Time since last frame (in seconds)
    */
    void BreakAnimationSystem(float deltaTime) {
        bagel::Mask breakMask;
        breakMask.set(bagel::Component<BreakAnimation>::Bit);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type entity{id};
            if (!bagel::World::mask(entity).test(breakMask)) continue;
            if (bagel::World::mask(entity).test(bagel::Component<DestroyedTag>::Bit)) continue;

            auto& anim = bagel::World::getComponent<BreakAnimation>(entity);
            anim.timer += deltaTime;

            if (anim.timer >= 0.555f) {
                bagel::World::addComponent(entity, breakout::DestroyedTag{});
            }
        }
    }

    /**
     * @brief Updates positions of entities that have both Position and Velocity components.
     *
     * This system moves all entities based on their Velocity and handles collision with
     * Checks for laser entities that move outside the top of the screen and marks them for destruction.
     */
    void MovementSystem() {
        bagel::Mask required;
        required.set(bagel::Component<Position>::Bit);
        required.set(bagel::Component<Velocity>::Bit);

        constexpr float SCREEN_WIDTH = 800.0f;

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};

            if (!bagel::World::mask(ent).test(required)) continue;

            // Skip entities marked for destruction
            if (bagel::World::mask(ent).test(bagel::Component<DestroyedTag>::Bit)) continue;

            auto& pos = bagel::World::getComponent<Position>(ent);
            auto& vel = bagel::World::getComponent<Velocity>(ent);
            auto& collider = bagel::World::getComponent<Collider>(ent);

            // Move entity
            pos.x += vel.dx;
            pos.y += vel.dy;

            // Check if it's a laser that moved off-screen (above)
            if (bagel::World::mask(ent).test(bagel::Component<LaserTag>::Bit)) {
                if (pos.y + collider.height < 0) {
                    bagel::World::addComponent(ent, breakout::DestroyedTag{});
                    continue;
                }
            }
        }
    }

    /**
    * @brief Detects and handles collisions between entities in the game world.
    *
    * This system checks for collisions between entities that have both Position and Collider components.
    * It supports the following interaction types:
    * - Laser vs Brick: decreases brick health, adds BreakAnimation, marks brick for destruction.
    * - Ball vs Brick: same as laser, with bounce effect.
    * - Ball vs Paddle: inverts Y velocity of the ball.
    * - Ball vs Floor: marks the ball for destruction.
    * - Ball vs Star: gives laser power-up to paddle and destroys the star.
    * - Ball vs Heart: gives wide paddle power-up to paddle and destroys the heart.
    *
    * Notes:
    * - Entities marked with DestroyedTag are skipped.
    * - Collision detection is axis-aligned bounding box (AABB).
    *
    * Requirements:
    * - Components: Position, Collider
    */
    void CollisionSystem() {
        bagel::Mask requiredMask;
        requiredMask.set(bagel::Component<Position>::Bit);
        requiredMask.set(bagel::Component<Collider>::Bit);

        for (bagel::id_type id1 = 0; id1 <= bagel::World::maxId().id; ++id1) {
            bagel::ent_type e1{id1};
            if (!bagel::World::mask(e1).test(requiredMask)) continue;

            // ====== Laser vs Brick ======
            if (bagel::World::mask(e1).test(bagel::Component<LaserTag>::Bit)) {
                for (bagel::id_type id2 = 0; id2 <= bagel::World::maxId().id; ++id2) {
                    if (id1 == id2) continue;

                    bagel::ent_type e2{id2};
                    if (!bagel::World::mask(e2).test(requiredMask)) continue;
                    if (!bagel::World::mask(e2).test(bagel::Component<BrickHealth>::Bit)) continue;
                    if (bagel::World::mask(e2).test(bagel::Component<DestroyedTag>::Bit)) continue;

                    auto& p1 = bagel::World::getComponent<Position>(e1);
                    auto& c1 = bagel::World::getComponent<Collider>(e1);
                    auto& p2 = bagel::World::getComponent<Position>(e2);
                    auto& c2 = bagel::World::getComponent<Collider>(e2);

                    if (!isColliding(p1, c1, p2, c2)) continue;

                    std::cout << "Laser hit brick!\n";

                    auto& brick = bagel::World::getComponent<BrickHealth>(e2);
                    if (brick.hits <= 0) continue;
                    brick.hits--;

                    if (brick.hits <= 0) {
                        auto& sprite = bagel::World::getComponent<Sprite>(e2);
                        sprite.spriteID = getBrokenVersion(sprite.spriteID);

                        if (!bagel::World::mask(e2).test(bagel::Component<BreakAnimation>::Bit)) {
                            bagel::World::addComponent(e2, breakout::BreakAnimation{0.5f});
                        }
                    }
                }
                continue;
            }

            // ====== Ball collisions ======
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

                // --- Ball hits Brick ---
                if (bagel::World::mask(e2).test(bagel::Component<BrickHealth>::Bit)) {
                    auto& brick = bagel::World::getComponent<BrickHealth>(e2);
                    if (brick.hits <= 0) continue;

                    std::cout << "Ball hit brick! Entity: " << e2.id
                              << ", Remaining hits: " << brick.hits << "\n";

                    brick.hits--;

                    auto& vel = bagel::World::getComponent<Velocity>(e1);
                    vel.dy *= -1;

                    if (brick.hits <= 0) {
                        auto& sprite = bagel::World::getComponent<Sprite>(e2);
                        sprite.spriteID = getBrokenVersion(sprite.spriteID);

                        if (!bagel::World::mask(e2).test(bagel::Component<BreakAnimation>::Bit)) {
                            bagel::World::addComponent(e2, breakout::BreakAnimation{0.5f});
                        }
                    }

                    auto& phys = bagel::World::getComponent<PhysicsBody>(e1);
                    b2Vec2 v = b2Body_GetLinearVelocity(phys.body);
                    v.y *= -1;
                    b2Body_SetLinearVelocity(phys.body, v);
                    break;
                }

                // --- Ball hits Paddle ---
                if (bagel::World::mask(e2).test(bagel::Component<PaddleControl>::Bit)) {
                    std::cout << "Ball hit paddle! Inverting Y velocity.\n";
                    auto& phys = bagel::World::getComponent<PhysicsBody>(e1);
                    b2Vec2 v = b2Body_GetLinearVelocity(phys.body);
                    v.y = -std::abs(v.y);
                    b2Body_SetLinearVelocity(phys.body, v);
                    break;
                }

                // --- Ball hits Floor ---
                if (bagel::World::mask(e2).test(bagel::Component<FloorTag>::Bit)) {
                    std::cout << "Ball hit the floor!\n";
                    if (!bagel::World::mask(e1).test(bagel::Component<DestroyedTag>::Bit)) {
                        bagel::World::addComponent(e1, breakout::DestroyedTag{});
                    }
                    break;
                }

                // --- Ball hits Star ---
                if (bagel::World::mask(e2).test(bagel::Component<StarPowerTag>::Bit)) {
                    std::cout << "Ball hit star! Paddle gains laser power.\n";

                    for (bagel::id_type pid = 0; pid <= bagel::World::maxId().id; ++pid) {
                        bagel::ent_type paddle{pid};
                        if (bagel::World::mask(paddle).test(bagel::Component<PaddleControl>::Bit)) {
                            bagel::World::addComponent(paddle, breakout::PowerUpType{ePowerUpType::SHOOTING_LASER});
                            bagel::World::addComponent(paddle, breakout::TimedEffect{0.8f});
                            break;
                        }
                    }

                    bagel::World::addComponent(e2, breakout::DestroyedTag{});
                    auto& phys = bagel::World::getComponent<PhysicsBody>(e1);
                    b2Vec2 v = b2Body_GetLinearVelocity(phys.body);
                    v.y *= -1;
                    b2Body_SetLinearVelocity(phys.body, v);
                    break;
                }

                // --- Ball hits Heart ---
                if (bagel::World::mask(e2).test(bagel::Component<HeartPowerTag>::Bit)) {
                    std::cout << "Ball hit heart! Paddle becomes wider.\n";

                    for (bagel::id_type pid = 0; pid <= bagel::World::maxId().id; ++pid) {
                        bagel::ent_type paddle{pid};
                        if (bagel::World::mask(paddle).test(bagel::Component<PaddleControl>::Bit)) {
                            bagel::World::addComponent(paddle, breakout::PowerUpType{breakout::ePowerUpType::WIDE_PADDLE});
                            bagel::World::addComponent(paddle, breakout::TimedEffect{3.0f});
                            break;
                        }
                    }

                    bagel::World::addComponent(e2, breakout::DestroyedTag{});
                    auto& phys = bagel::World::getComponent<PhysicsBody>(e1);
                    b2Vec2 v = b2Body_GetLinearVelocity(phys.body);
                    v.y *= -1;
                    b2Body_SetLinearVelocity(phys.body, v);
                    break;
                }
            }
        }
    }

    /**
     * @brief Handles keyboard input and updates paddle position accordingly.
     *
     * This system iterates over all entities that have both Position and PaddleControl components,
     * reads the current state of the keyboard, and moves paddles left or right based on the
     * corresponding key bindings.
     *
     * The paddle's horizontal position is clamped to ensure it stays within the screen bounds.
     *
     * Requirements:
     * - Components: Position, PaddleControl
     *
     * Notes:
     * - Uses SDL_GetKeyboardState to access current key states.
     * - Assumes paddle width is 161 * 0.7f and screen width is 800.
     */
    void PlayerControlSystem() {
        constexpr float SCREEN_WIDTH = 800.0f;
        constexpr float MAX_SPEED = 6.0f; // adjust as needed

        bagel::Mask mask;
        mask.set(bagel::Component<PaddleControl>::Bit);
        mask.set(bagel::Component<Position>::Bit);
        mask.set(bagel::Component<Collider>::Bit);

        SDL_PumpEvents();
        const bool* keys = SDL_GetKeyboardState(nullptr);

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (!bagel::World::mask(ent).test(mask)) continue;

            const auto& control = bagel::World::getComponent<PaddleControl>(ent);
            auto& pos = bagel::World::getComponent<Position>(ent);
            const auto& col = bagel::World::getComponent<Collider>(ent);

            float vx = 0.0f;
            if (keys[control.keyLeft])  vx -= MAX_SPEED;
            if (keys[control.keyRight]) vx += MAX_SPEED;

            pos.x += vx;

            // Clamp to screen bounds
            if (pos.x < 0) pos.x = 0;
            if (pos.x + col.width > SCREEN_WIDTH)
                pos.x = SCREEN_WIDTH - col.width;
        }
    }

    /**
    * @brief For each entity with PhysicsBody + Position, sync the position from Box2D.
    */
    void PhysicsSystem(float deltaTime) {
        using namespace bagel;

        constexpr float BOX_STEP = 1.0f / 60.0f;

        // Step the Box2D world
        b2World_Step(boxWorld, BOX_STEP, 8);

        bagel::Mask mask;
        mask.set(Component<PhysicsBody>::Bit);
        mask.set(Component<Position>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};

            if (!World::mask(ent).test(mask)) continue;

            auto& phys = World::getComponent<PhysicsBody>(ent);
            auto& pos = World::getComponent<Position>(ent);

            // Safety check
            if (!b2Body_IsValid(phys.body)) continue;

            // Get transform from Box2D
            b2Transform t = b2Body_GetTransform(phys.body);

            // Convert from meters to pixels (scale = 10)
            pos.x = t.p.x * 10.0f;
            pos.y = t.p.y * 10.0f;
        }
    }

    /**
     * @brief Handles timed power-up effects for entities, such as laser shooting and wide paddle.
     *
     * This system processes all entities that have:
     * - PowerUpType (specifying the type of power-up)
     * - TimedEffect (duration remaining)
     * - Position (for laser spawning)
     * - PaddleControl (indicating the entity is a paddle)
     *
     * For each matching entity:
     * - Decreases the remaining time (`TimedEffect`) by `deltaTime`.
     * - If time runs out:
     *   - Removes power-up components.
     *   - Resets paddle size if it had the WIDE_PADDLE effect.
     * - If the power-up is SHOOTING_LASER:
     *   - Fires two lasers periodically using a cooldown timer.
     * - If the power-up is WIDE_PADDLE:
     *   - Widens the paddle (once only).
     *
     * @param deltaTime Time (in seconds) since last frame, used for updating timers and cooldowns.
     *
     * Notes:
     * - Entities marked with DestroyedTag are ignored.
     * - Laser fire rate is currently hardcoded to 0.05 seconds between shots.
     * - Paddle width is reset to 100.0f when wide power-up expires.
     */
    void PowerUpSystem(float deltaTime) {
        using namespace bagel;

        static float laserCooldown = 0.0f;

        // Required components: power-up info, timer, paddle position and control
        Mask required;
        required.set(Component<PowerUpType>::Bit);
        required.set(Component<TimedEffect>::Bit);
        required.set(Component<Position>::Bit);
        required.set(Component<PaddleControl>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};

            // Skip entities without required components or marked for destruction
            if (!World::mask(ent).test(required)) continue;
            if (World::mask(ent).test(Component<DestroyedTag>::Bit)) continue;

            auto& effect = World::getComponent<TimedEffect>(ent);
            auto& power = World::getComponent<PowerUpType>(ent);

            // Update timer
            effect.remaining -= deltaTime;

            // If timer expired, remove power-up and reset properties
            if (effect.remaining <= 0.0f) {
                std::cout << "Power-up expired.\n";

                if (power.powerUp == breakout::ePowerUpType::WIDE_PADDLE) {
                    if (World::mask(ent).test(Component<Collider>::Bit)) {
                        auto& col = World::getComponent<Collider>(ent);
                        col.width = 100.0f; // Reset paddle width
                        std::cout << "Paddle size restored.\n";
                    }
                }

                World::delComponent<PowerUpType>(ent);
                World::delComponent<TimedEffect>(ent);
                continue;
            }

            // Laser power-up: fires two lasers every X seconds
            if (power.powerUp == ePowerUpType::SHOOTING_LASER) {
                laserCooldown -= deltaTime;
                if (laserCooldown <= 0.0f) {
                    const auto& pos = World::getComponent<Position>(ent);
                    std::cout << "Laser fired!\n";
                    CreateLaser(pos.x + 10, pos.y);     // left
                    CreateLaser(pos.x + 80, pos.y);     // right
                    laserCooldown = 0.05f; // adjust as needed
                }
            }

            // Wide paddle effect: widen only once
            if (power.powerUp == breakout::ePowerUpType::WIDE_PADDLE) {
                auto& col = World::getComponent<Collider>(ent);
                if (col.width < 500.0f) {

                    std::cout << "Paddle widened.\n";
                }
            }
        }
    }

    /**
     * @brief Removes all entities marked with the DestroyedTag from the game world and
     * destroy their Box2D physics body (if they have one).
     *
     * Notes:
     * - Components are not deleted using the ECS's built-in removal function; instead,
     *   the mask is manually cleared bit-by-bit for safety and control.
     */
    void DestroySystem() {
        bagel::Mask required;
        required.set(bagel::Component<DestroyedTag>::Bit);

        std::vector<bagel::ent_type> toDestroy;

        for (bagel::id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (bagel::World::mask(ent).test(required)) {
                toDestroy.push_back(ent);
            }
        }

        for (auto ent : toDestroy) {
            std::cout << "Destroying entity (manual bit clear): " << ent.id << "\n";

            if (bagel::World::mask(ent).test(bagel::Component<PhysicsBody>::Bit)) {
                auto& phys = bagel::World::getComponent<PhysicsBody>(ent);
                if (b2Body_IsValid(phys.body)) {
                    b2DestroyBody(phys.body);
                }
            }

            // Get non-const reference to the actual mask
            bagel::Mask& mask = bagel::World::maskMutable(ent);

            int bitIndex = mask.ctz();
            while (bitIndex >= 0) {
                mask.clear(bagel::Mask::bit(bitIndex));
                bitIndex = mask.ctz();
            }
        }
    }

    /**
     * @brief Renders all entities that have both Position and Sprite components.
     *
     * For paddles with PowerUpType::WIDE_PADDLE, the sprite is visually scaled wider and centered.
     * Other sprites are drawn normally, with fixed scaling (0.7 or 0.4 for ball).
     *
     * @param ren The SDL renderer
     * @param tex The texture containing all sprite graphics
     */
    void RenderSystem(SDL_Renderer* ren, SDL_Texture* tex) {
        using namespace bagel;

        Mask required;
        required.set(Component<Position>::Bit);
        required.set(Component<Sprite>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(required)) continue;

            const auto& pos = World::getComponent<Position>(ent);
            const auto& sprite = World::getComponent<Sprite>(ent);

            auto it = SPRITE_ATLAS.find(sprite.spriteID);
            if (it == SPRITE_ATLAS.end()) continue;

            const SDL_FRect& src = it->second;
            float scale = 0.7f;
            float scaledW = src.w * scale;
            float scaledH = src.h * scale;
            float drawX = pos.x;
            float drawY = pos.y;

            // Special scaling for the ball
            if (sprite.spriteID == eSpriteID::BALL) {
                scale = 0.4f;
                scaledW = src.w * scale;
                scaledH = src.h * scale;
            }

            // Special scaling for wide paddle (visual only)
            if (World::mask(ent).test(Component<PaddleControl>::Bit)) {
                if (World::mask(ent).test(Component<PowerUpType>::Bit)) {
                    const auto& power = World::getComponent<PowerUpType>(ent);
                    if (power.powerUp == breakout::ePowerUpType::WIDE_PADDLE) {
                        scale = 1.5f; // or 2.0f depending how wide you want
                        scaledW = src.w * scale;
                        scaledH = src.h * 0.7f;

                        // Center the paddle visually
                        drawX = pos.x - (scaledW - src.w * 0.7f) / 2.0f;
                    }
                }
            }

            SDL_FRect dst = {drawX, drawY, scaledW, scaledH};
            SDL_RenderTexture(ren, tex, &src, &dst);
        }
    }


    //----------------------------------
    /// @section Entity Creation Functions
    //----------------------------------

    /**
     * @brief Creates a new ball entity with visual, collision, and physics components.
     *
     * Components added:
     * - Position: Starting location on screen
     * - Sprite: Ball graphic
     * - Collider: Used for AABB collision checks
     * - BallTag: Identifies the entity as a ball
     * - PhysicsBody: Box2D body with circular shape and velocity
     *
     * @return Unique ID of the created ball entity.
     */
    id_type CreateBall() {
        bagel::Entity e = bagel::Entity::create();

        Position pos{400.0f, 450.0f};
        Sprite sprite{eSpriteID::BALL};
        Collider collider{87.0f * 0.4f, 77.0f * 0.4f};

                // Box2D body setup
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = true;
        bodyDef.position = {pos.x / 10.0f, pos.y / 10.0f};  // divide by scale
        b2BodyId body = b2CreateBody(boxWorld, &bodyDef);


        b2ShapeDef ballShapeDef = b2DefaultShapeDef();
        ballShapeDef.enableSensorEvents = true;
        ballShapeDef.density = 1;
        ballShapeDef.material.friction = 0;
        ballShapeDef.material.restitution = 1.0f; // the ball became too quick

        b2Circle circle = {0, 0, (87.0f * 0.4f / 2.0f) / 10.0f}; // radius in meters
        b2CreateCircleShape(body, &ballShapeDef, &circle);

        b2Vec2 velocity = {7.0f, -10.0f};
        b2Body_SetLinearVelocity(body, velocity);
        b2Body_SetUserData(body, new bagel::ent_type{e.entity()});

        e.addAll(pos, sprite, collider, BallTag{}, PhysicsBody{body}
        );

        return e.entity().id;
    }

    /**
     * @brief Creates a new brick entity with position, brickHealth sprite, and collision.
     *
     * @return Unique entity ID
     */
    id_type CreateBrick(int health, eSpriteID color, float x, float y) {
        bagel::Entity e = bagel::Entity::create();
        Collider collider{171.0f * 0.7f, 59.0f * 0.7f};
        e.addAll(Position{x,y}, Sprite{color}, collider, BrickHealth{health});
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
     id_type CreatePaddle(int leftKey, int rightKey) {
         bagel::Entity e = bagel::Entity::create();

         float paddleWidth = 161.0f * 0.7f;
         float paddleHeight = 55.0f * 0.7f;

         Position pos{320.0f, 560.0f};
         Sprite sprite{eSpriteID::PADDLE};
         Collider collider{paddleWidth, paddleHeight};
         PaddleControl control{leftKey, rightKey};

         e.addAll(pos, sprite, collider, control);
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
    * @brief Creates the walls - only Box2d not entities.
     */
    void CreateWalls() {
        constexpr float screenW = 800.0f;
        constexpr float screenH = 600.0f;
        constexpr float scale = 10.0f;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;

        // Top wall
        bodyDef.position = {screenW / 2.0f / 10.0f, -1.0f}; // y = -10px
        b2BodyId top = b2CreateBody(boxWorld, &bodyDef);
        b2Polygon topBox = b2MakeBox(screenW / 2.0f / 10.0f, 1.0f);
        b2CreatePolygonShape(top, &shapeDef, &topBox);

        // Left wall
        bodyDef.position = {-1.0f, screenH / 2.0f / 10.0f};
        b2BodyId left = b2CreateBody(boxWorld, &bodyDef);
        b2Polygon leftBox = b2MakeBox(1.0f, screenH / 2.0f / 10.0f);
        b2CreatePolygonShape(left, &shapeDef, &leftBox);

        // Right wall
        float halfWallW = 1.0f;
        float wallX = (screenW / scale) - halfWallW;
        bodyDef.position = {wallX, screenH / 2.0f / scale};
        b2BodyId right = b2CreateBody(boxWorld, &bodyDef);
        b2Polygon rightBox = b2MakeBox(halfWallW, screenH / 2.0f / scale);
        b2CreatePolygonShape(right, &shapeDef, &rightBox);
    }

    id_type CreateStar(float x, float y) {
        bagel::Entity e = bagel::Entity::create();

        Position pos{x, y};
        Sprite sprite{eSpriteID::STAR};
        Collider collider{84.0f * 0.7f, 73.0f * 0.7f};

        e.addAll(pos, sprite, collider, StarPowerTag{});
        return e.entity().id;
    }

    id_type CreateHeart(float x, float y) {
        bagel::Entity e = bagel::Entity::create();

        Position pos{x, y};
        Sprite sprite{eSpriteID::HEART};
        Collider collider{84.0f * 0.7f, 73.0f * 0.7f};

        e.addAll(pos, sprite, collider, HeartPowerTag{});
        return e.entity().id;
    }

    /**
     * @brief Creates a laser entity that moves upward and destroys bricks on contact.
     *
     * The laser moves slowly (dy = -100) to visually match a "weaker" power-up effect.
     *
     * @param x Horizontal position where the laser is spawned.
     * @param y Vertical position where the laser is spawned.
     * @return The unique ID of the created laser entity.
     */
    id_type CreateLaser(float x, float y) {
        bagel::Entity e = bagel::Entity::create();

        Position pos{x, y};
        Velocity vel{0.0f, -200.0f}; // slower upward movement
        Sprite sprite{eSpriteID::LASER};
        Collider collider{11.0f, 22.0f}; // exact sprite size
        LaserTag tag;

        e.addAll(pos, vel, sprite, collider, tag);
        return e.entity().id;
    }

    //----------------------------------
    /// @section Game Loop
    //----------------------------------

    /**
     * @brief Main game loop for the Breakout ECS-style game.
     *
     * Initializes all core entities (paddle, ball, bricks),
     * and continuously runs input handling, system updates, and rendering each frame.
     * This loop uses deltaTime (elapsed time per frame) to update all time-based systems.
     *
     * @param ren SDL renderer used for drawing game objects.
     * @param tex SDL texture sheet containing all game sprites.
     */
    void run(SDL_Renderer* ren, SDL_Texture* tex) {
        using namespace bagel;

        // === Initialization ===
        PrepareBoxWorld();
        CreateWalls();
        CreatePaddle(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT);
        CreateBall();
        CreateFloor();
        CreateBrickGrid(4, 6, 1); // 4 rows × 6 cols, health = 1

        // Timer and control flag for delayed star spawning
        float elapsedTime = 0.0f;
        bool starSpawned = false;

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

            // === Game logic systems ===
            PlayerControlSystem();     // Move paddle based on user input
            MovementSystem();          // Move entities with velocity
            CollisionSystem();         // Handle collisions (ball-brick, laser-brick, ball-star)

            // deltaTime will be passed in later
            World::step();             // Apply queued component changes

            // === Rendering ===
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);
            RenderSystem(ren, tex);
            SDL_RenderPresent(ren);

            // === Frame limiting (target ~60 FPS) ===
            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < 16) SDL_Delay(16 - frameTime);

            // === Time-based systems ===
            float deltaTime = frameTime / 1000.0f;
            elapsedTime += deltaTime;

            BreakAnimationSystem(deltaTime); // Animate broken bricks
            PowerUpSystem(deltaTime);        // Handle laser timer and shooting
            PhysicsSystem(deltaTime);        // Handle physics world movement
            DestroySystem();                 // Remove entities with DestroyedTag
        }
    }
} //namespace breakout;