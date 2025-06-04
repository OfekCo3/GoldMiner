cmake_minimum_required(VERSION 3.7)
project(BAGEL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(BAGEL
        bagel.h
        bagel_cfg.h
        main.cpp
        goldMiner/gold_miner_ecs.cpp goldMiner/gold_miner_ecs.h
        goldMiner/sprite_manager.cpp goldMiner/sprite_manager.h
)

add_executable(TEST_GOLDMINER_VISUAL
        bagel.h
        bagel_cfg.h
        main_visual.cpp
        goldMiner/gold_miner_ecs.cpp goldMiner/gold_miner_ecs.h
        goldMiner/sprite_manager.cpp goldMiner/sprite_manager.h
)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
add_subdirectory(lib/SDL)
add_subdirectory(lib/SDL_image)
add_subdirectory(lib/box2d)

target_link_libraries(BAGEL PUBLIC SDL3-static SDL3_image-static box2d)
target_link_libraries(TEST_GOLDMINER_VISUAL PUBLIC SDL3-static SDL3_image-static box2d)

target_include_directories(BAGEL PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/SDL/include
        ${PROJECT_SOURCE_DIR}/lib/SDL_image/include
)

target_include_directories(TEST_GOLDMINER_VISUAL PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/SDL/include
        ${PROJECT_SOURCE_DIR}/lib/SDL_image/include
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
)
