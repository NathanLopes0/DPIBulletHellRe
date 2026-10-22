//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#include "Julio.h"

#include <algorithm>

#include <SDL_log.h>

#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "BossesProjectiles/BossProjectile.h"
#include "../../../Components/AIComponents/FSMComponent.h"

Julio::Julio(Scene* scene) : Boss(scene)
{
}

void Julio::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarAgentes(deltaTime);
    AtualizarGuardrails(deltaTime);
}

bool Julio::NosGuardrails() const {
    const auto* fsm = GetComponent<FSMComponent>();
    return fsm && fsm->GetStateName() == kFaseDosGuardrails && !EhClone();
}

void Julio::OnProjetilBarrado() {
    if (!NosGuardrails()) return;
    mFalaDoGuardrail = kRecusa;
    mTempoDaFala = kTempoDaFala;
}

void Julio::OnProjetilDerrubado() {
    if (!NosGuardrails()) return;
    mFalaDoGuardrail = kJailbreak;
    mTempoDaFala = kTempoDaFala;
}

void Julio::AtualizarGuardrails(const float deltaTime) {
    if (!NosGuardrails()) {
        if (mNosGuardrails && !mNomeDosGuardrails.empty()) RegistrarAnuncio(kFaseDosGuardrails, mNomeDosGuardrails);
        mNosGuardrails = false;
        mTempoDaFala = 0.0f;
        return;
    }
    if (!mNosGuardrails) {
        mNosGuardrails = true;
        if (mNomeDosGuardrails.empty()) mNomeDosGuardrails = AnuncioDaFase(kFaseDosGuardrails);
    }

    // A FALA DURA UM POUCO E VOLTA AO NOME DA FASE. Um tiro barrado no meio da
    // fala a renova: enquanto o aluno insiste, o guardrail recusa.
    mTempoDaFala = std::max(0.0f, mTempoDaFala - deltaTime);
    const std::string& texto = (mTempoDaFala > 0.0f) ? mFalaDoGuardrail : mNomeDosGuardrails;
    if (AnuncioDaFase(kFaseDosGuardrails) != texto) RegistrarAnuncio(kFaseDosGuardrails, texto);
}

void Julio::AtualizarAgentes(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nosAgentes = fsm && fsm->GetStateName() == kFaseDosAgentes && !EhClone();

    if (!nosAgentes) {
        // A FASE ACABOU: o placar volta ao nome dela, para a proxima vez.
        if (mNosAgentes && !mNomeDosAgentes.empty()) RegistrarAnuncio(kFaseDosAgentes, mNomeDosAgentes);
        mNosAgentes = false;
        return;
    }
    if (!mNosAgentes) {
        mNosAgentes = true;
        mCiclo.t = 0.0f;
        if (mNomeDosAgentes.empty()) mNomeDosAgentes = AnuncioDaFase(kFaseDosAgentes);
    }

    // O AGE: todos os agentes em campo mandam a ferramenta ao mesmo tempo.
    if (ReAct::Avancar(mCiclo, deltaTime)) Disparar(ReAct::kGatilhoDeAgir);

    const char* texto = kPensando;
    switch (ReAct::PassoDe(mCiclo)) {
        case ReAct::Passo::Pensa:   texto = kPensando; break;
        case ReAct::Passo::Age:     texto = kAgindo; break;
        case ReAct::Passo::Observa: texto = kObservando; break;
    }
    if (AnuncioDaFase(kFaseDosAgentes) != texto) RegistrarAnuncio(kFaseDosAgentes, texto);
}

// Fracao do tempo de voo usada como antecipacao na fase 3.
//
// 1.0 seria interceptacao perfeita: quem anda em linha reta e SEMPRE atingido.
// 0.85 deixa uma margem pequena, entao correr reto quase sempre custa caro, mas
// nao e sentenca automatica. AJUSTE AQUI se a fase 3 ficar injusta (menor) ou
// facil demais (maior, ate 1.0).
//
// Derivar do tempo de voo, em vez de usar um numero fixo de segundos, importa:
// com antecipacao fixa de 0.5s e projetil a 265 px/s cruzando ~400 px, o tiro
// antecipava 150 px enquanto o jogador andava 450 px no mesmo intervalo - ou
// seja, correr reto era SEGURO, exatamente o oposto da intencao da fase.
static constexpr float kLeadFraction = 0.85f;

void Julio::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Convencao de centralAngle: 0 = direita, 90 = baixo, 180 = esquerda.
    auto anguloDe = [](const Vector2& dir) {
        return Math::ToDegrees(Math::Atan2(dir.y, dir.x));
    };

    if (stateName == "StateOne") {
        // OS TOKENS SAEM MIRADOS NO ALUNO: o mais provavel e o tiro certo. Quem
        // os desvia e a temperatura, sorteando entre os candidatos em volta da
        // mira (TokenAttack, Temperatura.h) - e nao um ruido daqui.
        params.centralAngle = anguloDe(GetDirectionToPlayer());
    }
    // A FASE 2 NAO MEXE NA MIRA: os agentes saem no leque escrito em fases.json,
    // e quem mira e cada agente, da PontosAttack.
    else if (stateName == "StateThree") {
        // O MODELO TE PREDIZ. Mira em onde o jogador ESTARA, nao onde ele esta
        // - e o token da fase dos guardrails.
        //
        // Sem ruido aleatorio de proposito: se o tiro errasse tambem por acaso,
        // o jogador nao conseguiria distinguir "ele me previu" de "ele errou
        // sozinho", e a fase perderia a leitura inteira. Todo erro aqui tem de
        // ser causado pelo jogador ter mudado de direcao.
        // Tempo que o projetil leva para chegar onde o jogador esta agora.
        const float distancia = GetDistanceToPlayer();
        const float tempoDeVoo = (params.projectileSpeed > 1.0f)
                                     ? distancia / params.projectileSpeed
                                     : 0.0f;

        params.centralAngle = anguloDe(GetPredictedPlayerDirection(tempoDeVoo * kLeadFraction));
    }
    else if (stateName == "StateFinal") {
        // RECOMECOU DO ZERO. Volta a atirar para baixo, previsivel e justo.
        params.centralAngle = 90.0f;
    }
}

void Julio::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {

    // Animacao por FASE, nao por projetil: o jogador aprende a associar a arte ao
    // comportamento que vem a seguir.
    //
    // Os nomes seguem os registrados em JulioProjectile1Factory. Estavam
    // desatualizados ("Ruido", "Gradiente", "Overfit") desde o redesenho das
    // fases, e o componente respondia com um aviso e mantinha a animacao
    // anterior - 78 avisos numa luta de 85s, e a arte exibida era a que sobrou
    // do projetil reciclado.
    std::string animation = "Coleta";
    if (stateName == "StateTwo")        animation = "Perseguicao";
    else if (stateName == "StateThree") animation = "Previsao";

    // SO OS DADOS: os tokens da fase 1 tem a cara da frase, que a TokenAttack
    // ja escolheu, e o tipo deles nem tem estas animacoes.
    const ProjectileFactory* dados = GetProjectileFactory("Dados");
    for (auto& p : projectiles) {
        if (!p) continue;
        const auto* doChefe = dynamic_cast<BossProjectile*>(p.get());
        if (!doChefe || doChefe->GetOriginFactory() != dados) continue;
        if (auto drawComp = p->GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation(animation);
        }
    }
}
