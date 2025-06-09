cmake_minimum_required(VERSION 3.16)
project(BAGEL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# הוספת ~/.local ל-CMAKE_PREFIX_PATH כדי ש-finde_package ימצא את SDL3
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")

# מציאת SDL3 מותקן
find_package(SDL3 REQUIRED COMPONENTS SDL3-shared Headers)

# BUILD EXECUTABLE BAGEL
add_executable(BAGEL
        bagel.h
        bagel_cfg.h
        main.cpp
        goldMiner/gold_miner_ecs.cpp goldMiner/gold_miner_ecs.h
        goldMiner/sprite_manager.cpp goldMiner/sprite_manager.h
)

# BUILD EXECUTABLE TEST_GOLDMINER_VISUAL
add_executable(TEST_GOLDMINER_VISUAL
        bagel.h
        bagel_cfg.h
        main_visual.cpp
        goldMiner/gold_miner_ecs.cpp goldMiner/gold_miner_ecs.h
        goldMiner/sprite_manager.cpp goldMiner/sprite_manager.h
)

# הוספת SDL_image ו-box2d
add_subdirectory(lib/SDL_image)
add_subdirectory(lib/box2d)

# לינקינג מול SDL3 ו־SDL3_image ו־box2d
target_link_libraries(BAGEL PUBLIC SDL3::SDL3 SDL3_image::SDL3_image box2d)
target_link_libraries(TEST_GOLDMINER_VISUAL PUBLIC SDL3::SDL3 SDL3_image::SDL3_image box2d)

# הוספת include directories
target_include_directories(BAGEL PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/SDL_image/include
)

target_include_directories(TEST_GOLDMINER_VISUAL PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/SDL_image/include
)

# העתקת res אחרי build
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
)
