
#include "SallesFactory.h"
#include "../../Teacher/Bosses/Salles.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Attacks/BaseStrategies/CircleSpreadAttack.h"
#include "../../../Actors/Teacher/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h"
#include "../../../Attacks/Behaviors.h"
#include "../../Projectile.h"
#include "../../../Random.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"

SallesFactory::SallesFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> SallesFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Salles>(scene);
}

void SallesFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHSalles.png",
                                                                                "../Assets/Teachers/DPIBHSalles.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");


    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    boss->AddComponent<CircleColliderComponent>(colliderRadius);


}


void SallesFactory::ConfigureAttacksAndFSM(Boss* boss) {

    SDL_Log("Instalando a SallesProjectileFactory no Boss...");
    boss->SetProjectileFactory(std::make_unique<SallesProjectile1Factory>());

    // A "chave" que vamos usar.
    const std::string STATE_NAME = "DefaultAttack";

    // --- PASSO 1: Criar os Parâmetros ---
    AttackParams params;
    params.numProjectiles = 30;     // Vamos atirar 30 balas
    params.projectileSpeed = 200.0f;  // Com velocidade 250


    // Agora, guardamos essa receita no "livro" do Boss
    boss->AddAttackTemplate(STATE_NAME, params);

    // --- PASSO 2: Definir o Cooldown ---
    // (Conserta o .at(stateName) no mapa de Cooldowns)
    boss->AddAttackCooldown(STATE_NAME, 2.0f); // Atirar a cada 2 segundos

    // --- PASSO 3: Criar a "Arma" (A Estratégia) ---
    // (A lógica que já tínhamos e estava correta)
    Actor* owner = boss;
    ProjectileFactory* spawner = boss->GetProjectileFactory();

    if (!spawner) {
        SDL_Log("ERRO CRÍTICO: SallesFactory não encontrou o ProjectileFactory no Boss!");
        return;
    }

    // 1. Criamos a "Máquina" (burra)
    auto circleStrategy = std::make_unique<CircleSpreadAttack>(spawner, owner);

    // 2. Criamos a "Lógica Customizada"
    const ProjectileConfigurator sallesHomingLogic =
        [](Projectile* p, const int index) {
    if (const float homingChance = Random::GetFloatRange(0,1); homingChance < 0.8) {
                // (Usando HomingBehavior)
                p->insertBehavior<HomingBehavior>(0.8f, 500.f);
            }
    };

    // 3. Adicionamos o "Passo de Ataque" (Máquina + Lógica Customizada)
    boss->AddAttackStep(STATE_NAME, std::move(circleStrategy), sallesHomingLogic);

    SDL_Log("SallesFactory::ConfigureAttacksAndFSM CONCLUÍDO. O Boss está armado.");
}