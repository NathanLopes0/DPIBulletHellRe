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
        Source/Actors/Teachers/BossFactory/SallesFactory.cpp
        Source/Actors/Teachers/BossFactory/SallesFactory.h
        Source/Actors/Teachers/Boss.cpp
        Source/Actors/Teachers/Boss.h
        Source/Actors/Teachers/Bosses/Salles.cpp
        Source/Actors/Teachers/Bosses/Salles.h
        Source/Actors/Player/Player.cpp
        Source/Actors/Player/Player.h
        Source/Components/AIComponents/FSMState.cpp
        Source/Components/AIComponents/FSMComponent.cpp
        Source/Actors/Projectile.cpp
        Source/Actors/Projectile.h
        Source/Actors/Player/PlayerProjectile.cpp
        Source/Actors/Player/PlayerProjectile.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.cpp
        Source/Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/SallesBossProjectile.cpp
        Source/Actors/Teachers/Bosses/BossesProjectiles/SallesBossProjectile.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/SallesProjectile1Factory.cpp
        Source/Scenes/Battle/ProjectileManager.cpp
        Source/Scenes/Battle/ProjectileManager.h
        Source/Actors/Teachers/BossFactory/IBossFactory.h
        Source/Actors/Teachers/BossFactory/IBossFactory.cpp
        Source/Attacks/IAttackStrategy.h
        Source/Attacks/Behaviors.cpp
        Source/Attacks/PathShapes.cpp
        Source/Attacks/PathAim.cpp
        Source/Attacks/PathAim.h
        Source/Attacks/PathShapes.h
        Source/Attacks/Behaviors.h
        Source/Attacks/BaseStrategies/CircleSpreadAttack.cpp
        Source/Attacks/BaseStrategies/CircleSpreadAttack.h
        Source/Attacks/BaseStrategies/AngledAttack.cpp
        Source/Attacks/BaseStrategies/AngledAttack.h
        Source/Actors/Teachers/BossAttackState.cpp
        Source/Actors/Teachers/BossAttackState.h
        Source/Components/ColliderComponents/ColliderComponent.cpp
        Source/Components/ColliderComponents/ColliderComponent.h
        Source/Scenes/Battle/BattleHUD.cpp
        Source/Scenes/Battle/BattleHUD.h
        Source/Components/DrawComponents/ProgressBarComponent.cpp
        Source/Components/DrawComponents/ProgressBarComponent.h
        Source/Attacks/BaseStrategies/WaveAttack.cpp
        Source/Movements/MovementStrategies.h
        Source/Movements/MovementStrategies.cpp
        Source/Actors/Teachers/ExtraPointItem.cpp
        Source/Actors/Teachers/ExtraPointItem.h
        Source/Actors/Teachers/BossFactory/AllFactories.h
        Source/Actors/Teachers/BossFactory/RicardoFactory.cpp
        Source/Actors/Teachers/BossFactory/RicardoFactory.h
        Source/Actors/Teachers/Bosses/Ricardo.cpp
        Source/Actors/Teachers/Bosses/Ricardo.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/RicardoBossProjectile.cpp
        Source/Actors/Teachers/Bosses/BossesProjectiles/RicardoBossProjectile.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/RicardoProjectile1Factory.cpp
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/RicardoProjectile1Factory.h
        Source/Actors/Teachers/Bosses/Andre.cpp
        Source/Actors/Teachers/Bosses/Andre.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/AndreBossProjectile.cpp
        Source/Actors/Teachers/Bosses/BossesProjectiles/AndreBossProjectile.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/AndreProjectile1Factory.cpp
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/AndreProjectile1Factory.h
        Source/Actors/Teachers/BossFactory/AndreFactory.cpp
        Source/Actors/Teachers/Bosses/Julio.cpp
        Source/Actors/Teachers/Bosses/Julio.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/JulioBossProjectile.h
        Source/Actors/Teachers/BossFactory/JulioFactory.cpp
        Source/Actors/Teachers/BossFactory/JulioFactory.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/JulioProjectile1Factory.cpp
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/JulioProjectile1Factory.h
        Source/Actors/Teachers/BossFactory/AndreFactory.h
        Source/Attacks/BaseStrategies/BaloonAttack.cpp
        Source/Attacks/BaseStrategies/BaloonAttack.h
        Source/Attacks/AttackParameters/BaloonAttackParams.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/AndreBaloonProjectile.cpp
        Source/Actors/Teachers/Bosses/BossesProjectiles/AndreBaloonProjectile.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/AndreBaloonProjectileFactory.cpp
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/AndreBaloonProjectileFactory.h
        Source/Attacks/BaseStrategies/LaserAttack.cpp
        Source/Attacks/BaseStrategies/LaserAttack.h
        Source/Actors/Teachers/Bosses/BossesProjectiles/SallesDoubleListProjectile.cpp
        Source/Actors/Teachers/Bosses/BossesProjectiles/SallesDoubleListProjectile.h
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/SallesDoubleListProjectileFactory.cpp
        Source/Actors/Teachers/BossFactory/BossProjectileFactory/SallesDoubleListProjectileFactory.h
        Source/Actors/ProjectilePool.h
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)
# =============================================================================
# TESTES UNITARIOS (doctest)
#
# Alvo SEPARADO do jogo: nao depende de SDL e compila em segundos, porque so
# inclui a camada pura (PathAim + Math). Rodar: ctest --test-dir build, ou
# executar dpi_tests direto.
#
# O jogo continua compilando normalmente mesmo que este alvo seja removido.
# =============================================================================
enable_testing()

add_executable(dpi_tests
        tests/test_main.cpp
        tests/test_path_aim.cpp
        tests/test_motion_contract.cpp
        tests/test_investida.cpp
        Source/Attacks/PathAim.cpp
        Source/Attacks/PathAim.h
        Source/Math.cpp
        Source/Math.h
)
# NAO adicione Source ao include path deste alvo (nem de nenhum outro).
# O projeto tem Source/Math.h, e no Windows (e no macOS) o sistema de
# arquivos nao diferencia maiusculas: com Source no include path, o
# #include <math.h> da PROPRIA biblioteca padrao encontra Source/Math.h em
# vez do cabecalho C, e sinf/fabs/sqrtf deixam de existir - centenas de
# erros em cmath e cstdlib. Os arquivos deste alvo ja usam includes
# relativos, entao nao precisam do caminho.

# Faz o doctest incluir os cabecalhos reais da biblioteca padrao em vez de
# declarar std::tuple por conta propria. O MSVC recente avisa (C5285) que
# declarar templates da std fora dela e proibido pelo padrao.
target_compile_definitions(dpi_tests PRIVATE DOCTEST_CONFIG_USE_STD_HEADERS)

add_test(NAME unidade COMMAND dpi_tests)
