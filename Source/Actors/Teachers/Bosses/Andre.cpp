//
// Andre - professor de Programacao 1 (INF 110)
//

#include "Andre.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../../../Random.h"
#include "../../../Attacks/AttackParameters/BaloonAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    bool Vivo(const BossProjectile* p) { return p && p->GetState() == ActorState::Active; }

    std::uint32_t Sorteio() {
        return static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    }

    void Parar(BossProjectile* p) {
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }

    void PorCara(BossProjectile* p, const std::string& cara) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    /// O losango do fluxograma: meia largura e meia altura.
    constexpr float kMeioLosango = 44.0f;
    constexpr float kMeiaAlturaDoLosango = 32.0f;

    /// O balao estoura na borda de baixo do cabecalho.
    constexpr float kMeiaAlturaDoRotulo = 15.0f;

    /// O cabecalho fica aceso enquanto o confete dele comeca a cair.
    constexpr float kRotuloAceso = 0.45f;

    /// O confete nao nasce em fileira certinha: cada um ate kTremor px acima
    /// do lugar dele.
    constexpr float kTremor = 26.0f;

    /// O balao sem case sobe e sai do campo.
    constexpr float kVelocidadeDeSaida = 260.0f;

    constexpr SDL_Color kApagado{110, 130, 170, 200};
    constexpr SDL_Color kAceso{255, 214, 72, 255};
    constexpr SDL_Color kVerdadeiro{96, 222, 112, 255};
    constexpr SDL_Color kFalso{236, 84, 72, 255};
}

Andre::Andre(Scene* scene) : Boss(scene)
{
}

void Andre::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    if (EhClone()) return;
    AtualizarCondicional(deltaTime);
    AtualizarLaco(deltaTime);
}

std::optional<Vector2> Andre::AlvoDoMovimento() const {
    return mAlvo;
}

SDL_FRect Andre::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, 1200.0f, 746.0f};
}

std::vector<BossProjectile*> Andre::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                               const std::optional<float> angulo) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mAnguloEm = angulo;
    mCriados.clear();
    Disparar(gatilho);
    mDisparandoEm = false;
    mQuantos = 0;
    mAnguloEm.reset();
    return mCriados;
}

// ---------------------------------------------------------------------------
// A FASE 1: O DESVIO CONDICIONAL
// ---------------------------------------------------------------------------

Vector2 Andre::Decisao() const {
    return GetPosition() + Vector2(0.0f, Condicional::kAbaixoDoProfessor);
}

Vector2 Andre::CabecalhoDoRamo(const Condicional::Comando& c, const int ramo) const {
    const SDL_FRect campo = Campo();
    const int ramos = Condicional::Ramos(c);
    const float meio = (Condicional::InicioDaFaixa(ramo, ramos, campo.w) + Condicional::FimDaFaixa(ramo, ramos, campo.w)) / 2.0f;
    return Vector2(campo.x + meio, campo.y + Condicional::kYDosRotulos);
}

std::vector<Vector2> Andre::Caminho(const Condicional::Comando& c, const int ramo) const {
    // EM ANGULO RETO, como num fluxograma: sai pelo lado do losango que da
    // para a faixa (ou por cima, se ela esta logo acima), vai de lado ate ela e
    // sobe ate o cabecalho.
    const Vector2 d = Decisao();
    const Vector2 alvo = CabecalhoDoRamo(c, ramo) + Vector2(0.0f, kMeiaAlturaDoRotulo);
    Vector2 saida(d.x, d.y - kMeiaAlturaDoLosango);
    if (alvo.x < d.x - kMeioLosango) saida = Vector2(d.x - kMeioLosango, d.y);
    else if (alvo.x > d.x + kMeioLosango) saida = Vector2(d.x + kMeioLosango, d.y);
    return {saida, Vector2(alvo.x, saida.y), alvo};
}

void Andre::AtualizarCondicional(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noSe = fsm && fsm->GetStateName() == Condicional::kFaseDoSe;

    if (!noSe) {
        // A FASE ACABOU: o fluxograma sai, e o placar volta ao do arquivo.
        if (mNoSe) {
            DesmontarCondicional();
            if (!mAnuncioDoSe.empty()) RegistrarAnuncio(Condicional::kFaseDoSe, mAnuncioDoSe);
            mAlvo.reset();
            mNoSe = false;
        }
        return;
    }
    const float agora = fsm->GetStateTime();
    if (!mNoSe) {
        mNoSe = true;
        mVez = 0;
        mProximoComando = 0.0f;
        mBaloes.clear();
        mQuedas.clear();
        if (mAnuncioDoSe.empty()) mAnuncioDoSe = AnuncioDaFase(Condicional::kFaseDoSe);
        if (!mFluxograma && mScene) {
            auto ator = std::make_unique<Actor>(mScene);
            mFluxograma = ator->AddComponent<DrawLinhasComponent>(kOrdemDoFluxograma);
            mScene->AddActor(std::move(ator));
        }
    }

    // O QUE O CAMPO JA LEVOU (o tiro, a borda) sai da conta ANTES de qualquer
    // disparo deste quadro: um projetil morto pode voltar do pool como outro.
    mBaloes.erase(std::remove_if(mBaloes.begin(), mBaloes.end(),
                                 [](const Balao& b) { return !Vivo(b.p); }), mBaloes.end());
    for (BossProjectile*& p : mRotulos) if (!Vivo(p)) p = nullptr;

    if (agora >= mProximoComando) NovoComando(agora);
    if (mBaloesDoComando < Condicional::kBaloesPorComando && agora >= mProximoBalao) {
        SoltarBalao();
        ++mBaloesDoComando;
        mProximoBalao += Condicional::kEntreBaloes;
    }

    // O SWITCH SEM BREAK: os cases de baixo, cada um na sua vez.
    for (auto it = mQuedas.begin(); it != mQuedas.end();) {
        if (agora < it->quando) { ++it; continue; }
        const Queda q = *it;
        it = mQuedas.erase(it);
        Estourar(q.c, q.ramo, q.cor, agora);
    }

    AtualizarBaloes(deltaTime, agora);

    for (size_t k = 0; k < mRotulos.size(); ++k) {
        BossProjectile* p = mRotulos[k];
        if (!p) continue;
        Parar(p);
        const bool aceso = k < mAcesoAte.size() && agora < mAcesoAte[k];
        PorCara(p, aceso ? Condicional::CaraDoRotuloAceso(mComando, static_cast<int>(k))
                         : Condicional::CaraDoRotulo(mComando, static_cast<int>(k)));
    }

    DesenharFluxograma(agora);
}

void Andre::NovoComando(const float agora) {
    mComando = Condicional::Proximo(mVez++, Sorteio());
    mProximoComando = static_cast<float>(mVez) * Condicional::kPorComando;
    mBaloesDoComando = 0;
    mProximoBalao = agora + Condicional::kPrimeiroBalao;
    RegistrarAnuncio(Condicional::kFaseDoSe, Condicional::Placar(mComando));

    // O PROFESSOR MUDA DE LUGAR a cada comando: o losango vai com ele, e a
    // faixa embaixo dele nao fica sempre a mesma.
    const SDL_FRect campo = Campo();
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mAlvo = Vector2(campo.x + Condicional::kXMinimo + fracao * (Condicional::kXMaximo - Condicional::kXMinimo),
                    campo.y + Condicional::kYDoProfessor);

    // OS CABECALHOS DOS RAMOS, um por faixa.
    for (BossProjectile* p : mRotulos) if (Vivo(p)) p->Kill();
    mRotulos.clear();
    const int ramos = Condicional::Ramos(mComando);
    mAcesoAte.assign(static_cast<size_t>(ramos), -1.0f);
    for (int k = 0; k < ramos; ++k) {
        const auto criados = DispararEm(Condicional::kGatilhoDoRotulo, CabecalhoDoRamo(mComando, k));
        BossProjectile* p = criados.empty() ? nullptr : criados.front();
        mRotulos.push_back(p);
        if (!p) continue;
        p->DefinirEnfeite(true);
        Parar(p);
        PorCara(p, Condicional::CaraDoRotulo(mComando, k));
    }
}

void Andre::SoltarBalao() {
    Balao b;
    b.c = mComando;
    b.n = Condicional::Sortear(mComando, Sorteio());
    b.cor = static_cast<int>(Sorteio() % static_cast<std::uint32_t>(Condicional::kCores));
    const auto criados = DispararEm(Condicional::kGatilhoDoBalao, GetPosition());
    if (criados.empty()) return;
    b.p = criados.front();
    Parar(b.p);
    PorCara(b.p, Condicional::CaraDoBalao(b.n, b.cor));
    mBaloes.push_back(b);
}

void Andre::AtualizarBaloes(const float deltaTime, const float agora) {
    const Vector2 decisao = Decisao();
    const float passo = Condicional::kVelocidadeDoBalao * deltaTime;
    const SDL_FRect campo = Campo();

    for (auto it = mBaloes.begin(); it != mBaloes.end();) {
        Balao& b = *it;
        BossProjectile* p = b.p;
        Parar(p);
        switch (b.etapa) {
            case Etapa::Descendo: {
                const Vector2 falta = decisao - p->GetPosition();
                if (falta.Length() > passo) {
                    p->SetPosition(p->GetPosition() + Vector2::Normalize(falta) * passo);
                    break;
                }
                // A CONDICAO E AVALIADA AQUI, com o valor deste balao - e a
                // atribuicao troca o numero dele na frente do aluno.
                p->SetPosition(decisao);
                const int depois = Condicional::Depois(b.c, b.n);
                if (depois != b.n) {
                    b.n = depois;
                    PorCara(p, Condicional::CaraDoBalao(b.n, b.cor));
                }
                b.ramos = Condicional::Executados(b.c, b.n);
                b.etapa = Etapa::Decidindo;
                b.ate = agora + Condicional::kNaDecisao;
                mDecisaoAte = b.ate;
                mDecisaoRamo = b.ramos.empty() ? -1 : b.ramos.front();
                // O PLACAR EXPLICA a regra que este balao mostrou - e volta ao
                // comando no balao seguinte que nao tem o que explicar.
                if (b.c == mComando) {
                    const std::string porque = Condicional::Explicacao(b.c, b.n);
                    RegistrarAnuncio(Condicional::kFaseDoSe, porque.empty() ? Condicional::Placar(b.c) : porque);
                }
                break;
            }
            case Etapa::Decidindo:
                p->SetPosition(decisao);
                if (agora < b.ate) break;
                if (b.ramos.empty()) {
                    b.etapa = Etapa::Saindo;     // nenhum case: nada executa
                } else {
                    b.caminho = Caminho(b.c, b.ramos.front());
                    b.etapa = Etapa::Voando;
                }
                break;
            case Etapa::Voando: {
                float resta = passo;
                while (resta > 0.0f && !b.caminho.empty()) {
                    const Vector2 falta = b.caminho.front() - p->GetPosition();
                    const float d = falta.Length();
                    if (d <= resta) {
                        p->SetPosition(b.caminho.front());
                        b.caminho.erase(b.caminho.begin());
                        resta -= d;
                    } else {
                        p->SetPosition(p->GetPosition() + falta * (resta / d));
                        resta = 0.0f;
                    }
                }
                if (!b.caminho.empty()) break;
                // CHEGOU AO CABECALHO DO RAMO: estoura, e o confete cai pela
                // faixa. No switch sem break, os cases de baixo vem depois.
                p->Kill();
                Estourar(b.c, b.ramos.front(), b.cor, agora);
                for (size_t k = 1; k < b.ramos.size(); ++k) {
                    mQuedas.push_back(Queda{b.c, b.ramos[k], b.cor, agora + Condicional::kCascata * static_cast<float>(k)});
                }
                it = mBaloes.erase(it);
                continue;
            }
            case Etapa::Saindo:
                p->SetPosition(p->GetPosition() + Vector2(0.0f, -kVelocidadeDeSaida * deltaTime));
                if (p->GetPosition().y < campo.y - 60.0f) {
                    p->Kill();
                    it = mBaloes.erase(it);
                    continue;
                }
                break;
        }
        ++it;
    }
}

void Andre::Estourar(const Condicional::Comando& c, const int ramo, const int cor, const float agora) {
    const SDL_FRect campo = Campo();
    const int ramos = Condicional::Ramos(c);
    const float inicio = campo.x + Condicional::InicioDaFaixa(ramo, ramos, campo.w);
    const float fim = campo.x + Condicional::FimDaFaixa(ramo, ramos, campo.w);
    const float y = campo.y + Condicional::kYDosRotulos;
    const auto confetes = Condicional::Confetes(inicio, fim);
    const auto pedacos = DispararEm(Condicional::kGatilhoDoConfete, Vector2((inicio + fim) / 2.0f, y),
                                    static_cast<int>(confetes.size()));
    for (size_t k = 0; k < pedacos.size() && k < confetes.size(); ++k) {
        const float tremor = Random::GetFloatRange(0.0f, kTremor);
        pedacos[k]->SetPosition(Vector2(confetes[k].x, y - confetes[k].acima - tremor));
        if (auto* rb = pedacos[k]->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2(0.0f, Condicional::kVelocidadeDoConfete));
        }
        PorCara(pedacos[k], Condicional::CaraDoConfete(cor));
    }
    if (c == mComando && ramo >= 0 && static_cast<size_t>(ramo) < mAcesoAte.size()) {
        mAcesoAte[static_cast<size_t>(ramo)] = agora + kRotuloAceso;
    }
}

void Andre::DesenharFluxograma(const float agora) {
    if (!mFluxograma) return;
    mFluxograma->Limpar();
    const Vector2 d = Decisao();

    // A SETA QUE ENTRA NO LOSANGO, do professor.
    mFluxograma->Acrescentar(GetPosition() + Vector2(0.0f, 40.0f), Vector2(d.x, d.y - kMeiaAlturaDoLosango), kApagado);

    // O LOSANGO: verde quando a condicao deu verdadeiro, vermelho no else, e
    // amarelo no case do switch.
    SDL_Color cor = kApagado;
    if (agora < mDecisaoAte) {
        if (mComando.forma == Condicional::Forma::Escolha) cor = mDecisaoRamo < 0 ? kFalso : kAceso;
        else cor = mDecisaoRamo == 0 ? kVerdadeiro : kFalso;
    }
    const Vector2 cima(d.x, d.y - kMeiaAlturaDoLosango), baixo(d.x, d.y + kMeiaAlturaDoLosango);
    const Vector2 esquerda(d.x - kMeioLosango, d.y), direita(d.x + kMeioLosango, d.y);
    mFluxograma->Acrescentar(cima, direita, cor);
    mFluxograma->Acrescentar(direita, baixo, cor);
    mFluxograma->Acrescentar(baixo, esquerda, cor);
    mFluxograma->Acrescentar(esquerda, cima, cor);

    // AS SETAS DOS RAMOS: a que tem um balao andando nela, ou cujo confete
    // acabou de cair, acesa.
    const int ramos = Condicional::Ramos(mComando);
    for (int k = 0; k < ramos; ++k) {
        bool aceso = static_cast<size_t>(k) < mAcesoAte.size() && agora < mAcesoAte[static_cast<size_t>(k)];
        for (const Balao& b : mBaloes) {
            aceso = aceso || (b.etapa == Etapa::Voando && b.c == mComando && !b.ramos.empty() && b.ramos.front() == k);
        }
        const auto caminho = Caminho(mComando, k);
        for (size_t i = 1; i < caminho.size(); ++i) {
            mFluxograma->Acrescentar(caminho[i - 1], caminho[i], aceso ? kAceso : kApagado);
        }
    }
}

void Andre::DesmontarCondicional() {
    for (BossProjectile* p : mRotulos) if (Vivo(p)) p->Kill();
    for (const Balao& b : mBaloes) if (Vivo(b.p)) b.p->Kill();
    mRotulos.clear();
    mAcesoAte.clear();
    mBaloes.clear();
    mQuedas.clear();
    if (mFluxograma) mFluxograma->Limpar();
}

// ---------------------------------------------------------------------------
// A FASE 2: A REPETICAO
// ---------------------------------------------------------------------------

namespace {
    /// O cacho: o lugar de cada balao em relacao ao meio da casa, e a
    /// diferenca de velocidade dele - um cacho de verdade nao sobe em fila.
    constexpr float kDesvioDoCacho[Laco::kBaloesPorCacho] = {0.0f, 14.0f, 6.0f};
    constexpr float kVariacaoDoCacho[Laco::kBaloesPorCacho] = {0.0f, -12.0f, 10.0f};
    const char* const kCoresDoBalaoOriginal[] = {"Red", "Blue", "Yellow"};

    /// O continue acende o indice de azul por este tempo.
    constexpr float kPuladoAceso = 0.45f;
}

void Andre::AtualizarLaco(const float deltaTime) {
    (void)deltaTime;
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noLaco = fsm && fsm->GetStateName() == Laco::kFaseDoLaco;

    if (!noLaco) {
        if (mNoLaco) {
            DesmontarLaco();
            if (!mAnuncioDoLaco.empty()) RegistrarAnuncio(Laco::kFaseDoLaco, mAnuncioDoLaco);
            mAlvo.reset();
            mNoLaco = false;
        }
        return;
    }
    const float agora = fsm->GetStateTime();
    const SDL_FRect campo = Campo();
    if (!mNoLaco) {
        mNoLaco = true;
        mVezDoLaco = 0;
        mRodando = false;
        mProximoLaco = Laco::kPrimeiro;
        mErroAte = 0.0f;
        mCachos.clear();
        mSubindo.clear();
        if (mAnuncioDoLaco.empty()) mAnuncioDoLaco = AnuncioDaFase(Laco::kFaseDoLaco);

        // O VETOR NO CHAO: o indice de cada casa, embaixo.
        mIndices.clear();
        mAcesoDoIndice.assign(Laco::kCasas, -1.0f);
        mPuladoDoIndice.assign(Laco::kCasas, -1.0f);
        for (int i = 0; i < Laco::kCasas; ++i) {
            const Vector2 onde(campo.x + Laco::XDaCasa(i, campo.w), campo.y + campo.h - Laco::kAcimaDaBorda);
            const auto criados = DispararEm(Laco::kGatilhoDoIndice, onde);
            BossProjectile* p = criados.empty() ? nullptr : criados.front();
            mIndices.push_back(p);
            if (!p) continue;
            p->DefinirEnfeite(true);
            Parar(p);
            PorCara(p, Laco::CaraDoIndice(i));
        }
    }

    // O que o campo ja levou sai da conta antes de qualquer disparo.
    mSubindo.erase(std::remove_if(mSubindo.begin(), mSubindo.end(),
                                  [](const Subindo& s) { return !Vivo(s.p); }), mSubindo.end());
    for (BossProjectile*& p : mIndices) if (!Vivo(p)) p = nullptr;

    if (!mRodando && agora >= mProximoLaco) NovoLaco(agora);

    if (mRodando) {
        const float t = agora - mInicioDoLaco;
        while (mProximaVisita < mVisitas.size() && mVisitas[mProximaVisita].quando <= t) {
            Visitar(mVisitas[mProximaVisita++], agora);
        }
        // O JUIZ DO BOCA MATA O while (1): o placar da o veredito, e o que o
        // programa soltou some com ele.
        if (mPrograma.forma == Laco::Forma::Infinito && !mTempoEsgotado && t >= Laco::kLimiteDeTempo) {
            mTempoEsgotado = true;
            RegistrarAnuncio(Laco::kFaseDoLaco, Laco::kTempoEsgotado);
            for (const Subindo& s : mSubindo) if (s.laco == mVezDoLaco && Vivo(s.p)) s.p->Kill();
            mSubindo.erase(std::remove_if(mSubindo.begin(), mSubindo.end(),
                                          [this](const Subindo& s) { return s.laco == mVezDoLaco; }), mSubindo.end());
            mCachos.erase(std::remove_if(mCachos.begin(), mCachos.end(),
                                         [this](const Cacho& c) { return c.laco == mVezDoLaco; }), mCachos.end());
            for (float& ate : mAcesoDoIndice) ate = -1.0f;
        }
        if (t >= mDuracaoDoLaco) {
            mRodando = false;
            mProximoLaco = agora + Laco::kEntreLacos;
        }
    }

    for (auto it = mCachos.begin(); it != mCachos.end();) {
        if (agora < it->quando) { ++it; continue; }
        const Cacho c = *it;
        it = mCachos.erase(it);
        SoltarCacho(c.casa, c.laco);
    }

    for (int i = 0; i < static_cast<int>(mIndices.size()); ++i) {
        BossProjectile* p = mIndices[static_cast<size_t>(i)];
        if (!p) continue;
        Parar(p);
        const auto k = static_cast<size_t>(i);
        if (agora < mErroAte) PorCara(p, Laco::CaraDoIndiceErro(i));
        else if (agora < mAcesoDoIndice[k]) PorCara(p, Laco::CaraDoIndiceAceso(i));
        else if (agora < mPuladoDoIndice[k]) PorCara(p, Laco::CaraDoIndicePulado(i));
        else PorCara(p, Laco::CaraDoIndice(i));
    }
}

void Andre::NovoLaco(const float agora) {
    ++mVezDoLaco;
    mPrograma = Laco::Proximo(mVezDoLaco - 1, Sorteio());
    mVisitas = Laco::Execucao(mPrograma, Sorteio());
    mDuracaoDoLaco = Laco::Duracao(mPrograma, mVisitas);
    mProximaVisita = 0;
    mInicioDoLaco = agora;
    mRodando = true;
    mTempoEsgotado = false;
    RegistrarAnuncio(Laco::kFaseDoLaco, Laco::Placar(mPrograma));

    const SDL_FRect campo = Campo();
    const float fracao = static_cast<float>(Sorteio() % 1001u) / 1000.0f;
    mAlvo = Vector2(campo.x + Laco::kXMinimo + fracao * (Laco::kXMaximo - Laco::kXMinimo),
                    campo.y + Laco::kYDoProfessor);
}

void Andre::Visitar(const Laco::Visita& v, const float agora) {
    const SDL_FRect campo = Campo();
    switch (v.tipo) {
        case Laco::Tipo::Balao:
            if (v.i >= 0 && v.i < Laco::kCasas) mAcesoDoIndice[static_cast<size_t>(v.i)] = agora + Laco::kAviso;
            mCachos.push_back(Cacho{v.i, agora + Laco::kAviso, mVezDoLaco});
            break;
        case Laco::Tipo::Pulado:
            if (v.i >= 0 && v.i < Laco::kCasas) mPuladoDoIndice[static_cast<size_t>(v.i)] = agora + kPuladoAceso;
            break;
        case Laco::Tipo::Fora: {
            // O i SAIU DO VETOR: o programa capota, e a memoria despejada voa
            // da borda por onde ele saiu, para dentro do campo.
            RegistrarAnuncio(Laco::kFaseDoLaco, Laco::kSegfault);
            mErroAte = agora + Laco::kCapotando;
            const bool direita = v.i >= Laco::kCasas;
            const Vector2 borda(direita ? campo.x + campo.w : campo.x, campo.y + campo.h - Laco::kAcimaDoChao);
            const float angulo = direita ? 180.0f + Laco::kInclinacaoDoDespejo : 360.0f - Laco::kInclinacaoDoDespejo;
            const auto bytes = DispararEm(Laco::kGatilhoDoDespejo, borda, 0, angulo);
            for (size_t k = 0; k < bytes.size(); ++k) PorCara(bytes[k], Laco::CaraDoByte(static_cast<int>(k)));
            break;
        }
    }
}

void Andre::SoltarCacho(const int casa, const int laco) {
    const SDL_FRect campo = Campo();
    const Vector2 onde(campo.x + Laco::XDaCasa(casa, campo.w), campo.y + campo.h + Laco::kAbaixoDoChao);
    const auto baloes = DispararEm(Laco::kGatilhoDoCacho, onde);
    const char* cor = kCoresDoBalaoOriginal[Sorteio() % 3u];
    for (size_t k = 0; k < baloes.size() && k < static_cast<size_t>(Laco::kBaloesPorCacho); ++k) {
        const float lado = (static_cast<float>(k) - 1.0f) * Laco::kLadoDoCacho;
        baloes[k]->SetPosition(onde + Vector2(lado, kDesvioDoCacho[k]));
        if (auto* rb = baloes[k]->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2(0.0f, -(Laco::kVelocidadeDoCacho + kVariacaoDoCacho[k])));
        }
        PorCara(baloes[k], cor);
        if (mPrograma.forma == Laco::Forma::Infinito && laco == mVezDoLaco) mSubindo.push_back(Subindo{baloes[k], laco});
    }
}

void Andre::DesmontarLaco() {
    for (BossProjectile* p : mIndices) if (Vivo(p)) p->Kill();
    mIndices.clear();
    mAcesoDoIndice.clear();
    mPuladoDoIndice.clear();
    mCachos.clear();
    mSubindo.clear();
    mRodando = false;
}

// ---------------------------------------------------------------------------
// OS GANCHOS DO DISPARO
// ---------------------------------------------------------------------------

void Andre::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O QUE O ANDRE DISPARA NUM LUGAR nasce nele, no numero que ele pediu.
    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (mQuantos > 0) params.numProjectiles = mQuantos;
        if (mAnguloEm) params.centralAngle = *mAnguloEm;
        return;
    }

    // Angulo, em GRAUS, da direcao ate o player, na convencao de
    // AttackParams::centralAngle (0 = direita, 90 = baixo, 180 = esquerda).
    if (dynamic_cast<BaloonAttackParams*>(&params)) return;
    const Vector2 dirToPlayer = GetDirectionToPlayer();
    const auto directionToPlayerAngle = Math::ToDegrees(Math::Atan2(dirToPlayer.y, dirToPlayer.x));
    if (stateName == "StateThree") {
        params.projectileSpeed = 400.f;

        //Se o player tiver acima do Andre, atirar nele
        if (dirToPlayer.y < 0) {
            params.projectileSpeed = 600.f;
            params.numProjectiles = 36;
            params.centralAngle = directionToPlayerAngle;
        }
        else {
            params.numProjectiles = 18;
            params.centralAngle = Random::GetFloatRange(75.f, 105.f);
        }
    }
}

void Andre::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O QUE ELE DISPAROU NUM LUGAR, ele mesmo pinta e move.
    if (mDisparandoEm) {
        for (auto& p : projectiles) {
            if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
        }
        return;
    }

    // Uma cor por RAJADA, nao por projetil: o sorteio fica fora do laco, igual
    // ao codigo original.
    static const std::vector<std::string> colors = { "Red", "Blue", "Yellow" };
    const int choose = Random::GetIntRange(0, static_cast<int>(colors.size()) - 1);

    for (auto& p : projectiles) {
        if (!p) continue;
        // Guarda de nulo que a versao antiga nao tinha: ela dereferenciava
        // GetComponent<DrawAnimatedComponent>() direto.
        if (auto drawComp = p->GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation(colors[choose]);
        }
    }
}
