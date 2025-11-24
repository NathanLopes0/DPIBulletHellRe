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
        Source/Math.cpp
        Source/Scenes/StageSelect.cpp
        Source/Actors/Buttons/Button.cpp
        Source/Actors/Buttons/Button.h
        Source/Actors/Buttons/StageSelectButton.cpp
        Source/Actors/Buttons/StageSelectButton.h
        Source/Components/DrawComponents/DrawAnimatedComponent.cpp
        Source/Components/DrawComponents/DrawAnimatedComponent.h
        Source/Scenes/Battle/Battle.cpp
        Source/Scenes/Battle/Battle.h
        Source/Actors/Teacher/BossFactory/SallesFactory.cpp
        Source/Actors/Teacher/BossFactory/SallesFactory.h
        Source/Actors/Teacher/Boss.cpp
        Source/Actors/Teacher/Boss.h
        Source/Actors/Teacher/Bosses/Salles.cpp
        Source/Actors/Teacher/Bosses/Salles.h
        Source/Actors/Player/Player.cpp
        Source/Actors/Player/Player.h
        Source/Components/AIComponents/FSMState.cpp
        Source/Components/AIComponents/FSMComponent.cpp
        Source/Actors/Projectile.cpp
        Source/Actors/Projectile.h
        Source/Actors/Player/PlayerProjectile.cpp
        Source/Actors/Player/PlayerProjectile.h
        Source/Actors/Teacher/Bosses/BossesProjectiles/BossProjectile.cpp
        Source/Actors/Teacher/Bosses/BossesProjectiles/BossProjectile.h
        Source/Actors/Teacher/Bosses/BossesProjectiles/SallesBossProjectile.cpp
        Source/Actors/Teacher/Bosses/BossesProjectiles/SallesBossProjectile.h
        Source/Actors/Teacher/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h
        Source/Actors/Teacher/BossFactory/BossProjectileFactory/SallesProjectile1Factory.cpp
        Source/Scenes/Battle/ProjectileManager.cpp
        Source/Scenes/Battle/ProjectileManager.h
        Source/Actors/Teacher/BossFactory/IBossFactory.h
        Source/Actors/Teacher/BossFactory/IBossFactory.cpp
        Source/Attacks/IAttackStrategy.h
        Source/Attacks/Behaviors.cpp
        Source/Attacks/Behaviors.h
        Source/Attacks/BaseStrategies/CircleSpreadAttack.cpp
        Source/Attacks/BaseStrategies/CircleSpreadAttack.h
        Source/Attacks/BaseStrategies/AngledAttack.cpp
        Source/Attacks/BaseStrategies/AngledAttack.h
        Source/Actors/Teacher/BossAttackState.cpp
        Source/Actors/Teacher/BossAttackState.h
        Source/Components/ColliderComponents/ColliderComponent.cpp
        Source/Components/ColliderComponents/ColliderComponent.h
        Source/Scenes/Battle/BattleHUD.cpp
        Source/Scenes/Battle/BattleHUD.h
        Source/Components/DrawComponents/ProgressBarComponent.cpp
        Source/Components/DrawComponents/ProgressBarComponent.h
        Source/Attacks/BaseStrategies/WaveAttack.cpp
        Source/Movements/IMovementStrategy.h
        Source/Movements/MovementStrategies.h
        Source/Movements/MovementStrategies.cpp
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)