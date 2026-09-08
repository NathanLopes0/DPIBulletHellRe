//
// Fábrica do Júlio (INF 420)
//

#include "JulioFactory.h"

#include "../BossAttackState.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Attacks/BaseStrategies/CircleSpreadAttack.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Movements/MovementStrategies.h"
#include "../Bosses/Julio.h"
#include "BossProjectileFactory/JulioProjectile1Factory.h"

JulioFactory::JulioFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> JulioFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Julio>(scene);
}

void JulioFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHJulio.png",
                                                              "../Assets/Teachers/DPIBHJulio.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // ----- FÁBRICA DE PROJÉTEIS ----- //
    // Um tipo só. Cada tipo registrado custa mais 300 instâncias no
    // pré-aquecimento, então só adicione outro se ele for mesmo diferente.
    boss->AddProjectileFactory("Dados", std::make_unique<JulioProjectile1Factory>());
}

void JulioFactory::ConfigureAttacksAndFSM(Boss* boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Julio nao tem FSMComponent");
        return;
    }

    ConfigureStateOne(boss, fsm);
    ConfigureStateTwo(boss, fsm);
    ConfigureStateThree(boss, fsm);
    ConfigureStateFinal(boss, fsm);

    boss->SetInitialState("StateOne");
}

// ---------------------------------------------------------------------------
// FASE 1 — "Exploração"
//
// O modelo ainda não aprendeu nada: atira em rajadas circulares mal miradas,
// com velocidades desiguais. O jogador tem espaço de sobra para se posicionar.
// Movimento: vagueio aleatório — busca cega pelo espaço de soluções.
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateOne(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateOne";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 16;
    params->projectileSpeed = 165.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(params),
        2.3f,
        [](Projectile* p, const int index) {
            // Velocidades desiguais dentro da mesma rajada: o anel sai
            // deformado, reforçando a ideia de amostragem ruidosa.
            if (index % 3 == 0) {
                p->insertBehavior<AccelerateBehavior>(0.6f, 1.6f);
            } else if (index % 3 == 1) {
                p->insertBehavior<SlowDownBehavior>(0.5f, 0.75f);
            }
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_ONE_DURATION,
                                                      "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<RandomWanderStrategy>(3.5f, 115.f));
}

// ---------------------------------------------------------------------------
// FASE 2 — "Descida do Gradiente"
//
// O modelo converge. A mira é exata e os projéteis corrigem a rota em direção
// ao jogador, com a taxa de correção decaindo até travarem.
// Movimento: o boss "desce" na direção do jogador, encurtando a distância.
//
// A leitura de jogo aqui é: fugir cedo não adianta, porque o projétil corrige;
// tem de esperar a taxa decair e então desviar. Ensina o jogador a ter timing.
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateTwo(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateTwo";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 7;
    params->projectileSpeed = 205.f;
    params->angle = 50.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        1.7f,
        [](Projectile* p, const int index) {
            // Taxa de aprendizado maior nas pontas do leque: os projéteis das
            // bordas fecham mais, o que "afunila" a rajada inteira.
            const float rate = (index == 0 || index == 6) ? 3.4f : 2.2f;
            p->insertBehavior<GradientDescentBehavior>(0.35f, rate, 2.0f);
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_TWO_DURATION,
                                                      "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<HoverAbovePlayerStrategy>(135.f, 170.f));
}

// ---------------------------------------------------------------------------
// FASE 3 — "Overfitting"
//
// O modelo decorou os dados: mira quase exata, mas os projéteis serpenteiam com
// variância alta e só estabilizam no fim do percurso.
// Movimento: o boss se fixa no centro — travou num mínimo local.
//
// Projéteis vizinhos recebem amplitudes de sinal oposto, então se cruzam e
// fecham brechas que pareciam seguras.
//
// nextState vazio: é aqui que a batalha é decidida (ver BossAttackState).
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateThree(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateThree";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 11;
    params->projectileSpeed = 190.f;
    params->angle = 115.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        1.5f,
        [](Projectile* p, const int index) {
            // Sinal alternado da amplitude = projéteis vizinhos oscilando em
            // contrafase, cruzando as trajetórias.
            const float amplitude = (index % 2 == 0) ? 42.f : -42.f;
            p->insertBehavior<OverfitBehavior>(0.15f, amplitude, 1.6f, 2.4f);
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_THREE_DURATION,
                                                      "");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<GoToCenterStrategy>());
}

// ---------------------------------------------------------------------------
// FASE FINAL — "Regularização" (repescagem, nota entre 40 e 60)
//
// PRECISA existir mesmo o Júlio sendo um boss de 3 fases. Se a nota cair nessa
// faixa ao fim da fase 3, a FSM tenta ir para "StateFinal"; sem o estado
// registrado, SetState apenas loga o erro e o boss trava.
//
// Tema: o modelo foi regularizado — ficou mais simples, mais lento e mais
// justo. Também é a fase mais fácil de propósito: é uma segunda chance.
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateFinal(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateFinal";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 12;
    params->projectileSpeed = 150.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    // Sem configurator: projéteis retos, sem curva nem oscilação.
    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(params),
        2.6f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_FINAL_DURATION,
                                                      "StateOne");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<RandomWanderStrategy>(4.5f, 85.f));
}
