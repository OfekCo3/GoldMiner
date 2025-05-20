/**
 * @file breakout_game.h
 * @brief Breakout-style game module using BAGEL ECS architecture.
 *
 * This module defines the game components, systems, and entity creation functions
 * according to the Entity-Component-System (ECS) model, implemented using BAGEL.
 */
#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include <cstdint>
#include "SDL3_image/SDL_image.h"

namespace breakout {

    using id_type = int;

    /** @brief Enum representing all possible sprite types in the game. */
    enum class eSpriteID {
        BALL = 0,
        PADDLE = 1,
        BRICK_BLUE = 2,
        BRICK_BLUE_DMG = 3,
        BRICK_PURPLE = 4,
        BRICK_PURPLE_DMG = 5,
        BRICK_YELLOW = 6,
        BRICK_YELLOW_DMG = 7,
        BRICK_ORANGE = 8,
        BRICK_ORANGE_DMG = 9,
        LASER = 10,
        STAR = 11,
        HEART = 12,
    };

    /** @brief Types of power-ups available in the game. */
    enum class ePowerUpType {
        NONE = 0,
        SHOTING_LASER = 1,
        WIDE_PADDLE = 2,
    };

    //----------------------------------
    /// @section Components
    //----------------------------------

    /** @brief Position of an entity on the screen (X,Y coordinates). */
    struct Position {
        float x = 0.0f; ///< Horizontal position
        float y = 0.0f; ///< Vertical position
    };

    /** @brief Velocity vector defining movement direction and speed. */
    struct Velocity {
        float dx = 0.0f; ///< Horizontal speed
        float dy = 0.0f; ///< Vertical speed
    };

    /** @brief Graphical representation of the entity. Uses sprite ID from eSpriteID enum. */
    struct Sprite {
        eSpriteID spriteID = eSpriteID::BALL;  ///< Which sprite to use when rendering
    };

    /** @brief Collider for detecting collisions with other entities. */
    struct Collider {
        float width = 0.0f;
        float height = 0.0f;
    };

    /** @brief Tracks how many hits a brick can take before breaking. */
    struct BrickHealth {
        int hits = 1; ///< Number of remaining hits (1 by default)
    };

    /** @brief Indicates paddle is controlled by player; includes control keys. */
    struct PaddleControl {
        int keyLeft = SDL_SCANCODE_LEFT;  ///< Key code to move left
        int keyRight = SDL_SCANCODE_RIGHT; ///< Key code to move right
    };

    /** @brief Tag component to identify the ball. */
    struct BallTag {};

    /** @brief Type of power-up available or collected. */
    struct PowerUpType {
        ePowerUpType powerUp = ePowerUpType::NONE;
    };

    /** @brief Temporary effect applied to the entity, with duration. */
    struct TimedEffect {
        float remaining = 0.0f; ///< Remaining time for power-up
    };

    /** @brief Marks an entity to be removed from the game. */
    struct DestroyedTag {};

    /** @brief Tag component to mark the floor. */
    struct FloorTag {};

    /** @brief Component used to animate a brick before it's destroyed. */
    struct BreakAnimation {
        float timer = 0.0f; ///< Countdown before removal (in seconds)
    };

    /** @brief Tag for star power-up. */
    struct StarPowerTag {};

    /** @brief Tag for heart power-up. */
    struct HeartPowerTag {};

    /** @brief Tag to identify laser entities. */
    struct LaserTag {};

    //----------------------------------
    /// @section Systems (declarations only)
    //----------------------------------

    /** @brief Updates entity positions based on velocity components. */
    void MovementSystem();

    /** @brief Handles collisions between entities and triggers side effects. */
    void CollisionSystem();

    /** @brief Handles player input and updates paddle position accordingly. */
    void PlayerControlSystem();

    /**
     * @brief Activates power-up logic and tracks timed effects.
     *
     * @param deltaTime Time elapsed since last frame.
     */
    void PowerUpSystem(float deltaTime);

    /** @brief Removes entities marked with DestroyedTag. */
    void DestroySystem();

    /** @brief Displays game information (score, lives, etc.) to the user. */
    void UISystem();

    /**
     * @brief Renders all entities that have both Position and Sprite components.
     *
     * @param ren The SDL renderer to use.
     * @param tex The texture sheet containing all sprites.
     */
    void RenderSystem(SDL_Renderer* ren, SDL_Texture* tex);

    /**
     * @brief Handles the animation of bricks breaking over time.
     *
     * @param deltaTime Time elapsed since last frame.
     */
    void BreakAnimationSystem(float deltaTime);

    /**
     * @brief Handles logic when a star power-up is active.
     *
     * @param deltaTime Time elapsed since last frame.
     */
    void StarSystem(float deltaTime);

    //----------------------------------
    /// @section Entity creation functions
    //----------------------------------

    /** @brief Creates a ball entity with required components.
     *  @return The unique ID of the created entity.
     */
    id_type CreateBall();

    /**
     * @brief Creates a brick entity with specific health, sprite color, and position.
     *
     * @param health Number of hits until the brick breaks.
     * @param color eSpriteID enum value to determine brick color.
     * @param x Horizontal position of the brick.
     * @param y Vertical position of the brick.
     * @return The unique ID of the created entity.
     */
    id_type CreateBrick(int health, eSpriteID color, float x, float y);

    /**
     * @brief Creates a paddle controlled by the player.
     *
     * @param left Key code for moving left.
     * @param right Key code for moving right.
     * @return The unique ID of the created entity.
     */
    id_type CreatePaddle(int left, int right);

    /**
     * @brief Creates a falling power-up with a specified type.
     *
     * @param type Power-up type identifier.
     * @return The unique ID of the created entity.
     */
    id_type CreatePowerUp(int type);

    /**
     * @brief Creates a floor entity that detects when the ball falls below.
     *
     * @return The unique ID of the created entity.
     */
    id_type CreateFloor();

    /**
     * @brief Creates a star power-up entity at the specified position.
     *
     * @param x Horizontal position.
     * @param y Vertical position.
     * @return The unique ID of the created star entity.
     */
    id_type CreateStar(float x, float y);

    /**
     * @brief Creates a full grid of bricks arranged in rows and columns.
     *
     * @param rows Number of brick rows.
     * @param cols Number of bricks per row.
     * @param health Health value assigned to each brick.
     */
    void CreateBrickGrid(int rows, int cols, int health);

    /**
     * @brief Creates a heart power-up entity at the specified position.
     *
     * @param x Horizontal position.
     * @param y Vertical position.
     * @return The unique ID of the created heart entity.
     */
    id_type CreateHeart(float x, float y);

    /**
     * @brief Creates a laser entity at the specified position.
     *
     * @param x Horizontal position.
     * @param y Vertical position.
     * @return The unique ID of the created laser entity.
     */
    id_type CreateLaser(float x, float y);

    /**
     * @brief Runs the main game loop or core execution logic.
     *
     * @param ren The SDL renderer.
     * @param tex The texture sheet.
     */
    void run(SDL_Renderer* ren, SDL_Texture* tex);

} // namespace breakout

#endif // BREAKOUT_GAME_H
