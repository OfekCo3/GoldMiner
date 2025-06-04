cmake_minimum_required(VERSION 3.7)
project(BAGEL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# === SDL + Box2D ===
set(SDL_STATIC ON)
set(SDL_SHARED OFF)

add_subdirectory(lib/SDL)
add_subdirectory(lib/SDL_image)
add_subdirectory(lib/box2d)

# === Executable 1: BAGEL ===
add_executable(BAGEL
        bagel.h
        bagel_cfg.h
        main.cpp
        goldMiner/gold_miner_ecs.cpp
        goldMiner/gold_miner_ecs.h
        goldMiner/sprite_manager.cpp
        goldMiner/sprite_manager.h
        )

target_link_libraries(BAGEL PUBLIC SDL3-static SDL3_image-static box2d)

target_include_directories(BAGEL PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/SDL/include
        ${PROJECT_SOURCE_DIR}/lib/SDL_image/include
        )

add_custom_command(
        TARGET BAGEL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:BAGEL>/res"
)

# === Executable 2: TEST_GOLDMINER_VISUAL ===
add_executable(TEST_GOLDMINER_VISUAL
        bagel.h
        bagel_cfg.h
        main_visual.cpp
        goldMiner/gold_miner_ecs.cpp
        goldMiner/gold_miner_ecs.h
        goldMiner/sprite_manager.cpp
        goldMiner/sprite_manager.h
        )

target_link_libraries(TEST_GOLDMINER_VISUAL PUBLIC SDL3-static SDL3_image-static box2d)

target_include_directories(TEST_GOLDMINER_VISUAL PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/SDL/include
        ${PROJECT_SOURCE_DIR}/lib/SDL_image/include
        )

add_custom_command(
        TARGET TEST_GOLDMINER_VISUAL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:TEST_GOLDMINER_VISUAL>/res"
)
