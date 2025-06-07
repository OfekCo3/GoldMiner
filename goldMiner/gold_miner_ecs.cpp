/**
 * @file gold_miner_ecs.cpp
 * @brief Implementation of entity creation functions and systems using BAGEL ECS for Gold Miner game.
 */
#include "gold_miner_ecs.h"
#include "../bagel.h"
#include "sprite_manager.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include <iostream>

namespace goldminer {
    b2WorldId gWorld = b2_nullWorldId;
    using namespace bagel;

//----------------------------------
/// @section Entity Creation Functions
//----------------------------------
//----------------------------------

//----------------------------------
/// @section Entity Creation Functions
//----------------------------------

/**
 * @brief Creates a new player entity with base components.
 * @param playerID The identifier of the player.
 * @return The ID of the created entity.
 */
    id_type CreatePlayer(int playerID) {
        Entity e = Entity::create();
        e.addAll(Position{570.0f, 10.0f}, Velocity{}, Renderable{SPRITE_PLAYER_IDLE}, PlayerInfo{playerID}, Score{0}, PlayerInput{});
        return e.entity().id;
    }

/**
 * @brief Creates a dynamic rope entity with a narrow rectangle body for collision testing.
 *
 * This version of the rope includes a Box2D dynamic body, allowing it to interact
 * with static objects such as gold, rocks, and treasure chests. The shape is a
 * narrow vertical rectangle (thin rope), with collision enabled.
 *
 * Components added:
 * - Position: top-left (for rendering)
 * - Rotation: (optional for future use)
 * - Length: logical rope length
 * - RopeControl, RoperTag: identify as rope
 * - Collidable: enables collision system
 * - PhysicsBody: Box2D body handle
 * - PlayerInfo: for multi-player logic
 *
 * @param playerID The rope's owning player
 * @return Entity ID
 */
    id_type CreateRope(int playerID) {
        Entity e = Entity::create();

        // Visual size in pixels
        float ropeW = 6.0f;
        float ropeH = 120.0f;

        float startX = 120.0f;
        float startY = 540.0f;

        constexpr float PPM = 50.0f;

        float centerX = startX + ropeW / 2.0f;
        float centerY = startY + ropeH / 2.0f;

        float hw = ropeW / 2.0f / PPM;
        float hh = ropeH / 2.0f / PPM;

        // Create dynamic Box2D body
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {centerX / PPM, centerY / PPM};

        b2BodyId bodyId = b2CreateBody(gWorld, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.5f;

        b2Polygon box = {};
        box.count = 4;
        box.vertices[0] = { -hw, -hh };
        box.vertices[1] = {  hw, -hh };
        box.vertices[2] = {  hw,  hh };
        box.vertices[3] = { -hw,  hh };

        b2CreatePolygonShape(bodyId, &shapeDef, &box);
        b2Body_SetUserData(bodyId, new bagel::ent_type{e.entity()});

        e.addAll(
                Position{startX, startY},
                Rotation{0.0f},
                Length{ropeH},
                RopeControl{},
                RoperTag{},
                PlayerInfo{playerID},
                Collidable{},
                PhysicsBody{bodyId}
        );

        return e.entity().id;
    }

/**
 * @brief Creates a gold item at the given coordinates.
 */
    id_type CreateGold(float x, float y) {
        Entity e = Entity::create();

        // Get sprite dimensions for gold
        SDL_Rect rect = GetSpriteSrcRect(SPRITE_GOLD);
        float width = static_cast<float>(rect.w);
        float height = static_cast<float>(rect.h);

        // Calculate center of sprite for Box2D positioning
        float centerX = x + width / 2.0f;
        float centerY = y + height / 2.0f;

        constexpr float PPM = 50.0f; // Pixels per meter

        // Define a static Box2D body at the gold's center
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {centerX / PPM, centerY / PPM};

        b2BodyId bodyId = b2CreateBody(gWorld, &bodyDef);

        // Create a circle shape for the gold body
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.3f;
        shapeDef.material.restitution = 0.1f;

        b2Circle circle;
        circle.center = {0.0f, 0.0f};
        circle.radius = (width / 2.0f) / PPM;

        b2CreateCircleShape(bodyId, &shapeDef, &circle);

        // Attach entity ID to body for reference if needed
        b2Body_SetUserData(bodyId, new bagel::ent_type{e.entity()});

        // Add ECS components to the entity
        e.addAll(
                Position{x, y},
                Renderable{SPRITE_GOLD},
                Collectable{},
                ItemType{ItemType::Type::Gold},
                Value{100},
                Weight{1.0f},
                Collidable{},
                PlayerInfo{-1},
                PhysicsBody{bodyId}
        );

        return e.entity().id;
    }


/**
 * @brief Creates a rock entity using a 6-vertex polygon to match its visual shape.
 *
 * This function generates a static physics body shaped like the visible rock sprite.
 * The hitbox uses 6 vertices to closely follow the stone's curved silhouette,
 * allowing more accurate collision detection than a simple rectangle.
 *
 * Components added:
 * - Position: screen-space top-left pixel coordinates
 * - Renderable: uses SPRITE_ROCK
 * - Collidable: for collision detection
 * - PhysicsBody: holds the Box2D body
 * - ItemType: set to Rock
 * - Value, Weight, PlayerInfo: standard properties
 *
 * @param x The top-left x position on screen in pixels.
 * @param y The top-left y position on screen in pixels.
 * @return The ID of the created entity.
 */
    id_type CreateRock(float x, float y) {
        Entity e = Entity::create();

        SDL_Rect rect = GetSpriteSrcRect(SPRITE_ROCK);
        float width = static_cast<float>(rect.w);
        float height = static_cast<float>(rect.h);

        float centerX = x + width / 2.0f;
        float centerY = y + height / 2.0f;

        constexpr float PPM = 50.0f; // Pixels per meter
        float hw = width / 2.0f / PPM;
        float hh = height / 2.0f / PPM;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {centerX / PPM, centerY / PPM};

        b2BodyId bodyId = b2CreateBody(gWorld, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.6f;
        shapeDef.material.restitution = 0.1f;

        // Six-point polygon approximating the rock's shape
        b2Vec2 verts[6] = {
                { -hw * 0.6f, -hh * 0.8f }, // p0 - top-left
                { -hw * 0.9f,  0.0f       }, // p1 - far left
                { -hw * 0.5f,  hh * 0.9f  }, // p2 - bottom-left
                {  hw * 0.6f,  hh * 0.9f  }, // p3 - bottom-right
                {  hw * 0.9f,  0.0f       }, // p4 - far right
                {  hw * 0.4f, -hh * 0.8f  }  // p5 - top-right
        };

        b2Polygon shape = {};
        shape.count = 6;
        for (int i = 0; i < 6; ++i) {
            shape.vertices[i] = verts[i];
        }

        b2CreatePolygonShape(bodyId, &shapeDef, &shape);
        b2Body_SetUserData(bodyId, new bagel::ent_type{e.entity()});

        e.addAll(
                Position{x, y},
                Renderable{SPRITE_ROCK},
                Collectable{},
                ItemType{ItemType::Type::Rock},
                Value{10},
                Weight{3.0f},
                Collidable{},
                PlayerInfo{-1},
                PhysicsBody{bodyId}
        );

        return e.entity().id;
    }


/**
 * @brief Creates a diamond entity using a 6-vertex polygon matching the visual sprite shape.
 *
 * The diamond is represented as a static Box2D body shaped like a stylized gemstone:
 * flat top, tapered sides, and a pointed bottom.
 * This allows more accurate collision detection than a circle or rectangle.
 *
 * Components added:
 * - Position: top-left pixel coordinate (for rendering)
 * - Renderable: uses SPRITE_DIAMOND
 * - Collidable: for collision
 * - PhysicsBody: holds Box2D body ID
 * - ItemType: set to Diamond
 * - Value, Weight, PlayerInfo: game logic metadata
 *
 * @param x Top-left x pixel coordinate
 * @param y Top-left y pixel coordinate
 * @return ID of the created entity
 */
    id_type CreateDiamond(float x, float y) {
        Entity e = Entity::create();

        SDL_Rect rect = GetSpriteSrcRect(SPRITE_DIAMOND);
        float width = static_cast<float>(rect.w);  // ~41 px
        float height = static_cast<float>(rect.h); // ~32 px

        float centerX = x + width / 2.0f;
        float centerY = y + height / 2.0f;

        constexpr float PPM = 50.0f;
        float hw = width / 2.0f / PPM;
        float hh = height / 2.0f / PPM;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {centerX / PPM, centerY / PPM};

        b2BodyId bodyId = b2CreateBody(gWorld, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.4f;
        shapeDef.material.restitution = 0.2f;

        // Six-point diamond shape
        b2Vec2 verts[6] = {
                { 0.0f,     -hh },         // top center
                { -hw,      -hh * 0.5f },  // top-left
                { -hw * 0.7f, hh * 0.1f }, // mid-left
                { 0.0f,      hh },         // bottom point
                { hw * 0.7f, hh * 0.1f },  // mid-right
                { hw,       -hh * 0.5f }   // top-right
        };

        b2Polygon shape = {};
        shape.count = 6;
        for (int i = 0; i < 6; ++i) {
            shape.vertices[i] = verts[i];
        }

        b2CreatePolygonShape(bodyId, &shapeDef, &shape);
        b2Body_SetUserData(bodyId, new bagel::ent_type{e.entity()});

        e.addAll(
                Position{x, y},
                Renderable{SPRITE_DIAMOND},
                Collectable{},
                ItemType{ItemType::Type::Diamond},
                Value{300},
                Weight{0.5f},
                Collidable{},
                PlayerInfo{-1},
                PhysicsBody{bodyId}
        );

        return e.entity().id;
    }

    /**
 * @brief Creates a treasure chest entity with scaled rendering and accurate physics shape.
 *
 * This function instantiates a static ECS entity representing a treasure chest.
 * The chest uses a scaled-down version of the original sprite (SCALE = 0.23)
 * so that it visually matches the size of other objects (like rocks or gold).
 *
 * The hitbox is defined as a 6-vertex polygon that closely matches the chest's visible outline.
 * No scaling is applied later in rendering or physics — all dimensions are pre-scaled here.
 *
 * Components added:
 * - Position: top-left pixel position (already scaled)
 * - Renderable: uses SPRITE_TREASURE_CHEST
 * - Collidable: participates in collision detection
 * - PhysicsBody: stores the Box2D body handle
 * - ItemType: set to TreasureChest
 * - Value, Weight, PlayerInfo: standard item properties
 *
 * @param x Top-left x position in pixels (screen coordinates)
 * @param y Top-left y position in pixels (screen coordinates)
 * @return ID of the created entity
 */

    id_type CreateTreasureChest(float x, float y) {
        Entity e = Entity::create();

        SDL_Rect rect = GetSpriteSrcRect(SPRITE_TREASURE_CHEST);
        float originalW = static_cast<float>(rect.w); // 356
        float originalH = static_cast<float>(rect.h); // 447

        constexpr float PPM = 50.0f;

        float scaledW = originalW;
        float scaledH = originalH   ;

        float centerX = x + scaledW / 2.0f;
        float centerY = y + scaledH / 2.0f;

        float hw = scaledW / 2.0f / PPM;
        float hh = scaledH / 2.0f / PPM;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {centerX / PPM, centerY / PPM};

        b2BodyId bodyId = b2CreateBody(gWorld, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.5f;
        shapeDef.material.restitution = 0.1f;

        // Six-point polygon representing the chest outline (scaled)
        b2Vec2 verts[6] = {
                { -hw * 0.9f, -hh * 0.5f },   // top-left
                { -hw * 0.7f,  hh * 0.4f },   // bottom-left
                {  0.0f,       hh * 0.6f },   // center-bottom
                {  hw * 0.7f,  hh * 0.4f },   // bottom-right
                {  hw * 0.9f, -hh * 0.5f },   // top-right
                {  0.0f,      -hh * 0.8f }    // top-center arch
        };

        b2Polygon shape = {};
        shape.count = 6;
        for (int i = 0; i < 6; ++i) {
            shape.vertices[i] = verts[i];
        }

        b2CreatePolygonShape(bodyId, &shapeDef, &shape);
        b2Body_SetUserData(bodyId, new bagel::ent_type{e.entity()});

        // Final position stored already scaled
        e.addAll(
                Position{x, y},
                Renderable{SPRITE_TREASURE_CHEST},
                Collectable{},
                ItemType{ItemType::Type::TreasureChest},
                Value{0},
                Weight{1.0f},
                Collidable{},
                PlayerInfo{-1},
                PhysicsBody{bodyId}
        );

        return e.entity().id;
    }


    /**
 * @brief Creates a mystery bag item at the given coordinates.
 */
    id_type CreateMysteryBag(float x, float y) {
        Entity e = Entity::create();

        SDL_Rect rect = GetSpriteSrcRect(SPRITE_MYSTERY_BAG);
        float width = static_cast<float>(rect.w);
        float height = static_cast<float>(rect.h);

        float centerX = x + width / 2.0f;
        float centerY = y + height / 2.0f;

        constexpr float PPM = 50.0f;
        float hw = width / 2.0f / PPM;
        float hh = height / 2.0f / PPM;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {centerX / PPM, centerY / PPM};

        b2BodyId bodyId = b2CreateBody(gWorld, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.4f;
        shapeDef.material.restitution = 0.2f;

        // Five-point polygon that mimics the mystery sack
        b2Vec2 verts[5] = {
                { 0.0f, -hh * 0.9f },    // top (tie)
                { -hw * 0.8f, -hh * 0.3f }, // upper left
                { -hw, hh * 0.6f },        // bottom left
                { hw, hh * 0.6f },         // bottom right
                { hw * 0.8f, -hh * 0.3f }  // upper right
        };

        b2Polygon sackShape = {};
        sackShape.count = 5;
        for (int i = 0; i < 5; ++i) {
            sackShape.vertices[i] = verts[i];
        }

        b2CreatePolygonShape(bodyId, &shapeDef, &sackShape);
        b2Body_SetUserData(bodyId, new bagel::ent_type{e.entity()});

        e.addAll(
                Position{x, y},
                Renderable{SPRITE_MYSTERY_BAG},
                Collectable{},
                ItemType{ItemType::Type::MysteryBag},
                Value{0},
                Weight{1.0f},
                Collidable{},
                PlayerInfo{-1},
                PhysicsBody{bodyId}
        );

        return e.entity().id;
    }

/**
 * @brief Creates the game timer entity.
 */
    id_type CreateTimer() {
        Entity e = Entity::create();
        e.add(GameTimer{60.0f});
        return e.entity().id;
    }

/**
 * @brief Creates a UI entity for a given player.
 */
    id_type CreateUIEntity(int playerID) {
        Entity e = Entity::create();
        e.addAll(UIComponent{0}, PlayerInfo{playerID});
        return e.entity().id;
    }

/**
 * @brief Creates a mole entity at the given position.
 */
    id_type CreateMole(float x, float y) {
        Entity e = Entity::create();
        e.addAll(Position{x, y}, Velocity{1.5f, 0.0f}, Renderable{5}, Mole{100.0f, true}, Collidable{});
        return e.entity().id;
    }

//----------------------------------
/// @section System Implementations (Skeletons)
//----------------------------------

/**
 * @brief Reads player input and stores it in PlayerInput component.
 */
    void PlayerInputSystem() {
        Mask mask;
        mask.set(Component<PlayerInfo>::Bit);
        mask.set(Component<PlayerInput>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Oscillates rope entities that are currently at rest.
 */
    void RopeSwingSystem() {
        Mask mask;
        mask.set(Component<RoperTag>::Bit);
        mask.set(Component<Rotation>::Bit);
        mask.set(Component<RopeControl>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Handles rope extension and retraction logic.
 */
    void RopeExtensionSystem() {
        Mask mask;
        mask.set(Component<RoperTag>::Bit);
        mask.set(Component<RopeControl>::Bit);
        mask.set(Component<Length>::Bit);
        mask.set(Component<Position>::Bit);
        mask.set(Component<PlayerInfo>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Detects collisions between the rope and collectible items.
 *
 * This system finds collisions between entities marked with RoperTag
 * (the rope) and entities that have ItemType and are collectable.
 * If a collision is detected, it prints the rope and item entity IDs.
 *
 * Requirements:
 * - Rope: PhysicsBody, Collidable, RoperTag
 * - Item: PhysicsBody, Collidable, ItemType
 */
    void CollisionSystem() {
        b2ContactEvents events = b2World_GetContactEvents(gWorld);

        for (int i = 0; i < events.beginCount; ++i) {
            b2ContactBeginTouchEvent contact = events.beginEvents[i];

            b2ShapeId a = contact.shapeIdA;
            b2ShapeId b = contact.shapeIdB;

            b2BodyId bodyA = b2Shape_GetBody(a);
            b2BodyId bodyB = b2Shape_GetBody(b);

            auto *entA = static_cast<bagel::ent_type *>(b2Body_GetUserData(bodyA));
            auto *entB = static_cast<bagel::ent_type *>(b2Body_GetUserData(bodyB));

            if (!entA || !entB) continue;

            bool aIsRope = bagel::World::mask(*entA).test(bagel::Component<RoperTag>::Bit);
            bool bIsRope = bagel::World::mask(*entB).test(bagel::Component<RoperTag>::Bit);
            bool aIsItem = bagel::World::mask(*entA).test(bagel::Component<ItemType>::Bit);
            bool bIsItem = bagel::World::mask(*entB).test(bagel::Component<ItemType>::Bit);

            if ((aIsRope && bIsItem) || (bIsRope && aIsItem)) {
                std::cout << "Collision: Rope hit item!" << std::endl;
            }
        }
    }

/**
 * @brief Pulls collected items towards the player.
 */
    void PullObjectSystem() {
        Mask mask;
        mask.set(Component<Collidable>::Bit);
        mask.set(Component<Position>::Bit);

        Mask optional;
        optional.set(Component<RoperTag>::Bit);
        optional.set(Component<Collectable>::Bit);
        optional.set(Component<ItemType>::Bit);
        optional.set(Component<PlayerInfo>::Bit);
        optional.set(Component<Weight>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Adds score to players based on collected items.
 */
    void ScoreSystem() {
        Mask mask;
        mask.set(Component<ItemType>::Bit);
        mask.set(Component<PlayerInfo>::Bit);

        Mask optional;
        optional.set(Component<Value>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Assigns random value to mystery bag items when collected.
 */
    void TreasureChestSystem() {
        Mask mask;
        mask.set(Component<PlayerInfo>::Bit);
        mask.set(Component<Value>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Renders all entities with a position and sprite.
 */
    void RenderSystem(SDL_Renderer* renderer) {
        using namespace bagel;
        using namespace goldminer;

        Mask mask;
        mask.set(Component<Renderable>::Bit);
        mask.set(Component<Position>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;

            const Position& pos = World::getComponent<Position>(ent);
            const Renderable& render = World::getComponent<Renderable>(ent);

            if (render.spriteID < 0 || render.spriteID >= SPRITE_COUNT) continue;

            SDL_Rect rect = GetSpriteSrcRect(static_cast<SpriteID>(render.spriteID));
            SDL_Texture* texture = GetSpriteTexture(static_cast<SpriteID>(render.spriteID));

            SDL_FRect src = {
                static_cast<float>(rect.x),
                static_cast<float>(rect.y),
                static_cast<float>(rect.w),
                static_cast<float>(rect.h)
            };

            SDL_FRect dest = {
                pos.x,
                pos.y,
                src.w,
                src.h
            };

            SDL_RenderTexture(renderer, texture, &src, &dest);
        }
    }

/**
 * @brief Returns the visual center offset of a sprite based on its ID.
 *
 * Given a sprite ID, this function retrieves its source rectangle and computes
 * half of its width and height (in pixels), scaled as needed.
 * This is used to convert from Box2D center-based coordinates to SDL top-left positioning.
 *
 * @param spriteID The sprite ID from the Renderable component.
 * @return SDL_FPoint containing {half width, half height} of the sprite.
 */
    SDL_FPoint GetSpriteOffset(int spriteID) {
        SDL_Rect rect = GetSpriteSrcRect(static_cast<SpriteID>(spriteID));
        float width = static_cast<float>(rect.w);
        float height = static_cast<float>(rect.h);

        constexpr float scale = 1.0f; // Adjust this if rendering uses a scale

        return SDL_FPoint{
                (width * scale) / 2.0f,
                (height * scale) / 2.0f
        };
    }

/**
 * @brief Synchronizes ECS Position components with their Box2D physics bodies.
 *
 * This system updates the Position (in pixels, top-left) of entities that have
 * both PhysicsBody and Renderable components. It uses the Box2D transform (center-based)
 * and applies an offset based on the sprite's size to align rendering with SDL.
 *
 * Requirements:
 * - Components: PhysicsBody, Position, Renderable
 *
 * Notes:
 * - Assumes PIXELS_PER_METER is defined globally.
 * - This system is essential for aligning sprite rendering with physics movement.
 */
    void PhysicsSyncSystem() {
        using namespace bagel;

        constexpr float PIXELS_PER_METER = 50.0f;

        Mask mask;
        mask.set(Component<PhysicsBody>::Bit);
        mask.set(Component<Position>::Bit);
        mask.set(Component<Renderable>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;

            auto& phys = World::getComponent<PhysicsBody>(ent);
            auto& pos = World::getComponent<Position>(ent);
            const auto& render = World::getComponent<Renderable>(ent);

            if (!b2Body_IsValid(phys.bodyId)) continue;

            b2Transform transform = b2Body_GetTransform(phys.bodyId);
            SDL_FPoint offset = GetSpriteOffset(render.spriteID);

            pos.x = transform.p.x * PIXELS_PER_METER - offset.x;
            pos.y = transform.p.y * PIXELS_PER_METER - offset.y;
        }
    }

    /**
 * @brief Updates the global game timer.
 */
    void TimerSystem() {
        Mask mask;
        mask.set(Component<GameTimer>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Displays score and time for each player.
 */
    void UISystem() {
        Mask mask;
        mask.set(Component<UIComponent>::Bit);

        Mask optional;
        optional.set(Component<Score>::Bit);
        optional.set(Component<PlayerInfo>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Controls the mole's horizontal movement.
 */
    void MoleSystem() {
        Mask mask;
        mask.set(Component<Mole>::Bit);
        mask.set(Component<Position>::Bit);
        mask.set(Component<Velocity>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }

/**
 * @brief Removes entities with a lifetime timer that expired.
 */
    void LifeTimeSystem() {
        Mask mask;
        mask.set(Component<LifeTime>::Bit);

        for (id_type id = 0; id <= World::maxId().id; ++id) {
            ent_type ent{id};
            if (!World::mask(ent).test(mask)) continue;
            // No logic implemented yet
        }
    }


} // namespace goldminer
