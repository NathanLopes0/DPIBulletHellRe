cmake_minimum_required(VERSION 3.26)
project(DPIBulletHellRe)

set(CMAKE_CXX_STANDARD 17)

add_executable(DPIBulletHellRe Source/main.cpp
        Source/Game.cpp
        Source/Game.h
        Source/CSV.h
        Source/Scenes/Scene.cpp
        Source/Scenes/Scene.h
        Source/Scenes/Scene.h
        Source/Actors/Actor.cpp
        Source/Actors/Actor.h
        Source/Components/Component.cpp
        Source/Components/Component.h
        Source/Components/RigidBodyComponent.cpp
        Source/Components/RigidBodyComponent.h
        Source/Components/DrawComponents/DrawComponent.cpp
        Source/Components/DrawComponents/DrawComponent.h
        Source/Components/DrawComponents/DrawSpriteComponent.cpp
        Source/Components/DrawComponents/DrawSpriteComponent.h
        Source/Components/DrawComponents/DrawTextComponent.cpp
        Source/Components/DrawComponents/DrawTextComponent.h
        Source/Components/ColliderComponents/CircleColliderComponent.cpp
        Source/Components/ColliderComponents/CircleColliderComponent.h
        Source/Scenes/MainMenu.cpp
        Source/Scenes/MainMenu.h
        Source/Font.cpp
        Source/Font.h
        Source/AudioSystem.cpp
        Source/AudioSystem.h
        Source/Random.cpp
        Source/Actors/Teacher/Boss.cpp
        Source/Actors/Teacher/Boss.h
        Source/Math.cpp
        Source/Scenes/StageSelect.cpp
        Source/Actors/Buttons/Button.cpp
        Source/Actors/Buttons/Button.h
        Source/Actors/Buttons/StageSelectButton.cpp
        Source/Actors/Buttons/StageSelectButton.h
        Source/Components/DrawComponents/DrawAnimatedComponent.cpp
        Source/Components/DrawComponents/DrawAnimatedComponent.h)

find_package(SDL2 2.28.2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)