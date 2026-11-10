//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"

#include <algorithm>

#include "../../Scenes/Battle/Battle.h"
#include "../Projectile.h"
#include "../ProjectileFactory.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../Player/Player.h"
#include "Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Movements/MovementStrategies.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "../../Attacks/FasesDeAtaque.h"
#include "../../Regras/PontoExtra.h"
#include "../../Attacks/AttackParameters/PontosAttackParams.h"
#include "SDL_log.h"
#include "../Player/PlayerProjectile.h"

Boss::Boss(Scene* scene) : Actor(scene), mBatalha(dynamic_cast<Battle*>(scene)) {

    AddComponent<RigidBodyComponent>();
    AddComponent<FSMComponent>();

}

Boss::~Boss() = default;

void Boss::Start() {
    CalculateNextDropThreshold();
    if (auto fsm = GetComponent<FSMComponent>()) {
        // O clone nao avisa a batalha: a fase e a do original.
        if (auto battleScene = Batalha(); battleScene && !mEhClone) {
            fsm->SetOnStateChanged([battleScene](float newDuration) {
                battleScene->AoMudarDeFase(newDuration);
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

void Boss::Congelar(const float segundos) {
    mCongelado = std::max(mCongelado, segundos);
    // A fase para: a maquina de estados e quem conta o tempo dela e solta os ataques.
    if (auto* fsm = GetComponent<FSMComponent>()) fsm->SetEnabled(false);
    if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
}

void Boss::OnUpdate(float deltaTime) {
    Actor::OnUpdate(deltaTime);

    // Congelado (o hlt): nada anda e nada sai.
    if (mCongelado > 0.0f) {
        mCongelado -= deltaTime;
        if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        if (mCongelado <= 0.0f) {
            mCongelado = 0.0f;
            if (auto* fsm = GetComponent<FSMComponent>()) fsm->SetEnabled(true);
        }
        return;
    }

    auto fsm = GetComponent<FSMComponent>();
    if (!fsm) return;

    std::string currState = fsm->GetStateName();
    if (currState.empty()) { return; }

    // ---- Lógica de movimento ----
    if (currState != mLastStateName) {

        // A divisao acontece na troca de fase, e so no corpo original.
        if (!mEhClone) {
            if (const auto batalha = Batalha()) {
                batalha->JuntarCorpos();                       // desfaz a divisao anterior
                const std::string doClone = CloneDaFase(currState);
                if (!doClone.empty()) batalha->DividirChefe(doClone);
            }
        }

        mLastStateName = currState;

        // Procura se existe estratégia para o novo estado.
        if (mMovementStrategies.find(currState) != mMovementStrategies.end()) {
            mCurrentMovementStrategy = mMovementStrategies[currState].get();
            mCurrentMovementStrategy->Initialize(this); // Reset timers/alvos
        } else {
            // Se não tiver estratégia definida, talvez parar o boss?
            mCurrentMovementStrategy = nullptr;
            if(auto rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }

    // Executa a estratégia atual.
    if (mCurrentMovementStrategy) {
        mCurrentMovementStrategy->Update(this, deltaTime);
    }

    // Verifica se existem ataques registrados pro estado atual.
    if (mAttacksMap.find(currState) != mAttacksMap.end()) {

        // O relogio da fase, para as janelas das etapas.
        const float tempoNaFase = fsm->GetStateTime();

        // Itera sobre todos os ataques configurados para esse estado.
        for (auto& attackDefinition : mAttacksMap[currState]) {

            // O ataque de gatilho nao tem relogio: sai quando o chefe chama Disparar, e so.
            if (!attackDefinition.gatilho.empty()) continue;

            // Fora da janela, o ataque espera sem contar.
            if (tempoNaFase < attackDefinition.janelaDe || tempoNaFase >= attackDefinition.janelaAte) {
                attackDefinition.currentTimer = std::min(attackDefinition.currentTimer, kPrimeiroDisparoDaJanela);
                continue;
            }

            attackDefinition.currentTimer -= deltaTime;

            if (attackDefinition.currentTimer <= 0.0f) {
                ExecuteAttack(attackDefinition, currState);
                // A cadencia sorteada no disparo (Variacao::Disparo::espera); sem "variar", 1.
                attackDefinition.currentTimer = attackDefinition.cooldownTotal * attackDefinition.espera;
            }
        }
    }
}

void Boss::AddAttackPattern(const std::string &stateName, std::unique_ptr<IAttackStrategy> strategy, std::unique_ptr<AttackParams> params, float cooldown, ProjectileConfigurator config,
                            const float janelaDe, const float janelaAte, std::string gatilho,
                            std::optional<Variacao::DoDisparo> variar) {
    AttackDefinition def;
    def.variar = variar;
    def.indice = mAttacksMap[stateName].size();
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

    // Chama customização do boss específico.
    CustomizeAttackParams(*(attackDef.params), stateName);

    auto battleScene = Batalha();
    if (!battleScene) { return; }

    auto projManager = battleScene->GerenteDeProjeteis();
    if (!projManager) { return; }

    // O revezamento e aqui, e so aqui.
    const size_t alvoDoDisparo = battleScene->IndiceDoAlvo();

    // A pontos attack atira de onde o chefe disser, e o chefe diz na hora.
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

    // ---- Executa o ataque, com a variacao do disparo ("variar" em fases.json) ---------- // ----
    AttackParams& p = *attackDef.params;
    const int projeteisDoArquivo = p.numProjectiles;
    const float velocidadeDoArquivo = p.projectileSpeed;
    const float aberturaDoArquivo = p.angle;
    const float centroDoArquivo = p.centralAngle;
    p.projectileSpeed = velocidadeDoArquivo * rapidez;
    if (attackDef.variar) {
        const Variacao::Disparo d = SortearDisparo(attackDef, stateName);
        p.centralAngle += d.giro;
        p.giroDoDisparo = d.giro;
        p.angle *= d.abertura;
        p.projectileSpeed *= d.velocidade;
        p.numProjectiles = Variacao::Projeteis(p.numProjectiles, d.projeteis);
        attackDef.espera = d.espera;
    }
    auto projectiles = attackDef.strategy->Execute(p);
    p.numProjectiles = projeteisDoArquivo;
    p.projectileSpeed = velocidadeDoArquivo;
    p.angle = aberturaDoArquivo;
    p.centralAngle = centroDoArquivo;
    p.giroDoDisparo = 0.0f;
    // ---- ------- - ------ ---------- // ----

    // Um disparo que nao soltou nada nao e disparo: nao passa a vez e nao faz barulho.
    if (projectiles.empty()) return;

    // Cada projetil sai dono do seu alvo.
    for (const auto& projetil : projectiles) {
        if (auto* doChefe = dynamic_cast<BossProjectile*>(projetil.get())) {
            doChefe->DefinirAlvo(alvoDoDisparo);
        }
    }

    // A vez passa depois do disparo, e nao antes.
    battleScene->RevezarAlvo();

    // Depois de executar, aplicar a personalização lambda, se existir.
    if (attackDef.configurator) {
        for (int i = 0; i < projectiles.size(); ++i) {
            attackDef.configurator(projectiles[i].get(), i);
        }
    }

    // Gancho para customizacoes do boss concreto sobre a leva inteira.
    OnProjectilesCreated(projectiles, stateName);

    // Converte pra BossProjectile e envia para o Manager.
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

    // ---- Lidando com o audio do ataque -- // ----
    std::string attackAudio;
    {
        if (auto choose = Random::GetIntRange(0,1)) attackAudio = "tan00.wav";
        else attackAudio = "tan01.wav";
    }
    auto handler = mScene->GetGame()->GetAudio()->PlaySound(attackAudio, false);
    mScene->GetGame()->GetAudio()->SetSoundVolume(handler, 10);
    // ---- // ----

}

float Boss::GetDistanceToPlayer() const {
    if (auto battleScene = Batalha()) {
        if (auto player = battleScene->Alvo()) {
            return (player->GetPosition() - GetPosition()).Length();
        }
    }
    return 0.0f;
}

Vector2 Boss::GetPredictedPlayerDirection(const float leadTime) const {
    if (auto battleScene = Batalha()) {
        if (auto player = battleScene->Alvo()) {

            Vector2 target = player->GetPosition();

            // Extrapola pela velocidade atual.
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

Variacao::Disparo Boss::SortearDisparo(const AttackDefinition& def, const std::string& stateName) const {
    if (!def.sorteio) {
        def.sorteio = SorteioDaFase(stateName + (mEhClone ? "#clone#" : "#") + std::to_string(def.indice));
    }
    return Variacao::Sortear(*def.sorteio, def.variar ? *def.variar : Variacao::DoDisparo{});
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
            if (auto* batalha = Batalha()) batalha->DarAVez(static_cast<size_t>(alvo));
        }
        ExecuteAttack(ataque, estado, rapidez);
    }
}

std::vector<Vector2> Boss::PontosDoPosto(const std::string& posto) const {
    std::vector<Vector2> pontos;
    const auto* batalha = Batalha();
    if (posto.empty() || !batalha || !batalha->GerenteDeProjeteis()) return pontos;

    for (const auto& p : batalha->GerenteDeProjeteis()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active) continue;
        if (p->Posto() == posto) pontos.push_back(p->GetPosition());
    }
    return pontos;
}

Vector2 Boss::GetDirectionToPlayer() const {
    if (auto battleScene = Batalha()) {
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
    mNextDropThreshold = PontoExtra::kAcertosParaCair
                       + Random::GetIntRange(-PontoExtra::kFolgaDaQueda, PontoExtra::kFolgaDaQueda);
}

void Boss::OnCollision(Actor *other) {
    if (!dynamic_cast<PlayerProjectile*>(other)) { return; }

    mHitCounter++;
    if (mHitCounter == mNextDropThreshold) {
        if (auto battle = Batalha()) {
            battle->SoltarPontoExtra(GetPosition());
        }

        mHitCounter = 0;

        CalculateNextDropThreshold();
    }
}

SDL_FRect Boss::Campo() const {
    if (mBatalha) return mBatalha->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, 1200.0f, 746.0f};
}

Player* Boss::Aluno() const {
    return mBatalha ? mBatalha->Alvo() : nullptr;
}

Vector2 Boss::PosicaoDoAluno() const {
    if (const Player* aluno = Aluno()) return aluno->GetPosition();
    const SDL_FRect campo = Campo();
    return Vector2(campo.x + campo.w / 2.0f, campo.y + campo.h - 60.0f);
}

void Boss::ApontarParaOAluno(const Vector2& de, Vector2& d) const {
    if (const Player* aluno = Aluno()) {
        d = aluno->GetPosition() - de;
        if (d.LengthSq() > 0.0f) d.Normalize();
    }
}

void Boss::TrocarAnuncio(const std::string& fase, const std::string& texto) {
    if (mAnunciosDoArquivo.count(fase) == 0) mAnunciosDoArquivo[fase] = AnuncioDaFase(fase);
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

void Boss::RestaurarAnuncio(const std::string& fase) {
    const auto it = mAnunciosDoArquivo.find(fase);
    if (it != mAnunciosDoArquivo.end()) RegistrarAnuncio(fase, it->second);
}
