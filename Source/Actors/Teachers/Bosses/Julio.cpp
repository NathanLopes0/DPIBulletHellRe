//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#include "Julio.h"

#include <algorithm>

#include <SDL_log.h>

#include "../../../Attacks/AttackParameters/TokenAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"

Julio::Julio(Scene* scene) : Boss(scene)
{
}

void Julio::SortearALuta() {
    mLutaSorteada = true;
    mDegrauDosAgentes = ReAct::DegrauDo(NivelDeDificuldade());
    mSorteioDosAgentes = SorteioDaFase(kFaseDosAgentes);

    // A FASE 1: cada etapa com a sua temperatura, e o placar dela dizendo o
    // numero - o placar e a regra.
    Variacao::Sorteio sorteio = SorteioDaFase(kFaseDosTokens);
    mTemperaturas = Temperatura::Sortear(sorteio, NivelDeDificuldade());
    auto etapas = EtapasDa(kFaseDosTokens);
    for (std::size_t k = 0; k < etapas.size() && k < mTemperaturas.size(); ++k) {
        etapas[k].second = Temperatura::Anuncio(mTemperaturas[k], k + 1 == mTemperaturas.size());
    }
    if (!etapas.empty()) RegistrarEtapas(kFaseDosTokens, etapas);
}

int Julio::EtapaDosTokens() const {
    const auto* fsm = GetComponent<FSMComponent>();
    const float t = fsm ? fsm->GetStateTime() : 0.0f;
    const auto etapas = EtapasDa(kFaseDosTokens);
    for (std::size_t k = 0; k < etapas.size(); ++k) {
        if (t < etapas[k].first) return static_cast<int>(std::min(k, mTemperaturas.size() - 1));
    }
    return static_cast<int>(mTemperaturas.size()) - 1;
}

void Julio::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarAgentes(deltaTime);
    AtualizarGuardrails(deltaTime);
    AtualizarLoop();
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

void Julio::AtualizarLoop() {
    // A FINAL COMECA SEM REPETICAO: o placar diz o nome dela, e o primeiro
    // disparo poe o primeiro "e".
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noLoop = fsm && fsm->GetStateName() == kFaseDoLoop;
    if (noLoop && !mNoLoop) {
        if (mNomeDoLoop.empty()) mNomeDoLoop = AnuncioDaFase(kFaseDoLoop);
        mRepeticoes = 0;
        RegistrarAnuncio(kFaseDoLoop, mNomeDoLoop);
    }
    mNoLoop = noLoop;
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
        mCiclo = ReAct::Ciclo{};
        mCiclo.observa = mDegrauDosAgentes.observa;
        mCiclo.pensa = ReAct::PensaDaVolta(mSorteioDosAgentes, mDegrauDosAgentes.pensa);
        if (mNomeDosAgentes.empty()) mNomeDosAgentes = AnuncioDaFase(kFaseDosAgentes);
    }

    // O AGE: todos os agentes em campo mandam a ferramenta ao mesmo tempo.
    const float antes = mCiclo.t;
    if (ReAct::Avancar(mCiclo, deltaTime)) Disparar(ReAct::kGatilhoDeAgir);

    // UMA VOLTA NOVA, um Pensa novo - nunca menor que o que a volta ja andou.
    if (mCiclo.t < antes) {
        mCiclo.pensa = std::max(ReAct::PensaDaVolta(mSorteioDosAgentes, mDegrauDosAgentes.pensa), mCiclo.t + 0.01f);
    }

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
// facil demais (maior, ate 1.0). NO DIFICIL o modelo preve melhor:
// kLeadFractionNoDificil.
//
// Derivar do tempo de voo, em vez de usar um numero fixo de segundos, importa:
// com antecipacao fixa de 0.5s e projetil a 265 px/s cruzando ~400 px, o tiro
// antecipava 150 px enquanto o jogador andava 450 px no mesmo intervalo - ou
// seja, correr reto era SEGURO, exatamente o oposto da intencao da fase.
static constexpr float kLeadFraction = 0.85f;
static constexpr float kLeadFractionNoDificil = 0.97f;

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

        // E A TEMPERATURA E A DA ETAPA NESTA LUTA - a que o placar diz.
        if (auto* tokens = dynamic_cast<TokenAttackParams*>(&params)) {
            tokens->temperatura = mTemperaturas[static_cast<std::size_t>(EtapaDosTokens())];
        }
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

        params.centralAngle = anguloDe(GetPredictedPlayerDirection(tempoDeVoo * NoNivel(kLeadFraction, kLeadFractionNoDificil)));

        // NO DIFICIL saem dois tokens por vez (o bloco "dificil" do ataque),
        // e o segundo nao vai exatamente no mesmo lugar: um pouco de
        // temperatura abre o par em volta da previsao.
        if (auto* tokens = dynamic_cast<TokenAttackParams*>(&params)) {
            tokens->temperatura = NoNivel(tokens->temperatura, Temperatura::kDaPrevisaoNoDificil);
        }
    }
    else if (stateName == "StateFinal") {
        // O LOOP: as frases de "e" saem miradas no aluno. Os aneis sao uma
        // CircleSpreadAttack, que nem le o angulo. No Dificil o modelo em loop
        // tambem esquenta, e as frases se abrem.
        params.centralAngle = anguloDe(GetDirectionToPlayer());
        if (auto* tokens = dynamic_cast<TokenAttackParams*>(&params)) {
            tokens->temperatura = NoNivel(tokens->temperatura, Temperatura::kDoLoopNoDificil);
        }
    }
}

void Julio::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O LOOP NO PLACAR: na final, cada disparo poe mais um "e" - o modelo
    // repetindo a mesma palavra, ate encher o placar.
    //
    // (Este gancho pintava os DADOS com a cor de cada fase. Desde o 126 o
    // Julio nao usa dado nenhum - tokens, agentes e guardrails tem a cara que
    // a estrategia ou o tipo deles da.)
    if (stateName == kFaseDoLoop) {
        if (mRepeticoes < kRepeticoesNoPlacar) ++mRepeticoes;
        std::string texto = "e";
        for (int i = 1; i < mRepeticoes; ++i) texto += " e";
        RegistrarAnuncio(kFaseDoLoop, texto);
    }
}
