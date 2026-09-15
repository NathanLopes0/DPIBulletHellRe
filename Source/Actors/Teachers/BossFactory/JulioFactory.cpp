//
// Fábrica do Júlio (INF 420)
//

#include "JulioFactory.h"

#include "../BossAttackState.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Attacks/BaseStrategies/CircleSpreadAttack.h"
#include "../../../Attacks/BaseStrategies/WaveAttack.h"
#include "../../../Attacks/PathShapes.h"
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
// FASE 1 - "Esta te procurando"
//
// Varredura: um leque largo que acorda projetil por projetil, varrendo a tela
// de um lado ao outro. Le como um scanner passando, que e o que um calouro
// espera de "a IA esta te procurando".
//
// Trocado de CircleSpreadAttack para WaveAttack de proposito: a fase 1 do
// Ricardo JA e um anel de CircleSpread com RandomWander, e a versao anterior
// desta fase era indistinguivel dela. Dois chefes diferentes precisam abrir a
// luta de formas diferentes, ou o jogador acha que e o mesmo jogo.
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateOne(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateOne";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 13;
    params->projectileSpeed = 175.f;
    params->angle = 150.f;

    // Atraso entre um projetil e o seguinte. 13 x 0.055 = varredura de ~0.7s,
    // rapida o bastante para ler como um movimento unico e nao como 13 tiros.
    params->creationSpeed = 0.055f;

    auto spawner = boss->GetProjectileFactory("Dados");

    // Sem configurator: o escalonamento da propria WaveAttack ja da o carater
    // da fase. Nao use WobbleBehavior aqui - a WaveAttack insere Deactivate +
    // Activate, e o Wobble capturaria velocidade zero na ativacao e se
    // encerraria sozinho.
    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        std::move(params),
        2.4f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_ONE_DURATION,
                                                      "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<RandomWanderStrategy>(3.5f, 115.f));
}

// ---------------------------------------------------------------------------
// FASE 2 - "Aprendeu onde voce esta"
//
// Mira exata na posicao atual, e os projeteis curvam durante o voo com forca
// que decai. Movimento: o boss desce na direcao do jogador.
//
// So TRES projeteis, num leque estreito. A versao anterior tinha sete num
// leque de 50 graus e a perseguicao simplesmente nao aparecia: com muitos
// projeteis na tela o jogador nao consegue atribuir a curva a nenhum deles em
// particular, e o conjunto vira ruido. Leque esconde comportamento.
//
// Leitura de jogo: fugir cedo nao adianta, porque o projetil corrige; e preciso
// esperar a forca decair e desviar no fim. Ensina timing.
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateTwo(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateTwo";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 3;
    params->projectileSpeed = 205.f;
    params->angle = 24.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        1.3f,
        [](Projectile* p, const int index) {
            // Correcao mais forte nas pontas do leque: os das bordas fecham
            // mais, o que afunila a rajada inteira.
            const float forca = (index == 1) ? 2.4f : 3.2f;
            p->insertBehavior<TrackingBehavior>(0.3f, forca, 2.4f);
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_TWO_DURATION,
                                                      "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<HoverAbovePlayerStrategy>(135.f, 170.f));
}

// ---------------------------------------------------------------------------
// FASE 3 - "Aprendeu como voce se move"
//
// UM projetil rapido por vez, com cooldown curto: um fluxo continuo de balas
// individualmente legiveis, em vez de uma rajada que o jogador so pode
// atravessar torcendo.
//
// Cada tiro tem dois tempos:
//   1. Sai adiantado, mirando em onde o jogador ESTARA (ver
//      Julio::CustomizeAttackParams e Boss::GetPredictedPlayerDirection).
//   2. Depois de 0.8s voando reto, da UMA correcao curta e moderada.
//
// A correcao e deliberadamente fraca e limitada. Se fosse uma perseguicao de
// verdade, ela desfaria o contrajogo da previsao: a licao da fase e "seja
// imprevisivel", e uma bala que corrige de qualquer jeito faz ser imprevisivel
// deixar de ajudar. Fraca assim, o jogador ganha DOIS tempos de desvio, e a
// leitura fica "ele errou a previsao e tentou consertar na marra".
//
// Movimento: o boss se fixa no centro. Parou de procurar, agora so calcula.
//
// nextState vazio: e aqui que a batalha e decidida (ver BossAttackState).
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateThree(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateThree";

    auto params = std::make_unique<AttackParams>();
    // AngledAttack com 1 projetil e angulo 0 dispara um tiro unico exatamente
    // em centralAngle (o angleStep fica zerado e o laco roda uma vez). Nao
    // precisa de estrategia nova.
    params->numProjectiles = 1;
    params->projectileSpeed = 300.f;
    params->angle = 0.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        // Cooldown curto: ~4 balas vivas ao mesmo tempo, formando um fluxo.
        // AJUSTE AQUI para calibrar a pressao da fase.
        0.35f,
        [](Projectile* p, int) {
            // Voa reto 0.8s (o jogador ve que foi adiantado), entao UMA
            // correcao de 0.7s. Ver comentario do cabecalho desta fase.
            p->insertBehavior<TrackingBehavior>(0.8f, 1.6f, 0.7f);
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_THREE_DURATION,
                                                      "");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<GoToCenterStrategy>());
}

// ---------------------------------------------------------------------------
// FASE FINAL - "Entrou em loop" (repescagem, nota entre 40 e 60)
//
// PRECISA existir mesmo o Julio sendo um boss de 3 fases. Se a nota cair nessa
// faixa ao fim da fase 3, a FSM tenta ir para "StateFinal"; sem o estado
// registrado, SetState apenas loga o erro e o boss trava.
//
// Tema: o modelo travou. A piada e de graca para calouro de computacao - as
// balas entram em LOOP antes de vir para cima de voce.
//
// Esta e tambem a fase de teste do PathBehavior. Escolhida de proposito: e a
// mais lenta e a mais permissiva da luta, entao da para observar o caminho com
// calma e julgar se a mecanica se paga. Continua sendo a fase mais facil, que e
// o papel de uma repescagem.
// ---------------------------------------------------------------------------
void JulioFactory::ConfigureStateFinal(Boss* boss, FSMComponent* fsm) {

    const std::string STATE_NAME = "StateFinal";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 6;
    params->projectileSpeed = 150.f;

    auto spawner = boss->GetProjectileFactory("Dados");

    // Anel de 6 projeteis, cada um dando uma volta antes de seguir em frente.
    // Como o PathBehavior alinha o caminho com a direcao inicial de cada
    // projetil, o MESMO laco sai girado de um jeito diferente para cada ponto
    // do anel: seis lacos apontando para fora, sem escrever seis caminhos.
    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(params),
        2.6f,
        [](Projectile* p, int) {
            // Raio pequeno e velocidade baixa: a volta precisa ser LENTA para
            // ser lida. Um laco rapido vira um borrao e perde a graca.
            p->insertBehavior<PathBehavior>(PathShapes::Loop(70.f, 500.f, 12), 150.f);
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      STATE_FINAL_DURATION,
                                                      "StateOne");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<RandomWanderStrategy>(4.5f, 85.f));
}
