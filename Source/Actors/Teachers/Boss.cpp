//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Actors/Projectile.h"
#include "../ProjectileFactory.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../Player/Player.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Movements/MovementStrategies.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "../../Attacks/FasesDeAtaque.h"
#include "../../Attacks/AttackParameters/PontosAttackParams.h"
#include "SDL_log.h"
#include "../Player/PlayerProjectile.h"

Boss::Boss(Scene* scene) : Actor(scene) {

    AddComponent<RigidBodyComponent>();
    AddComponent<FSMComponent>();

}

Boss::~Boss() = default;

void Boss::Start() {
    CalculateNextDropThreshold();
    if (auto fsm = GetComponent<FSMComponent>()) {
        if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
            fsm->SetOnStateChanged([battleScene](float newDuration) {
                battleScene->OnFaseTrocada(newDuration);
            });
        }

        if (!mInitialState.empty()) {
            fsm->Start(mInitialState);
        }
        else {
            SDL_Log("Boss::Start(): Nenhum estado inicial foi definido pela Factory");
        }
    }
    else {
        SDL_Log("Erro: Boss::Start() nao encontrou um FSMComponent");
    }
}

void Boss::OnUpdate(float deltaTime) {
    Actor::OnUpdate(deltaTime);

    auto fsm = GetComponent<FSMComponent>();
    if (!fsm) return;

    std::string currState = fsm->GetStateName();
    if (currState.empty()) { return; }

    // --- LÓGICA DE MOVIMENTO ---
    // Verifica se mudou de estado para trocar a estratégia
    if (currState != mLastStateName) {

        // A DIVISAO ACONTECE NA TROCA DE FASE, e so no corpo original: um clone
        // que se dividisse viraria quatro corpos, depois oito. Quem o impede de
        // chegar aqui e BossAttackState, que nao o transiciona - esta guarda e a
        // segunda tranca, para o dia em que alguem mexer naquela.
        if (!mEhClone) {
            if (const auto batalha = dynamic_cast<Battle*>(mScene)) {
                batalha->JuntarCorpos();                       // desfaz a divisao anterior
                const std::string doClone = CloneDaFase(currState);
                if (!doClone.empty()) batalha->DividirChefe(doClone);
            }
        }

        mLastStateName = currState;

        // Procura se existe estratégia para o novo estado
        if (mMovementStrategies.find(currState) != mMovementStrategies.end()) {
            mCurrentMovementStrategy = mMovementStrategies[currState].get();
            mCurrentMovementStrategy->Initialize(this); // Reset timers/alvos
        } else {
            // Se não tiver estratégia definida, talvez parar o boss?
            mCurrentMovementStrategy = nullptr;
            if(auto rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }

    // Executa a estratégia atual
    if (mCurrentMovementStrategy) {
        mCurrentMovementStrategy->Update(this, deltaTime);
    }

    // Verifica se existem ataques registrados pro estado atual
    if (mAttacksMap.find(currState) != mAttacksMap.end()) {

        // O RELOGIO DA FASE, para as janelas das etapas.
        const float tempoNaFase = fsm->GetStateTime();

        // Itera sobre todos os ataques configurados para esse estado
        for (auto& attackDefinition : mAttacksMap[currState]) {

            // O ATAQUE DE GATILHO NAO TEM RELOGIO: sai quando o chefe chama
            // Disparar, e so.
            if (!attackDefinition.gatilho.empty()) continue;

            // FORA DA JANELA, o ataque espera sem contar. Ao abrir, o primeiro
            // disparo sai logo - e nao depois de um cooldown inteiro.
            if (tempoNaFase < attackDefinition.janelaDe || tempoNaFase >= attackDefinition.janelaAte) {
                attackDefinition.currentTimer = std::min(attackDefinition.currentTimer, kPrimeiroDisparoDaJanela);
                continue;
            }

            attackDefinition.currentTimer -= deltaTime;

            if (attackDefinition.currentTimer <= 0.0f) {
                ExecuteAttack(attackDefinition, currState);
                attackDefinition.currentTimer = attackDefinition.cooldownTotal;
            }
        }
    }
}

void Boss::AddAttackPattern(const std::string &stateName, std::unique_ptr<IAttackStrategy> strategy, std::unique_ptr<AttackParams> params, float cooldown, ProjectileConfigurator config,
                            const float janelaDe, const float janelaAte, std::string gatilho) {
    AttackDefinition def;
    def.janelaDe = janelaDe;
    def.janelaAte = janelaAte;
    def.gatilho = std::move(gatilho);
    def.strategy = std::move(strategy);
    def.params = std::move(params);
    def.configurator = std::move(config);
    def.cooldownTotal = cooldown;
    def.currentTimer = kPrimeiroDisparoDaFase;

    mAttacksMap[stateName].push_back(std::move(def));
}
void Boss::RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy) {
    mMovementStrategies[stateName] = std::move(strategy);
}
void Boss::ExecuteAttack(const AttackDefinition& attackDef, const std::string& stateName, const float rapidez)
{

    if (!attackDef.params || !attackDef.strategy) {
        SDL_Log("Erro em ExecuteAttack: params ou strategy estao nulos no estado %s!", stateName.c_str());
        return;
    }

    // Atualiza a posição de tiro pra posição do boss.
    attackDef.params->firePosition = GetPosition();

    // Chama customização do boss específico
    CustomizeAttackParams(*(attackDef.params), stateName);

    auto battleScene = dynamic_cast<Battle*>(mScene);
    if (!battleScene) { return; }

    auto projManager = battleScene->GetProjectileManager();
    if (!projManager) { return; }

    // O REVEZAMENTO E AQUI, E SO AQUI.
    //
    // Este e o unico lugar por onde todo ataque de todo chefe passa, e e por
    // isso que D3 cabe em duas linhas: ler de quem e a vez, e passa-la depois
    // do disparo. Dentro de Execute, Alvo() devolve o mesmo aluno do comeco ao
    // fim - o laser pergunta duas vezes e as duas dao a mesma resposta.
    //
    // UMA RAJADA E UM DISPARO. Os vinte projeteis de um leque miram o mesmo
    // aluno; revezar dentro da rajada espalharia metade para cada lado e nao
    // seria mira nenhuma.
    const size_t alvoDoDisparo = battleScene->IndiceDoAlvo();

    // A PONTOS ATTACK ATIRA DE ONDE O CHEFE DISSER, e o chefe diz na hora: os
    // pontos de um posto mudam a cada quadro (uma historia entregue sai do
    // quadro), entao sao lidos aqui, e nao guardados.
    if (auto* pontos = dynamic_cast<PontosAttackParams*>(attackDef.params.get())) {
        pontos->pontos = PontosDoPosto(pontos->posto);
        pontos->temAlvo = false;
        pontos->velocidadeDoAlvo = Vector2::Zero;
        if (const auto* aluno = battleScene->Alvo()) {
            pontos->alvo = aluno->GetPosition();
            if (const auto* rb = aluno->GetComponent<RigidBodyComponent>()) pontos->velocidadeDoAlvo = rb->GetVelocity();
            pontos->temAlvo = true;
        }
    }

    // ---------- EXECUTA O ATAQUE ---------- //
    // A RAPIDEZ VALE SO PARA ESTE DISPARO (Disparar): a estrategia le a
    // velocidade aqui dentro, e o objeto e o mesmo no disparo seguinte.
    const float velocidadeDoArquivo = attackDef.params->projectileSpeed;
    attackDef.params->projectileSpeed = velocidadeDoArquivo * rapidez;
    auto projectiles = attackDef.strategy->Execute(*(attackDef.params));
    attackDef.params->projectileSpeed = velocidadeDoArquivo;
    // ---------- ------- - ------ ---------- //

    // UM DISPARO QUE NAO SOLTOU NADA NAO E DISPARO: nao passa a vez e nao faz
    // barulho. So a PontosAttack de um posto vazio chega aqui - a sprint que
    // ainda nao tem historia parada no quadro.
    if (projectiles.empty()) return;

    // CADA PROJETIL SAI DONO DO SEU ALVO. Sem isto, os que perseguem leriam "de
    // quem e a vez" a cada quadro e trocariam de rumo no meio do voo, todos
    // juntos, a cada disparo novo. Ver BossProjectile::DefinirAlvo.
    for (const auto& projetil : projectiles) {
        if (auto* doChefe = dynamic_cast<BossProjectile*>(projetil.get())) {
            doChefe->DefinirAlvo(alvoDoDisparo);
        }
    }

    // A VEZ PASSA DEPOIS DO DISPARO, e nao antes. Antes, o primeiro ataque da
    // batalha saia mirado no jogador 2 - a vez comeca em 0 e passar a vez antes
    // de atirar a levava para 1 sem ninguem ter sido mirado. Nao muda o
    // equilibrio ao longo da batalha, e muda o que o jogador 1 ve no comeco.
    battleScene->RevezarAlvo();

    // Depois de executar, aplicar a personalização lambda, se existir
    if (attackDef.configurator) {
        for (int i = 0; i < projectiles.size(); ++i) {
            attackDef.configurator(projectiles[i].get(), i);
        }
    }

    // Gancho para customizacoes do boss concreto sobre a leva inteira.
    OnProjectilesCreated(projectiles, stateName);

    // Converte pra BossProjectile e envia para o Manager
    std::vector<std::unique_ptr<BossProjectile>> bossProjVector;
    bossProjVector.reserve(projectiles.size());

    for (auto& p : projectiles) {
        if (dynamic_cast<BossProjectile*>(p.get())) {
            bossProjVector.emplace_back(std::unique_ptr<BossProjectile>(dynamic_cast<BossProjectile*>(p.release())));
        }
    }

    if (!bossProjVector.empty()) {
        projManager->AddBossProjectiles(std::move(bossProjVector));
    }

    // -- LIDANDO COM O AUDIO DO ATAQUE -- //
    std::string attackAudio;
    {
        if (auto choose = Random::GetIntRange(0,1)) attackAudio = "tan00.wav";
        else attackAudio = "tan01.wav";
    }
    auto handler = mScene->GetGame()->GetAudio()->PlaySound(attackAudio, false);
    mScene->GetGame()->GetAudio()->SetSoundVolume(handler, 10);
    // ----------------------------- //

}

float Boss::GetDistanceToPlayer() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->Alvo()) {
            return (player->GetPosition() - GetPosition()).Length();
        }
    }
    return 0.0f;
}

Vector2 Boss::GetPredictedPlayerDirection(const float leadTime) const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->Alvo()) {

            Vector2 target = player->GetPosition();

            // Extrapola pela velocidade atual. Se o jogador estiver parado, a
            // velocidade e zero e isto vira a mira normal: ficar parado
            // continua sendo a pior escolha possivel.
            if (auto rb = player->GetComponent<RigidBodyComponent>()) {
                target += rb->GetVelocity() * leadTime;
            }

            Vector2 direction = target - GetPosition();
            direction.Normalize();
            return direction;
        }
    }

    // Failsafe igual ao de GetDirectionToPlayer: atira para baixo.
    return Vector2(0.0f, 1.0f);
}

Dificuldade::Nivel Boss::NivelDeDificuldade() const {
    if (const auto* batalha = dynamic_cast<const Battle*>(mScene)) return batalha->NivelDeDificuldade();
    return Dificuldade::Nivel::Normal;
}

Variacao::Sorteio Boss::SorteioDaFase(const std::string& fase) const {
    const auto* batalha = dynamic_cast<const Battle*>(mScene);
    return Variacao::SorteioDa(batalha ? batalha->SementeDaBatalha() : 0u, fase);
}

void Boss::Disparar(const std::string& gatilho, const int alvo, const float rapidez) {
    if (gatilho.empty()) return;
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm) return;
    const std::string estado = fsm->GetStateName();
    const auto fase = mAttacksMap.find(estado);
    if (fase == mAttacksMap.end()) return;

    const float tempoNaFase = fsm->GetStateTime();
    for (const auto& ataque : fase->second) {
        if (ataque.gatilho != gatilho) continue;
        if (tempoNaFase < ataque.janelaDe || tempoNaFase >= ataque.janelaAte) continue;
        if (alvo >= 0) {
            if (auto* batalha = dynamic_cast<Battle*>(mScene)) batalha->DarAVez(static_cast<size_t>(alvo));
        }
        ExecuteAttack(ataque, estado, rapidez);
    }
}

std::vector<Vector2> Boss::PontosDoPosto(const std::string& posto) const {
    std::vector<Vector2> pontos;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (posto.empty() || !batalha || !batalha->GetProjectileManager()) return pontos;

    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active) continue;
        if (p->Posto() == posto) pontos.push_back(p->GetPosition());
    }
    return pontos;
}

Vector2 Boss::GetDirectionToPlayer() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->Alvo()) {
            Vector2 direction = player->GetPosition() - GetPosition();
            direction.Normalize();
            return direction;
        }
    }

    //Failsafe: se não achar o player, atira pra baixo.
    return Vector2(0.0f, 1.0f);
}

ProjectileFactory* Boss::GetProjectileFactory(const std::string& factoryName) const {

    if (const auto it = mProjectileFactories.find(factoryName); it != mProjectileFactories.end()) {
        return it->second.get(); // Retorna o ponteiro bruto da fábrica específica
    }

    SDL_Log("Erro: Fabrica '%s' nao encontrada no Boss!", factoryName.c_str());
    return nullptr;
}

void Boss::AddProjectileFactory(const std::string& projectileName, std::unique_ptr<ProjectileFactory> factory) {
    mProjectileFactories[projectileName] = std::move(factory);
}

std::vector<std::string> Boss::GetProjectileFactoryNames() const {
    std::vector<std::string> names;
    names.reserve(mProjectileFactories.size());
    for (const auto& [name, factory] : mProjectileFactories) {
        names.push_back(name);
    }
    return names;
}

void Boss::PrewarmProjectilePools(int countPerType) {
    for (auto& [name, factory] : mProjectileFactories) {
        if (!factory) continue;
        SDL_Log("Prewarm: criando %d instancias de '%s'...", countPerType, name.c_str());
        factory->Prewarm(GetScene(), this, countPerType);
    }
}

// Implementacao padrao: nao faz nada. A maioria dos bosses nao precisa mexer
// nos projeteis depois de criados, entao herda este corpo vazio.
void Boss::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                const std::string& stateName) {
}

void Boss::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {

}

void Boss::RegistrarClone(const std::string& fase, const std::string& ataquesDoClone) {
    mClonePorFase[fase] = ataquesDoClone;
}

std::string Boss::CloneDaFase(const std::string& fase) const {
    const auto it = mClonePorFase.find(fase);
    return it == mClonePorFase.end() ? std::string{} : it->second;
}

void Boss::RegistrarAnuncio(const std::string& fase, const std::string& texto) {
    mAnuncioPorFase[fase] = texto;
}


std::string Boss::AnuncioDaFase(const std::string& fase) const {
    if (fase == mLastStateName) {
        if (const auto* fsm = GetComponent<FSMComponent>()) return AnuncioDaFaseEm(fase, fsm->GetStateTime());
    }
    return AnuncioDaFaseEm(fase, -1.0f);
}

std::string Boss::AnuncioDaFaseEm(const std::string& fase, const float t) const {
    const auto etapas = mEtapasPorFase.find(fase);
    if (etapas != mEtapasPorFase.end() && t >= 0.0f) {
        for (const auto& [fim, texto] : etapas->second) {
            if (t < fim) return texto;
        }
    }
    const auto it = mAnuncioPorFase.find(fase);
    return it == mAnuncioPorFase.end() ? std::string{} : it->second;
}

void Boss::RegistrarEtapas(const std::string& fase, std::vector<std::pair<float, std::string>> fimETexto) {
    mEtapasPorFase[fase] = std::move(fimETexto);
}

std::vector<std::pair<float, std::string>> Boss::EtapasDa(const std::string& fase) const {
    const auto it = mEtapasPorFase.find(fase);
    return it == mEtapasPorFase.end() ? std::vector<std::pair<float, std::string>>{} : it->second;
}

void Boss::RegistrarIntercambio(const std::string& fase, const float aCada, const float aviso) {
    mIntercambioPorFase[fase] = {aCada, aviso};
}

std::optional<std::pair<float, float>> Boss::IntercambioDaFase(const std::string& fase) const {
    const auto it = mIntercambioPorFase.find(fase);
    if (it == mIntercambioPorFase.end()) return std::nullopt;
    return it->second;
}

void Boss::SetInitialState(const std::string &stateName) {
    mInitialState = stateName;
}

void Boss::CalculateNextDropThreshold() {
    int baseHits = 80;
    int variance = 8;

    mNextDropThreshold = baseHits + Random::GetIntRange(-variance, variance);

}

void Boss::OnCollision(Actor *other) {
    if (!dynamic_cast<PlayerProjectile*>(other)) { return; }

    mHitCounter++;
    if (mHitCounter == mNextDropThreshold) {
        if (auto battle = dynamic_cast<Battle*>(GetScene())) {
            battle->SpawnExtraPoint(GetPosition());
        }

        mHitCounter = 0;

        CalculateNextDropThreshold();
    }
}






