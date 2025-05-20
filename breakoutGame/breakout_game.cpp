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

    void PrepareBoxWorld() {
        b2WorldDef def = b2DefaultWorldDef();
        def.gravity = {0.0f, 0.0f};
        boxWorld = b2CreateWorld(&def);
    }

    //----------------------------------
    // System Implementations
    //----------------------------------

   /**
 * @brief Updates positions of entities that have both Position and Velocity components.
 *
 * This system moves all entities based on their Velocity and handles collision with
 * screen bounds (for the ball and paddle). Additionally, it checks for laser entities
 * that move outside the top of the screen and marks them for destruction.
 *
 * It skips entities already marked with DestroyedTag.
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

        // Skip entities marked for destruction
        if (bagel::World::mask(ent).test(bagel::Component<DestroyedTag>::Bit)) continue;

        // Optional debug: warn if a brick has Velocity (should not happen)
        if (bagel::World::mask(ent).test(bagel::Component<BrickHealth>::Bit)) {
            std::cout << "⚠️ Warning: Brick entity " << id << " has Velocity!\n";
        }

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

        // Bounce off left/right walls
        if (pos.x < 0 || pos.x + collider.width > SCREEN_WIDTH) {
            std::cout << "Entity " << id << " hit horizontal wall\n";
            pos.x = std::clamp(pos.x, 0.0f, SCREEN_WIDTH - collider.width);
            vel.dx *= -1;
        }

        // Bounce off top wall
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
 *        Handles ball vs. brick/paddle/floor/star, and laser vs. brick.
 */
void CollisionSystem() {
    bagel::Mask requiredMask;
    requiredMask.set(bagel::Component<Position>::Bit);
    requiredMask.set(bagel::Component<Collider>::Bit);

    for (bagel::id_type id1 = 0; id1 <= bagel::World::maxId().id; ++id1) {
        bagel::ent_type e1{id1};
        if (!bagel::World::mask(e1).test(requiredMask)) continue;

        // Handle laser-brick separately
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
                    sprite.spriteID = static_cast<eSpriteID>(static_cast<int>(sprite.spriteID) + 1);

                    if (!bagel::World::mask(e2).test(bagel::Component<BreakAnimation>::Bit)) {
                        bagel::World::addComponent(e2, breakout::BreakAnimation{0.5f});
                    }
                }

                break;
            }

            continue; // Done handling laser, skip rest
        }

        // === Ball collision starts here ===
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
                    sprite.spriteID = static_cast<eSpriteID>(static_cast<int>(sprite.spriteID) + 1);

                    if (!bagel::World::mask(e2).test(bagel::Component<BreakAnimation>::Bit)) {
                        bagel::World::addComponent(e2, breakout::BreakAnimation{0.5f});
                    }
                }

                break;
            }

            // --- Ball hits Paddle ---
            if (bagel::World::mask(e2).test(bagel::Component<PaddleControl>::Bit)) {
                std::cout << "Ball hit paddle! Inverting Y velocity.\n";
                auto& vel = bagel::World::getComponent<Velocity>(e1);
                vel.dy *= -1;
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
                        bagel::World::addComponent(paddle, breakout::PowerUpType{ePowerUpType::SHOTING_LASER});
                        bagel::World::addComponent(paddle, breakout::TimedEffect{0.8f});
                        break;
                    }
                }

                bagel::World::addComponent(e2, breakout::DestroyedTag{});

                auto& vel = bagel::World::getComponent<Velocity>(e1);
                vel.dy *= -1;
                break;
            }
            // --- Ball hits Heart ---
            if (bagel::World::mask(e2).test(bagel::Component<HeartPowerTag>::Bit)) {
                std::cout << "Ball hit heart! Paddle becomes wider.\n";

                for (bagel::id_type pid = 0; pid <= bagel::World::maxId().id; ++pid) {
                    bagel::ent_type paddle{pid};
                    if (bagel::World::mask(paddle).test(bagel::Component<PaddleControl>::Bit)) {
                        bagel::World::addComponent(paddle, breakout::PowerUpType{breakout::ePowerUpType::WIDE_PADDLE});
                        bagel::World::addComponent(paddle, breakout::TimedEffect{5.0f});
                        break;
                    }
                }

                bagel::World::addComponent(e2, breakout::DestroyedTag{});

                auto& vel = bagel::World::getComponent<Velocity>(e1);
                vel.dy *= -1;
                break;
            }

        }
    }
}

    /**
     * @brief Reads player input and updates paddle position accordingly.
     */
    void PlayerControlSystem() {
        constexpr float SCREEN_WIDTH = 800.0f;
        constexpr float PADDLE_WIDTH = 161.0f * 0.7f;
        constexpr float MAX_SPEED = 20.0f;

        bagel::Mask mask;
        mask.set(bagel::Component<PaddleControl>::Bit);
        mask.set(bagel::Component<PhysicsBody>::Bit);
        mask.set(bagel::Component<Collider>::Bit);

        SDL_PumpEvents();
        const bool* keys = SDL_GetKeyboardState(nullptr);

        for (id_type id = 0; id <= bagel::World::maxId().id; ++id) {
            bagel::ent_type ent{id};
            if (!bagel::World::mask(ent).test(mask)) continue;

            const auto& control = bagel::World::getComponent<PaddleControl>(ent);
            auto& phys = bagel::World::getComponent<PhysicsBody>(ent);
            auto& col = bagel::World::getComponent<Collider>(ent);

            if (!b2Body_IsValid(phys.body)) continue;

            float vx = 0.0f;
            if (keys[control.keyLeft])  vx -= MAX_SPEED;
            if (keys[control.keyRight]) vx += MAX_SPEED;

            b2Vec2 pos = b2Body_GetPosition(phys.body);
            float halfWidth = col.width / 2.0f / 10.0f;
            float minX = halfWidth;
            float maxX = (SCREEN_WIDTH / 10.0f) - halfWidth;

            if ((pos.x <= minX && vx < 0) || (pos.x >= maxX && vx > 0)) {
                vx = 0.0f;
            }

            b2Vec2 newVel = {vx, 0.0f};
            b2Body_SetLinearVelocity(phys.body, newVel);
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
 * @brief Handles time-limited power-up effects for entities, such as laser firing or wide paddle.
 *
 * This system manages active power-ups by decreasing their timers and applying their effects.
 * When time runs out, it removes the effect and resets entity properties (e.g., paddle width).
 *
 * @param deltaTime The time in seconds since the last frame (used to update timers)
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
        if (power.powerUp == ePowerUpType::SHOTING_LASER) {
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
            if (col.width < 180.0f) {
                col.width = 180.0f;
                std::cout << "Paddle widened.\n";
            }
        }
    }
}

    /**
     * @brief Removes entities that are marked for deletion using the DestroyedTag component.
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
        ballShapeDef.material.restitution = 1.1f;

        b2Circle circle = {0, 0, (87.0f * 0.4f / 2.0f) / 10.0f}; // radius in meters
        b2CreateCircleShape(body, &ballShapeDef, &circle);

        b2Vec2 velocity = {10.0f, -15.0f};
        b2Body_SetLinearVelocity(body, velocity);
        b2Body_SetUserData(body, new bagel::ent_type{e.entity()});

        e.addAll(pos, sprite, collider, BallTag{}, PhysicsBody{body}
        ); //without velocity

        return e.entity().id;
    }

    /**
     * @brief Creates a new brick entity with position, brickHealth sprite, and collision.
     *        The ball starts near the center of the screen and moves upward.
     *
     * @return Unique entity ID
     */
    id_type CreateBrick(int health, eSpriteID color, float x, float y) {
        bagel::Entity e = bagel::Entity::create();

        Collider collider{171.0f * 0.7f, 59.0f * 0.7f};

        // Box2D body
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_staticBody;
        def.position = {x / 10.0f, y / 10.0f};

        b2BodyId body = b2CreateBody(boxWorld, &def);

        b2ShapeDef shape = b2DefaultShapeDef();
        shape.density = 1.0f;

        b2Polygon box = b2MakeBox(collider.width / 2 / 10.0f, collider.height / 2 / 10.0f);
        b2CreatePolygonShape(body, &shape, &box);

        b2Body_SetUserData(body, new bagel::ent_type{e.entity()});

        e.addAll(Position{x, y}, Sprite{color}, collider, BrickHealth{health}, PhysicsBody{body});

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
        Sprite sprite{eSpriteID::PADDLE};
        Collider collider{161.0f * 0.7f, 55.0f * 0.7f};
        PaddleControl control{left, right};

         // Box2D body
         b2BodyDef def = b2DefaultBodyDef();
         def.type = b2_kinematicBody;
         def.position = {pos.x / 10.0f, pos.y / 10.0f};

         b2BodyId body = b2CreateBody(boxWorld, &def);

         b2ShapeDef shape = b2DefaultShapeDef();
         shape.density = 1.0f;

         b2Polygon box = b2MakeBox(collider.width / 2 / 10.0f, collider.height / 2 / 10.0f);
         b2CreatePolygonShape(body, &shape, &box);

         b2Body_SetUserData(body, new bagel::ent_type{e.entity()});

         e.addAll(pos, sprite, collider, control, PhysicsBody{body});
        return e.entity().id;

    }

    /**
     * @brief Creates a falling power-up with a defined type and timed effect.
     * @param type Identifier for the power-up effect
     * @return Unique entity ID
     */
    id_type CreatePowerUp(ePowerUpType type) {
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
 * After 5 seconds, a star power-up is spawned that grants laser-shooting
 * ability if collected by the paddle.
 *
 * @param ren SDL renderer for drawing.
 * @param tex Texture sheet for all game sprites.
 */
/**
 * @brief Main game loop for the Breakout ECS-style game.
 *
 * Initializes all core entities (UI, paddle, ball, bricks),
 * and continuously runs input handling, system updates, and rendering each frame.
 *
 * After 0.6 seconds, a special star power-up is spawned. If the ball hits it,
 * the paddle gains laser-shooting ability for 5 seconds.
 *
 * This loop uses deltaTime (elapsed time per frame) to update all time-based systems.
 *
 * @param ren SDL renderer used for drawing game objects.
 * @param tex SDL texture sheet containing all game sprites.
 */
void run(SDL_Renderer* ren, SDL_Texture* tex) {
    using namespace bagel;

    // === Initialization ===
    PrepareBoxWorld();

    CreateUIManager();
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
        MovementSystem();          // Move entities with velocity (ball, lasers)
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
        PhysicsSystem(deltaTime);
        DestroySystem();                 // Remove entities with DestroyedTag


    }
}

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
                    if (sprite.spriteID == eSpriteID::BALL) {
                        dst.w = src.w * 0.4f;  // scale down width
                        dst.h = src.h * 0.4f;  // scale down height
                    }
                    SDL_RenderTexture(ren, tex, &src, &dst);
                }
            }
        }
    }

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

                // Place a static star in the middle of the top row
                if (row == 2 && col == cols / 2) {
                    CreateStar(x, y); // This star does not fall – no velocity
                }else if (row == 3 && col == cols / 2) {
                    CreateHeart(x,y);
                }
                else {
                    CreateBrick(health, color, x, y);
                }
            }
        }
    }


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


    void StarSystem(float deltaTime) {
        using namespace bagel;

        Mask required;
        required.set(Component<Position>::Bit);
        required.set(Component<Velocity>::Bit);
        required.set(Component<Collider>::Bit);
        required.set(Component<StarPowerTag>::Bit);

        Mask paddleMask;
        paddleMask.set(Component<Position>::Bit);
        paddleMask.set(Component<Collider>::Bit);
        paddleMask.set(Component<PaddleControl>::Bit);

        Mask floorMask;
        floorMask.set(Component<Position>::Bit);
        floorMask.set(Component<Collider>::Bit);
        floorMask.set(Component<FloorTag>::Bit);

        for (id_type starID = 0; starID <= World::maxId().id; ++starID) {
            ent_type star{starID};
            if (!World::mask(star).test(required)) continue;

            auto& pos = World::getComponent<Position>(star);
            auto& vel = World::getComponent<Velocity>(star);
            auto& col = World::getComponent<Collider>(star);

            // Move star
            pos.y += vel.dy * deltaTime;

            // Check collision with paddle
            for (id_type pid = 0; pid <= World::maxId().id; ++pid) {
                ent_type paddle{pid};
                if (!World::mask(paddle).test(paddleMask)) continue;

                auto& pPos = World::getComponent<Position>(paddle);
                auto& pCol = World::getComponent<Collider>(paddle);

                if (isColliding(pos, col, pPos, pCol)) {
                    std::cout << "Star hit paddle! Granting laser effect.\n";
                    World::addComponent(paddle, breakout::PowerUpType{ePowerUpType::SHOTING_LASER});
                    World::addComponent(paddle, breakout::TimedEffect{2.0f});
                    World::addComponent(star, breakout::DestroyedTag{});
                    break;
                }
            }

            // Check collision with floor
            for (id_type fid = 0; fid <= World::maxId().id; ++fid) {
                ent_type floor{fid};
                if (!World::mask(floor).test(floorMask)) continue;

                auto& fPos = World::getComponent<Position>(floor);
                auto& fCol = World::getComponent<Collider>(floor);

                if (isColliding(pos, col, fPos, fCol)) {
                    std::cout << "Star hit floor! Removing.\n";
                    World::addComponent(star, breakout::DestroyedTag{});
                    break;
                }
            }
        }
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
} //namespace breakout;