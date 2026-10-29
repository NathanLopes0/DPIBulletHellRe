//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#include "Julio.h"

#include <algorithm>


#include "../../../Attacks/Shared/Amarra.h"
#include "../../../Attacks/Shared/Ancora.h"
#include "../../../Attacks/Engine/Behaviors.h"
#include "../../../Attacks/Julio/Guardrail.h"
#include "../../../Attacks/AttackParameters/TokenAttackParams.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Core/Random.h"
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

    // UMA FRASE NOVA a cada entreFrases, saindo do professor mirada no aluno.
    mProximaFrase -= deltaTime;
    if (mProximaFrase <= 0.0f) {
        mProximaFrase += mDegrauDaGeracao.entreFrases;
        Frase f;
        f.ponta = GetPosition();
        f.rumo = anguloDe(GetDirectionToPlayer());
        f.total = Geracao::TokensDaFrase(temperatura);
        mFrases.push_back(f);
    }

    // CADA FRASE ANDA: a ponta segue o ultimo token, e a cada kEntreTokens
    // nasce o proximo, sorteado em volta do rumo dele virado para o aluno.
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

        // O LEQUE DA DISTRIBUICAO, saindo da ponta.
        float maior = 0.0f;
        for (const float p : passo.probabilidades) maior = std::max(maior, p);
        for (std::size_t k = 0; k < passo.rumos.size() && maior > 0.0f; ++k) {
            mRaios.push_back(Raio{f.ponta, passo.rumos[k], passo.probabilidades[k] / maior,
                                  static_cast<int>(k) == passo.escolhido, 0.0f});
        }

        // O TOKEN, A FRENTE DA PONTA, no rumo sorteado - o primeiro sai do
        // professor. A ponta passa a ser ele.
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

        // A SILABA: a frase em ordem, e o que passa do fim e alucinacao - uma
        // silaba qualquer da frase.
        const int silaba = (f.feitos < kSilabas) ? f.feitos : Random::GetIntRange(0, kSilabas - 1);
        for (BossProjectile* p : mGerados) {
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(kFrase[silaba]);
        }
        ++f.feitos;
    }
    mFrases.erase(std::remove_if(mFrases.begin(), mFrases.end(),
        [](const Frase& f) { return f.feitos >= f.total; }), mFrases.end());

    // OS RAIOS SE APAGAM em kVidaDoRaio.
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
        mOndaDosGuardrails = 0;   // zerada na saida: a primeira onda pode nascer antes deste quadro
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
        // A FASE ACABOU: os agentes vao embora, e o placar volta ao nome dela,
        // para a proxima vez.
        if (mNosAgentes && !mNomeDosAgentes.empty()) RegistrarAnuncio(kFaseDosAgentes, mNomeDosAgentes);
        if (mNosAgentes) {
            for (auto& [agente, dados] : mAgentes) SoltarAgente(agente);
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

    // O AGE: todos os agentes em campo mandam a ferramenta ao mesmo tempo.
    const float antes = mCiclo.t;
    if (ReAct::Avancar(mCiclo, deltaTime)) Disparar(ReAct::kGatilhoDeAgir);

    // UMA VOLTA NOVA, um Pensa novo - nunca menor que o que a volta ja andou -
    // e os agentes replanejam: cada um vai pensar num lugar novo.
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
    // SAI DO POSTO, e as ferramentas deixam de sair dele; e vai embora por
    // cima, por onde veio o professor.
    if (!agente || agente->GetState() != ActorState::Active) return;
    agente->DefinirPosto("");
    if (auto* rb = agente->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, -kSaidaDoAgente));
}

void Julio::MoverAgentes(const float deltaTime, bool replanejar) {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager() || !mScene->GetGame()) return;

    // QUEM ESTA EM CAMPO: o agente derrubado sai da conta, e o recem-convocado
    // entra - e a chegada dele replaneja o grupo, para ninguem cair em cima de
    // ninguem.
    std::unordered_map<BossProjectile*, Agente> emCampo;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active || p->Posto() != ReAct::kPostoDosAgentes) continue;
        const auto it = mAgentes.find(p.get());
        if (it == mAgentes.end()) replanejar = true;
        emCampo[p.get()] = it == mAgentes.end() ? Agente{0.0f, p->GetPosition()} : it->second;
    }
    mAgentes = std::move(emCampo);

    // A VIDA ACABOU: vai embora. Os outros envelhecem.
    for (auto it = mAgentes.begin(); it != mAgentes.end();) {
        it->second.idade += deltaTime;
        if (it->second.idade < ReAct::kVidaDoAgente) { ++it; continue; }
        SoltarAgente(it->first);
        it = mAgentes.erase(it);
    }

    if (replanejar && !mAgentes.empty()) {
        const float xDoAluno = batalha->Alvo() ? batalha->Alvo()->GetPosition().x : GetPosition().x;
        const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
        const auto lugares = ReAct::Destinos(mSorteioDosAgentes, xDoAluno, static_cast<int>(mAgentes.size()), largura);
        size_t k = 0;
        for (auto& [agente, dados] : mAgentes) {
            dados.lugar = Vector2(lugares[k].x, lugares[k].y);
            ++k;
        }
    }

    // CADA UM VOA ATE O SEU LUGAR, e la fica, olhando o aluno.
    for (auto& [agente, dados] : mAgentes) {
        agente->SetPosition(Ancora::Aproximar(agente->GetPosition(), dados.lugar, ReAct::kRapidezDoAgente, deltaTime));
        if (auto* rb = agente->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
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
        // O TOKEN NASCE NA PONTA DA FRASE, no rumo que AtualizarTokens
        // sorteou com a temperatura da etapa (Geracao.h) - e nao no professor.
        if (mGerando) {
            params.firePosition = mOndeGerar;
            params.centralAngle = mRumoAGerar;
            mVelocidadeDoToken = params.projectileSpeed;
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
    if (mGerando) {
        for (auto& p : projectiles) {
            if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mGerados.push_back(b);
        }
    }

    // O ANEL RETREINADO (Guardrail.h): cada onda de guardrails alterna o anel
    // fechado e o aberto, troca o giro e gira mais depressa. O guardrail e o
    // unico derrubavel da fase - o token nao e.
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
