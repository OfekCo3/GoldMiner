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
#include "SDL3_image/SDL_image.h"



namespace breakout {

    using id_type = int;

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

    /** @brief Graphical representation (sprite ID) */
    struct Sprite {
        int spriteID = -1; ///< Placeholder ID for graphics (can be replaced with actual sprite reference)
    };

    /** @brief Collider for detecting collisions with other entities */
    struct Collider {
        int radius = 5; ///< Simple circular collider
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

    //----------------------------------
    // Entity creation functions
    //----------------------------------

    /**
     * @brief Creates a ball entity with required components.
     * @return The unique ID of the created entity.
     */
    int CreateBall();

    /**
     * @brief Creates a brick entity with specified durability.
     * @param health Number of hits until the brick breaks.
     * @return The unique ID of the created entity.
     */
    id_type CreateBrick(int health);

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

} // namespace breakout

#endif // BREAKOUT_GAME_H
