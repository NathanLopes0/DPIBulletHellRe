//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#include "Julio.h"

#include <algorithm>

#include "../../../Attacks/Amarra.h"
#include "../../../Attacks/Ancora.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Attacks/Guardrail.h"
#include "../../../Attacks/AttackParameters/TokenAttackParams.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Motor/Random.h"
#include "../../../Scenes/Scene.h"

#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// A frase do modelo, um token por silaba: "o modelo te prediz".
    const char* const kFrase[] = {"o", "mo", "de", "lo", "te", "pre", "diz"};
    constexpr int kSilabas = 7;
}

Julio::Julio(Scene* scene) : Boss(scene)
{
}

void Julio::SortearALuta() {
    mLutaSorteada = true;
    mDegrauDosAgentes = ReAct::DegrauDo(NivelDeDificuldade());
    mDegrauDaGeracao = Geracao::DegrauDo(NivelDeDificuldade());
    mSorteioDosAgentes = SorteioDaFase(kFaseDosAgentes);

    // A fase 1: cada etapa com a sua temperatura, e o placar dela dizendo o numero.
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
    AtualizarTokens(deltaTime);
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

void Julio::AtualizarTokens(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nosTokens = fsm && fsm->GetStateName() == kFaseDosTokens && !EhClone();

    if (!nosTokens) {
        if (mNosTokens) {
            mFrases.clear();
            mRaios.clear();
            if (mDistribuicao) mDistribuicao->Limpar();
        }
        mNosTokens = false;
        return;
    }
    if (!mNosTokens) {
        mNosTokens = true;
        mProximaFrase = kPrimeiraFrase;
        if (!mDistribuicao) {
            auto ator = std::make_unique<Actor>(mScene);
            mDistribuicao = ator->AddComponent<DrawLinhasComponent>(92);
            mScene->AddActor(std::move(ator));
        }
    }

    auto anguloDe = [](const Vector2& dir) { return Math::ToDegrees(Math::Atan2(dir.y, dir.x)); };
    const float temperatura = mTemperaturas[static_cast<std::size_t>(EtapaDosTokens())];

    // Uma frase nova a cada entreFrases, saindo do professor mirada no aluno.
    mProximaFrase -= deltaTime;
    if (mProximaFrase <= 0.0f) {
        mProximaFrase += mDegrauDaGeracao.entreFrases;
        Frase f;
        f.ponta = GetPosition();
        f.rumo = anguloDe(GetDirectionToPlayer());
        f.total = Geracao::TokensDaFrase(temperatura);
        mFrases.push_back(f);
    }

    const Vector2 aluno = GetPosition() + GetDirectionToPlayer() * GetDistanceToPlayer();
    for (Frase& f : mFrases) {
        if (f.feitos > 0) {
            const float r = Math::ToRadians(f.rumo);
            f.ponta += Vector2(Math::Cos(r), Math::Sin(r)) * (mVelocidadeDoToken * deltaTime);
        }
        f.relogio -= deltaTime;
        if (f.relogio > 0.0f || f.feitos >= f.total) continue;
        f.relogio += Geracao::kEntreTokens;

        const float aoAlvo = anguloDe(aluno - f.ponta);
        const Geracao::Passo passo = (f.feitos == 0)
            ? Geracao::Passo{{f.rumo}, {1.0f}, 0}     // o primeiro sai mirado
            : Geracao::Proximo(f.rumo, aoAlvo, temperatura, Random::GetFloatRange(0.0f, 1.0f));

        // O leque da distribuicao, saindo da ponta.
        float maior = 0.0f;
        for (const float p : passo.probabilidades) maior = std::max(maior, p);
        for (std::size_t k = 0; k < passo.rumos.size() && maior > 0.0f; ++k) {
            mRaios.push_back(Raio{f.ponta, passo.rumos[k], passo.probabilidades[k] / maior,
                                  static_cast<int>(k) == passo.escolhido, 0.0f});
        }

        // O token, a frente da ponta, no rumo sorteado - o primeiro sai do professor.
        f.rumo = passo.Rumo();
        if (f.feitos > 0) {
            const float r = Math::ToRadians(f.rumo);
            f.ponta += Vector2(Math::Cos(r), Math::Sin(r)) * Geracao::kPasso;
        }
        mGerando = true;
        mOndeGerar = f.ponta;

        mRumoAGerar = f.rumo;
        mGerados.clear();
        Disparar(kGatilhoDoToken);
        mGerando = false;

        // A silaba: a frase em ordem, e o que passa do fim e alucinacao - uma silaba qualquer da frase.
        const int silaba = (f.feitos < kSilabas) ? f.feitos : Random::GetIntRange(0, kSilabas - 1);
        for (BossProjectile* p : mGerados) {
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(kFrase[silaba]);
        }
        ++f.feitos;
    }
    mFrases.erase(std::remove_if(mFrases.begin(), mFrases.end(),
        [](const Frase& f) { return f.feitos >= f.total; }), mFrases.end());

    // Os raios se apagam em kVidaDoRaio.
    for (Raio& r : mRaios) r.idade += deltaTime;
    mRaios.erase(std::remove_if(mRaios.begin(), mRaios.end(),
        [](const Raio& r) { return r.idade >= kVidaDoRaio; }), mRaios.end());
    if (!mDistribuicao) return;
    mDistribuicao->Limpar();
    for (const Raio& r : mRaios) {
        const float vida = 1.0f - r.idade / kVidaDoRaio;
        const auto alfa = static_cast<Uint8>((40.0f + 215.0f * r.peso) * vida);
        const float a = Math::ToRadians(r.rumo);
        const float tamanho = kTamanhoDoRaio * (0.45f + 0.55f * r.peso);
        const Vector2 ate = r.de + Vector2(Math::Cos(a), Math::Sin(a)) * tamanho;
        const SDL_Color cor = r.escolhido ? SDL_Color{240, 250, 255, alfa} : SDL_Color{110, 200, 230, alfa};
        mDistribuicao->Acrescentar(r.de, ate, cor);
    }
}

void Julio::AtualizarLoop() {
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
        mOndaDosGuardrails = 0;
        return;

    }
    if (!mNosGuardrails) {
        mNosGuardrails = true;
        if (mNomeDosGuardrails.empty()) mNomeDosGuardrails = AnuncioDaFase(kFaseDosGuardrails);
    }

    // A fala dura um pouco e volta ao nome da fase.
    mTempoDaFala = std::max(0.0f, mTempoDaFala - deltaTime);
    const std::string& texto = (mTempoDaFala > 0.0f) ? mFalaDoGuardrail : mNomeDosGuardrails;
    if (AnuncioDaFase(kFaseDosGuardrails) != texto) RegistrarAnuncio(kFaseDosGuardrails, texto);
}

void Julio::AtualizarAgentes(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nosAgentes = fsm && fsm->GetStateName() == kFaseDosAgentes && !EhClone();

    if (!nosAgentes) {
        // A fase acabou: os agentes vao embora, e o placar volta ao nome dela, para a proxima vez.
        if (mNosAgentes && !mNomeDosAgentes.empty()) RegistrarAnuncio(kFaseDosAgentes, mNomeDosAgentes);
        if (mNosAgentes) {
            for (const Agente& a : mAgentes) SoltarAgente(a.p);
            mAgentes.clear();
        }
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

    // O age: todos os agentes em campo mandam a ferramenta ao mesmo tempo.
    const float antes = mCiclo.t;
    if (ReAct::Avancar(mCiclo, deltaTime)) Disparar(ReAct::kGatilhoDeAgir);

    // Uma volta nova, um Pensa novo - nunca menor que o que a volta ja andou.
    const bool voltaNova = mCiclo.t < antes;
    if (voltaNova) {
        mCiclo.pensa = std::max(ReAct::PensaDaVolta(mSorteioDosAgentes, mDegrauDosAgentes.pensa), mCiclo.t + 0.01f);
    }
    MoverAgentes(deltaTime, voltaNova);

    const char* texto = kPensando;
    switch (ReAct::PassoDe(mCiclo)) {
        case ReAct::Passo::Pensa:   texto = kPensando; break;
        case ReAct::Passo::Age:     texto = kAgindo; break;
        case ReAct::Passo::Observa: texto = kObservando; break;
    }
    if (AnuncioDaFase(kFaseDosAgentes) != texto) RegistrarAnuncio(kFaseDosAgentes, texto);
}

void Julio::SoltarAgente(BossProjectile* agente) {
    // Sai do posto, e as ferramentas deixam de sair dele.
    if (!agente || agente->GetState() != ActorState::Active) return;
    agente->DefinirPosto("");
    if (auto* rb = agente->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, -kSaidaDoAgente));
}

void Julio::MoverAgentes(const float deltaTime, bool replanejar) {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager() || !mScene->GetGame()) return;

    // Quem esta em campo: o agente derrubado sai da conta, e o recem-convocado entra.
    std::vector<Agente> emCampo;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active || p->Posto() != ReAct::kPostoDosAgentes) continue;
        const auto it = std::find_if(mAgentes.begin(), mAgentes.end(),
                                     [&p](const Agente& a) { return a.p == p.get(); });
        if (it == mAgentes.end()) replanejar = true;
        emCampo.push_back(it == mAgentes.end() ? Agente{p.get(), 0.0f, p->GetPosition()} : *it);
    }
    mAgentes = std::move(emCampo);

    // A vida acabou: vai embora.
    for (auto it = mAgentes.begin(); it != mAgentes.end();) {
        it->idade += deltaTime;
        if (it->idade < ReAct::kVidaDoAgente) { ++it; continue; }
        SoltarAgente(it->p);
        it = mAgentes.erase(it);
    }

    if (replanejar && !mAgentes.empty()) {
        const float xDoAluno = batalha->Alvo() ? batalha->Alvo()->GetPosition().x : GetPosition().x;
        const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
        const auto lugares = ReAct::Destinos(mSorteioDosAgentes, xDoAluno, static_cast<int>(mAgentes.size()), largura);
        for (size_t k = 0; k < mAgentes.size(); ++k) mAgentes[k].lugar = Vector2(lugares[k].x, lugares[k].y);
    }

    // Cada um voa ate o seu lugar, e la fica, olhando o aluno.
    for (const Agente& a : mAgentes) {
        a.p->SetPosition(Ancora::Aproximar(a.p->GetPosition(), a.lugar, ReAct::kRapidezDoAgente, deltaTime));
        if (auto* rb = a.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

// Fracao do tempo de voo usada como antecipacao na fase 3.
static constexpr float kLeadFraction = 0.85f;
static constexpr float kLeadFractionNoDificil = 0.97f;

void Julio::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Convencao de centralAngle: 0 = direita, 90 = baixo, 180 = esquerda.
    auto anguloDe = [](const Vector2& dir) {
        return Math::ToDegrees(Math::Atan2(dir.y, dir.x));
    };

    if (stateName == "StateOne") {
        if (mGerando) {
            params.firePosition = mOndeGerar;
            params.centralAngle = mRumoAGerar;
            mVelocidadeDoToken = params.projectileSpeed;
        }
    }
    else if (stateName == "StateThree") {
        // O modelo te prediz.
        const float distancia = GetDistanceToPlayer();
        const float tempoDeVoo = (params.projectileSpeed > 1.0f)
                                     ? distancia / params.projectileSpeed
                                     : 0.0f;

        params.centralAngle = anguloDe(GetPredictedPlayerDirection(tempoDeVoo * NoNivel(kLeadFraction, kLeadFractionNoDificil)));

        if (auto* tokens = dynamic_cast<TokenAttackParams*>(&params)) {
            tokens->temperatura = NoNivel(tokens->temperatura, Temperatura::kDaPrevisaoNoDificil);
        }
    }
    else if (stateName == "StateFinal") {
        // O loop: as frases de "e" saem miradas no aluno.
        params.centralAngle = anguloDe(GetDirectionToPlayer());
        if (auto* tokens = dynamic_cast<TokenAttackParams*>(&params)) {
            tokens->temperatura = NoNivel(tokens->temperatura, Temperatura::kDoLoopNoDificil);
        }
    }
}

void Julio::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (mGerando) {
        for (auto& p : projectiles) {
            if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mGerados.push_back(b);
        }
    }

    // O anel retreinado (Guardrail.h).
    if (stateName == kFaseDosGuardrails) {
        bool houveOnda = false;
        const Guardrail::Onda onda = Guardrail::DaVez(mOndaDosGuardrails);
        for (auto& p : projectiles) {
            auto* g = dynamic_cast<BossProjectile*>(p.get());
            if (!g || !g->Derrubavel()) continue;
            houveOnda = true;
            Amarra::Corda corda;
            corda.comprimento = onda.comprimento;
            corda.duracao = kDuracaoDoAnel;
            corda.sentido = onda.horario ? Amarra::Sentido::Horario : Amarra::Sentido::AntiHorario;
            g->insertMotion<AmarrarBehavior>(corda, "");
            if (auto* rb = g->GetComponent<RigidBodyComponent>()) rb->SetVelocity(rb->GetVelocity() * onda.rapidez);
        }
        if (houveOnda) ++mOndaDosGuardrails;
    }

    // O loop no placar: na final, cada disparo poe mais um "e".
    if (stateName == kFaseDoLoop) {
        if (mRepeticoes < kRepeticoesNoPlacar) ++mRepeticoes;
        std::string texto = "e";
        for (int i = 1; i < mRepeticoes; ++i) texto += " e";
        RegistrarAnuncio(kFaseDoLoop, texto);
    }
}
