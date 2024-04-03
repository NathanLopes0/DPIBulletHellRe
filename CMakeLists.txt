cmake_minimum_required(VERSION 3.27)
project(DPIBulletHellRe)

set(CMAKE_CXX_STANDARD 17)

add_executable(DPIBulletHellRe Source/main.cpp
        Source/Game.cpp
        Source/Game.h
        Source/CSV.h)

find_package(SDL2 2.30.1 EXACT REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)