/**
 * @file breakout_game.h
 * @brief Breakout-style game module using BAGEL ECS architecture.
 *
 * This module defines the game components, systems, and entity creation functions
 * according to the Entity-Component-System (ECS) model, implemented using BAGEL.
 * All functions and structures are documented with Doxygen-style comments
 * to support understanding and future development.
 */
#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include <cstdint>
//#include "SDL3_image/SDL_image.h"
#include "SDL3_image/SDL_image.h"



namespace breakout {

    using id_type = int;

    /** @brief Enum representing all possible sprite types in the game. */
    enum class SpriteID {
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
        LASER = 10
    };

    //----------------------------------
    // Components
    //----------------------------------

    /** @brief Position of an entity on the screen (X,Y coordinates) */
    struct Position {
        float x = 0.0f; ///< Horizontal position
        float y = 0.0f; ///< Vertical position
    };

    /** @brief Velocity vector defining movement direction and speed */
    struct Velocity {
        float dx = 0.0f; ///< Horizontal speed
        float dy = 0.0f; ///< Vertical speed
    };

    /** @brief Graphical representation of the entity. Uses sprite ID from SpriteID enum. */
    struct Sprite {
        SpriteID spriteID = SpriteID::BALL;  ///< Which sprite to use when rendering
    };


    /** @brief Collider for detecting collisions with other entities */
    struct Collider {
        float width = 0.0f;
        float height = 0.0f;
    };

    /** @brief Tracks how many hits a brick can take before breaking */
    struct BrickHealth {
        int hits = 1; ///< Number of remaining hits (1 by default)
    };

    /** @brief Indicates paddle is controlled by player; includes control keys */
    struct PaddleControl {
        int keyLeft = -1;  ///< Key code to move left
        int keyRight = -1; ///< Key code to move right
    };

    /** @brief Tag component to identify the ball */
    struct BallTag {};

    /** @brief Type of power-up available or collected */
    struct PowerUpType {
        int type = 0; ///< 0 = none, 1 = wide paddle, 2 = multi-ball, etc.
    };

    /** @brief Temporary effect applied to the entity, with duration */
    struct TimedEffect {
        float duration = 0.0f; ///< Duration of the temporary effect in seconds
    };

    /** @brief Number of lives remaining for the player */
    struct LifeCount {
        int lives = 3; ///< Default lives count
    };

    /** @brief Current score accumulated by the player */
    struct Score {
        int points = 0; ///< Score value
    };

    /** @brief Marks an entity to be removed from the game */
    struct DestroyedTag {};

    /** @brief Tag component to mark the floor */
    struct FloorTag {};

    struct BreakAnimation {
        float timer = 0.0f; //seconds
    };

    //----------------------------------
    // Systems (declarations only)
    //----------------------------------

    /**
     * @brief Updates entity positions based on velocity components.
     */
    void MovementSystem();

    /**
     * @brief Handles collisions between entities and triggers side effects.
     */
    void CollisionSystem();

    /**
     * @brief Handles player input and updates paddle position accordingly.
     */
    void PlayerControlSystem();

    /**
     * @brief Activates power-up logic and tracks timed effects.
     */
    void PowerUpSystem();

    /**
     * @brief Removes entities marked with DestroyedTag.
     */
    void DestroySystem();

    /**
     * @brief Displays game information (score, lives, etc.) to the user.
     */
    void UISystem();

     /**
     * @brief Renders all entities that have both Position and Sprite components.
     *
     * @param ren The SDL renderer to use.
     * @param tex The texture sheet containing all sprites.
     */
    void RenderSystem(SDL_Renderer* ren, SDL_Texture* tex);

    void BreakAnimationSystem(float deltaTime);


    //----------------------------------
    // Entity creation functions
    //----------------------------------

    /**
     * @brief Creates a ball entity with required components.
     * @return The unique ID of the created entity.
     */
    int CreateBall();

    /**
     * @brief Creates a brick entity with specific health, sprite color, and position.
     * @param health Number of hits until the brick breaks.
     * @param color SpriteID enum value to determine brick color.
     * @param x Horizontal position of the brick.
     * @param y Vertical position of the brick.
     * @return The unique ID of the created entity.
     */
    id_type CreateBrick(int health, SpriteID color, float x, float y);


    /**
     * @brief Creates a paddle controlled by the player.
     * @param left Key code for moving left.
     * @param right Key code for moving right.
     * @return The unique ID of the created entity.
     */
    id_type CreatePaddle(int left, int right);

    /**
     * @brief Creates a falling power-up with a specified type.
     * @param type Power-up type identifier.
     * @return The unique ID of the created entity.
     */
     id_type CreatePowerUp(int type);

    /**
     * @brief Creates a static UI manager to track score and lives.
     * @return The unique ID of the created entity.
     */
     id_type CreateUIManager();


     void run(SDL_Renderer* ren, SDL_Texture* tex);

    /**
    * @brief Creates a floor entity that detects when the ball falls below.
    * @return The unique ID of the created entity.
    */
    id_type CreateFloor();

    /**
     * @brief Creates a full grid of bricks arranged in rows and columns.
     *
     * @param rows Number of brick rows.
     * @param cols Number of bricks per row.
     * @param health Health value assigned to each brick.
     */
    void CreateBrickGrid(int rows, int cols, int health);

} // namespace breakout

#endif // BREAKOUT_GAME_H
