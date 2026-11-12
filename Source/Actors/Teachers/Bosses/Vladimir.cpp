//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#include "Vladimir.h"

#include <algorithm>
#include <limits>

#include "../../../Motor/Random.h"
#include "../../../Attacks/Alfabeto.h"
#include "../../../Attacks/Ancora.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Attacks/Fita.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../Player/Player.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// As arestas da arvore: o azul do diagrama do automato, esmaecido.
    constexpr SDL_Color kAresta{150, 170, 230, 150};
    constexpr int kOrdemDasLinhas = 84;   // embaixo dos nos e das marcas (ordemDeDesenho 88 e 90)

    /// A fita: as casas esmaecidas, e a cabeca no ambar do "a".
    constexpr SDL_Color kCasaDaFita{110, 120, 150, 110};
    constexpr SDL_Color kCabecaDaFita{236, 196, 110, 230};
}

Vladimir::Vladimir(Scene* scene) : Boss(scene) {
}

void Vladimir::SortearALuta() {
    mLutaSorteada = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    mDegrauDoCompilador = Compilador::DegrauDo(nivel);
    mDegrauDaFita = Fita::DegrauDo(nivel);
    mDegrauDaParada = Parada::DegrauDo(nivel);
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Fita::kFaseDaFita);
        mPlanoDaFita = Fita::Sortear(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Parada::kFaseDaParada);
        mPlanoDaParada = Parada::Sortear(sorteio);
    }
    {
        Variacao::Sorteio sorteio = SorteioDaFase(Compilador::kFaseDoCompilador);
        mPlanoDoCompilador = Compilador::Sortear(sorteio);
    }
}

void Vladimir::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarAutomato();
    AtualizarCompilador(deltaTime);
    AtualizarFita();
    AtualizarParada(deltaTime);
}

void Vladimir::AtualizarFita() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naFita = fsm && fsm->GetStateName() == Fita::kFaseDaFita && !EhClone();
    if (!naFita) {
        if (mNaFita) {
            for (BossProjectile* marca : NoPosto(Fita::kPostoDaFita)) marca->Kill();
            mMarcas.fill(nullptr);
            if (mLinhas) mLinhas->Limpar();
            if (!mAnuncioDaFita.empty()) RegistrarAnuncio(Fita::kFaseDaFita, mAnuncioDaFita);
            mAlvo.reset();
        }
        mNaFita = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto casa = [&](const int k) { return Vector2(Fita::XDaCasa(k, largura), Fita::kYDaFita); };

    if (!mNaFita) {
        // A fita em branco, a cabeca na origem, em q0.
        mNaFita = true;
        mFita = Fita::Inicio(mPlanoDaFita.espelho);
        mProximoPassoDaFita = Fita::kPrimeiroPasso;
        mMarcas.fill(nullptr);
        if (mAnuncioDaFita.empty()) mAnuncioDaFita = AnuncioDaFase(Fita::kFaseDaFita);
        RegistrarAnuncio(Fita::kFaseDaFita, mAnuncioDaFita);
        CriarLinhas();
    }

    // A marca que nao esta mais na fileira sai da conta (o pool pode te-la dado a outro tiro).
    for (BossProjectile*& marca : mMarcas) {
        if (marca && (marca->GetState() != ActorState::Active || marca->Posto() != Fita::kPostoDaFita)) marca = nullptr;
    }

    const float agora = fsm->GetStateTime();
    while (!mFita.parou && agora >= mProximoPassoDaFita) {
        mProximoPassoDaFita += Fita::kPasso;
        const Fita::Passo p = Fita::Avancar(mFita, mPlanoDaFita.espelho);
        const Vector2 onde = casa(p.casa);
        BossProjectile*& marca = mMarcas[static_cast<size_t>(p.casa)];

        if (!p.leu && p.escreveu) {
            // Escreve: a marca nova fica na casa.
            const auto nascidas = DispararDe(Fita::kGatilhoDaEscrita, onde, 90.0f, "");
            if (!nascidas.empty()) marca = nascidas.front();
        } else if (p.leu && !p.escreveu) {
            // Apaga: a marca sai da fita no aluno.
            if (marca) { marca->Kill(); marca = nullptr; }
            DispararDe(mDegrauDaFita.apagadaPersegue ? Fita::kGatilhoDaReescrita : Fita::kGatilhoDoApagar,
                       onde, std::nullopt, "");

        } else if (p.leu && p.escreveu && mDegrauDaFita.relerAtira) {
            // Rele, no Dificil: a marca fica, e atira uma copia de si no aluno.
            DispararDe(Fita::kGatilhoDoReler, onde, std::nullopt, "");
        }

        RegistrarAnuncio(Fita::kFaseDaFita, Fita::Placar(mFita, p));
        if (mFita.parou) {
            // Parou: o anel.
            DispararDe(Fita::kGatilhoDaParada, GetPosition(), 90.0f, "");
        }
    }

    const int naCabeca = std::clamp(mFita.cabeca, 0, Fita::kCasas - 1);
    mAlvo = Vector2(Fita::XDoProfessor(casa(naCabeca).x, largura), Fita::kAlturaDaCabeca);

    DesenharCabeca();
}

void Vladimir::DesenharCabeca() {
    if (!mLinhas || !mScene || !mScene->GetGame()) return;
    mLinhas->Limpar();
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());

    // A fita: a fileira de casas, apagada; e a cabeca, o quadro aceso em volta da casa que ela le.
    const float meia = Fita::kEntreCasas / 2.0f - 3.0f;
    const auto quadro = [&](const float x, const SDL_Color cor) {
        const Vector2 a(x - meia, Fita::kYDaFita - meia), b(x + meia, Fita::kYDaFita - meia);
        const Vector2 c(x + meia, Fita::kYDaFita + meia), d(x - meia, Fita::kYDaFita + meia);
        mLinhas->Acrescentar(a, b, cor);
        mLinhas->Acrescentar(b, c, cor);
        mLinhas->Acrescentar(c, d, cor);
        mLinhas->Acrescentar(d, a, cor);
    };
    for (int k = 0; k < Fita::kCasas; ++k) quadro(Fita::XDaCasa(k, largura), kCasaDaFita);
    const float x = Fita::XDaCasa(std::clamp(mFita.cabeca, 0, Fita::kCasas - 1), largura);
    quadro(x, kCabecaDaFita);
    mLinhas->Acrescentar(GetPosition(), Vector2(x, Fita::kYDaFita - meia), kCabecaDaFita);
}

void Vladimir::AtualizarParada(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naParada = fsm && fsm->GetStateName() == Parada::kFaseDaParada && !EhClone();
    if (!naParada) {
        if (mNaParada) {
            if (!mAnuncioDaParada.empty()) RegistrarAnuncio(Parada::kFaseDaParada, mAnuncioDaParada);
            mNaParada = false;
            mAlvo.reset();
        }
        return;
    }
    if (!mNaParada) {
        mNaParada = true;
        mX = 0;
        mProximoPassoDaParada = Parada::kPrimeiro;
        mXDaParada = GetPosition().x;
        mSentidoDaParada = mPlanoDaParada.sentido;
        mProgramaDaVez = 0;
        if (mAnuncioDaParada.empty()) mAnuncioDaParada = AnuncioDaFase(Parada::kFaseDaParada);
        RegistrarAnuncio(Parada::kFaseDaParada, mAnuncioDaParada);
    }

    // De lado a lado, devagar, e na altura do valor de x: o granizo.
    if (mScene && mScene->GetGame()) {
        const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
        mXDaParada += mSentidoDaParada * kVarreduraDaParada * deltaTime;
        if (mXDaParada > largura - kMargemDaParada) { mXDaParada = largura - kMargemDaParada; mSentidoDaParada = -1.0f; }
        if (mXDaParada < kMargemDaParada)          { mXDaParada = kMargemDaParada;          mSentidoDaParada = 1.0f; }
    }
    mAlvo = Vector2(mXDaParada, Parada::Altura(mX == 0 ? 1u : mX));

    const float agora = fsm->GetStateTime();
    if (agora < mProximoPassoDaParada) return;

    if (mX == 0) {
        // Um programa novo, da proxima semente da luta.
        mX = Parada::SementeDaVez(mPlanoDaParada, mProgramaDaVez++);
        mProximoPassoDaParada += mDegrauDaParada.passo;
    } else if (!Parada::Parou(mX)) {
        // Um passo: atira conforme a paridade.
        Disparar(mX % 2u == 0u ? Parada::kGatilhoDaMetade : Parada::kGatilhoDoTriplo, -1, mDegrauDaParada.rapidez);
        mX = Parada::Proximo(mX);
        mProximoPassoDaParada += mDegrauDaParada.passo;
    } else {
        // Parou: o anel, e depois de um tempo, outro programa.
        Disparar(Parada::kGatilhoDeParar);
        mX = 0;
        mProximoPassoDaParada += Parada::kParado;
        RegistrarAnuncio(Parada::kFaseDaParada, Parada::Placar(1u));
        return;
    }
    RegistrarAnuncio(Parada::kFaseDaParada, Parada::Placar(mX));
}

std::string Vladimir::AvisoDoIntercambio() const {
    return Alfabeto::kAvisoDaTroca;
}

std::optional<Vector2> Vladimir::AlvoDoMovimento() const {
    return mAlvo;
}

std::string Vladimir::CaraDoEstado(const int q, const bool aceso) {
    return "Q" + std::to_string(q) + (aceso ? "Aceso" : "");
}

Vector2 Vladimir::CentroDoEstado(const int q) {
    const auto i = static_cast<size_t>(q < 0 || q >= Automato::kEstados ? 0 : q);
    return Vector2(Automato::kXDosEstados[i], Automato::kYDosEstados[i]);
}

std::vector<BossProjectile*> Vladimir::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = Batalha();
    if (!batalha || !batalha->GerenteDeProjeteis()) return achados;
    for (const auto& p : batalha->GerenteDeProjeteis()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

void Vladimir::OnIntercambio() {
    const auto* batalha = Batalha();
    if (!batalha || !batalha->GerenteDeProjeteis()) return;

    // O "a" ganha o Tracking do "b", e o "b" perde o dele e segue reto no rumo em que estava.
    for (const auto& p : batalha->GerenteDeProjeteis()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active) continue;
        auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (!anim || !Alfabeto::Troca(anim->GetAnimation(), p->Posto(), p->Enfeite())) continue;

        const std::string nova = Alfabeto::Trocar(anim->GetAnimation());
        anim->SetAnimation(nova);
        if (nova == Alfabeto::kB) {
            p->insertMotion<TrackingBehavior>(kAtrasoDaTroca, kForcaDaTroca, kDuracaoDaTroca);
        } else {
            p->DescartarMotion();
        }
    }

    // E o que o automato ainda vai ler tambem troca: o placar muda, e com ele o caminho do professor.
    if (mNoAutomato && mMomento == Momento::Lendo) {
        Automato::Trocar(mLeitura);
        RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
    }
}

void Vladimir::Formar(const char* gatilho, const Vector2 onde, std::string cara) {
    mFormando = true;
    mDisparoDoFormando = onde;
    mCaraDoFormando = std::move(cara);
    Disparar(gatilho);
    mFormando = false;
}

void Vladimir::AtualizarAutomato() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noAutomato = fsm && fsm->GetStateName() == Automato::kFaseDoAutomato && !EhClone();

    if (!noAutomato) {
        if (mNoAutomato) {
            for (BossProjectile* p : NoPosto(Automato::kPostoDosEstados)) p->Kill();
            if (!mAnuncioDoAutomato.empty()) RegistrarAnuncio(Automato::kFaseDoAutomato, mAnuncioDoAutomato);
            mNoAutomato = false;
            mAlvo.reset();
        }
        return;
    }
    if (!mNoAutomato) {
        mNoAutomato = true;
        mAutomatoFormado = false;
        mMomento = Momento::Anuncio;
        mProximoPasso = kPrimeiraPalavra;
        mLeitura = Automato::Leitura{};
        if (mAnuncioDoAutomato.empty()) mAnuncioDoAutomato = AnuncioDaFase(Automato::kFaseDoAutomato);
        RegistrarAnuncio(Automato::kFaseDoAutomato, mAnuncioDoAutomato);
        mAlvo = CentroDoEstado(Automato::kInicial);
    }
    const float agora = fsm->GetStateTime();

    if (!mAutomatoFormado && agora >= kFormaOAutomato) {
        Formar(Automato::kGatilhoDoDiagrama, Vector2(Automato::kXDoDiagrama, Automato::kYDoDiagrama), "");
        for (int q = 0; q < Automato::kEstados; ++q) {
            Formar(Automato::kGatilhoDosEstados, CentroDoEstado(q), CaraDoEstado(q, false));
        }
        mAutomatoFormado = true;
    }

    if (agora >= mProximoPasso) {
        mProximoPasso += (mMomento == Momento::Lendo && Automato::Terminou(mLeitura)) ? kVeredito : kPasso;

        if (mMomento != Momento::Lendo) {
            // Uma palavra nova, sempre do estado inicial.
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            mLeitura = Automato::Leitura{Automato::Sortear(sorteio), 0, Automato::kInicial};
            mMomento = Momento::Lendo;
            mAlvo = CentroDoEstado(Automato::kInicial);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else if (!Automato::Terminou(mLeitura)) {
            // Um simbolo: a letra sai do estado em que ele esta, e ele voa.
            const Automato::Passo passo = Automato::Ler(mLeitura);
            Disparar(passo.letra == 'a' ? Automato::kGatilhoDoA : Automato::kGatilhoDoB);
            mAlvo = CentroDoEstado(passo.para);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else {
            // O veredito: aceita, e o estado de aceitacao estoura num anel.
            Disparar(Automato::Aceita(mLeitura.estado) ? Automato::kGatilhoDeAceitar
                                                       : Automato::kGatilhoDeRejeitar);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Veredito(mLeitura));
            mMomento = Momento::Veredito;
        }
    }

    // O estado em que ele esta fica aceso - lendo, ou no veredito.
    const bool lendo = mMomento != Momento::Anuncio;
    for (BossProjectile* p : NoPosto(Automato::kPostoDosEstados)) {
        auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (!anim || anim->GetAnimation() == Automato::kCaraDoDiagrama) continue;
        for (int q = 0; q < Automato::kEstados; ++q) {
            if ((p->GetPosition() - CentroDoEstado(q)).LengthSq() > 1.0f) continue;
            const std::string cara = CaraDoEstado(q, lendo && q == mLeitura.estado);
            if (anim->GetAnimation() != cara) anim->SetAnimation(cara);
        }
    }
}

std::vector<BossProjectile*> Vladimir::DispararDe(const char* gatilho, const Vector2 onde,
                                                  const std::optional<float> rumo, const std::string& cara) {
    mDisparandoDe = true;
    mOndeDisparar = onde;
    if (rumo) {
        mRumoDoDisparo = *rumo;
    } else {
        const Vector2 d = PosicaoDoAluno() - onde;
        mRumoDoDisparo = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
    mCaraDoDisparo = cara;
    mDisparados.clear();
    Disparar(gatilho);
    mDisparandoDe = false;
    return mDisparados;
}

void Vladimir::CriarLinhas() {
    if (mLinhas || !mScene) return;
    auto ator = std::make_unique<Actor>(mScene);
    mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
    mScene->AddActor(std::move(ator));
}

void Vladimir::ComecarRodada(const int rodada, const float inicio) {
    // O que sobrou da linha anterior sai.
    for (BossProjectile* no : NoPosto(Compilador::kPostoDaArvore)) no->Kill();
    mRodada = rodada;
    mInicioDaRodada = inicio;
    mLinha = mPlanoDoCompilador.linhas[static_cast<size_t>(rodada)];
    mTokens = Compilador::Lexico(mLinha);
    mArvore = Compilador::Sintatico(mTokens);
    mOrdem = Compilador::PosOrdem(mArvore);
    const auto largura = static_cast<float>(mScene && mScene->GetGame() ? mScene->GetGame()->GetWindowWidth() : 1200);
    mNaLinha = Compilador::PosicoesDaLinha(mTokens, largura);
    mNaArvore = Compilador::PosicoesDaArvore(mArvore, largura);
    mMarcos = Compilador::MarcosDa(static_cast<int>(mTokens.size()), static_cast<int>(mArvore.nos.size()),
                                   mDegrauDoCompilador.porInstrucao);
    mLidos = mMontados = mEmitidos = 0;
    mNos.assign(mArvore.nos.size(), nullptr);
    mLexemas.assign(mTokens.size(), nullptr);
    mFilaParada = false;
    mEtapa = Compilador::Etapa::Lexico;
    RegistrarAnuncio(Compilador::kFaseDoCompilador, Compilador::Placar(mLinha, mEtapa));
}

void Vladimir::AtualizarCompilador(const float deltaTime) {
    using namespace Compilador;
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noCompilador = fsm && fsm->GetStateName() == kFaseDoCompilador && !EhClone();

    if (!noCompilador) {
        if (mNoCompilador) {
            for (BossProjectile* no : NoPosto(kPostoDaArvore)) no->Kill();
            mNos.clear();
            mLexemas.clear();
            if (mLinhas) mLinhas->Limpar();
            if (!mAnuncioDoCompilador.empty()) RegistrarAnuncio(kFaseDoCompilador, mAnuncioDoCompilador);
            mNoCompilador = false;
            mAlvo.reset();
        }
        return;
    }
    if (!mNoCompilador) {
        mNoCompilador = true;
        mRodada = -1;
        mNos.clear();
        mLexemas.clear();
        if (mAnuncioDoCompilador.empty()) mAnuncioDoCompilador = AnuncioDaFase(kFaseDoCompilador);
        RegistrarAnuncio(kFaseDoCompilador, mAnuncioDoCompilador);
        CriarLinhas();
        const auto largura = static_cast<float>(mScene && mScene->GetGame() ? mScene->GetGame()->GetWindowWidth() : 1200);
        mAlvo = Vector2(largura / 2.0f, kYDaLinha);
    }

    // Uma rodada nova a cada kRodada, e nao mais que kRodadas.
    const float agora = fsm->GetStateTime();
    if (agora >= kPrimeira) {
        const int rodada = std::min(kRodadas - 1, static_cast<int>((agora - kPrimeira) / kRodada));
        if (rodada > mRodada) ComecarRodada(rodada, kPrimeira + static_cast<float>(rodada) * kRodada);
    }
    if (mRodada < 0) return;
    const float t = agora - mInicioDaRodada;

    // A ficha ou o no que nao esta mais em campo sai da conta (o pool pode te-lo dado a outro tiro).
    for (auto* lista : {&mNos, &mLexemas}) {
        for (BossProjectile*& p : *lista) {
            if (p && (p->GetState() != ActorState::Active || p->Posto() != kPostoDaArvore)) p = nullptr;
        }
    }
    const auto lugar = [&](const int n) {
        const Ponto& p = mNaArvore[static_cast<size_t>(n)];
        return Vector2(p.x, p.y);
    };
    const auto nos = static_cast<int>(mOrdem.size());

    // O lexico: cada token reconhecido cai em coluna, do lugar dele na linha.
    while (mLidos < static_cast<int>(mTokens.size()) && t >= static_cast<float>(mLidos) * kPorToken) {
        const Token& tk = mTokens[static_cast<size_t>(mLidos)];
        const Vector2 onde(mNaLinha[static_cast<size_t>(mLidos)].x, mNaLinha[static_cast<size_t>(mLidos)].y);
        if (tk.texto == "a") DispararDe(kGatilhoDoA, onde, 90.0f, "");
        else if (tk.texto == "b") DispararDe(kGatilhoDoB, onde, 90.0f, "");
        else {
            const auto nascidos = DispararDe(kGatilhoDoLexema, onde, 90.0f, CaraDoLexema(tk.texto));
            if (!nascidos.empty()) mLexemas[static_cast<size_t>(mLidos)] = nascidos.front();
        }
        ++mLidos;
    }

    // O parser comeca: a fila de tokens para onde esta, para ele a pegar - e o.
    if (!mFilaParada && t >= mMarcos.sintatico) {
        mFilaParada = true;
        for (size_t k = 0; k < mLexemas.size(); ++k) {
            BossProjectile* p = mLexemas[k];
            if (!p) continue;
            if (mTokens[k].tipo == Tipo::Fim) { p->Kill(); mLexemas[k] = nullptr; continue; }
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }

    while (mMontados < nos && t >= mMarcos.sintatico + static_cast<float>(mMontados) * kPorNo) {
        const int n = mOrdem[static_cast<size_t>(mMontados++)];
        const No& no = mArvore.nos[static_cast<size_t>(n)];
        const auto k = static_cast<size_t>(no.token);
        BossProjectile*& ficha = mLexemas[k];
        if (ficha) {
            ficha->DefinirEnfeite(true);
            if (auto* anim = ficha->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(CaraDoNo(mArvore, n));
            mNos[static_cast<size_t>(n)] = ficha;
            ficha = nullptr;
        } else {
            // O a E O b seguiram caindo como letras: o no deles nasce na linha.
            const auto nascidos = DispararDe(kGatilhoDoNo, Vector2(mNaLinha[k].x, mNaLinha[k].y), 90.0f, CaraDoNo(mArvore, n));
            mNos[static_cast<size_t>(n)] = nascidos.empty() ? nullptr : nascidos.front();
        }

        if (no.tipo == Tipo::Op) DispararDe(kGatilhoDaReducao, lugar(n), std::nullopt, CaraDoLexema(no.texto));
    }

    // O alvo do = nao gera nada e espera o =; o = sai no anel do store.
    while (mEmitidos < nos && t >= mMarcos.codigo + static_cast<float>(mEmitidos) * mDegrauDoCompilador.porInstrucao) {
        const int n = mOrdem[static_cast<size_t>(mEmitidos++)];
        const std::string instrucao = Instrucao(mArvore, n);
        if (instrucao.empty()) continue;
        const Vector2 onde = mNos[static_cast<size_t>(n)] ? mNos[static_cast<size_t>(n)]->GetPosition() : lugar(n);
        for (const int k : {n, n == mArvore.raiz ? mArvore.nos[static_cast<size_t>(n)].esq : -1}) {
            if (k < 0) continue;
            if (BossProjectile*& p = mNos[static_cast<size_t>(k)]) { p->Kill(); p = nullptr; }
        }
        DispararDe(n == mArvore.raiz ? kGatilhoDoStore : kGatilhoDaInstrucao, onde, std::nullopt, CaraDaInstrucao(instrucao));
        RegistrarAnuncio(kFaseDoCompilador, Placar(mLinha, Etapa::Codigo, instrucao));
    }

    // A etapa mudou: o placar diz qual e (na geracao de codigo, e a instrucao que acabou de sair).
    const Etapa etapa = EtapaEm(t, mMarcos);
    if (etapa != mEtapa) {
        mEtapa = etapa;
        if (etapa != Etapa::Codigo) RegistrarAnuncio(kFaseDoCompilador, Placar(mLinha, etapa));
    }

    float x = mScene && mScene->GetGame() ? static_cast<float>(mScene->GetGame()->GetWindowWidth()) / 2.0f
                                          : GetPosition().x;

    if (mLidos < static_cast<int>(mTokens.size())) x = mNaLinha[static_cast<size_t>(mLidos)].x;
    mAlvo = Vector2(x, kYDaLinha);

    // Cada no anda ate o lugar dele, e as arestas o seguem.
    for (int n = 0; n < static_cast<int>(mNos.size()); ++n) {
        BossProjectile* no = mNos[static_cast<size_t>(n)];
        if (!no) continue;
        no->SetPosition(Ancora::Aproximar(no->GetPosition(), lugar(n), kRapidezDoNo, deltaTime));
        if (auto* rb = no->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    DesenharArestas();
}

void Vladimir::DesenharArestas() {
    if (!mLinhas) return;
    mLinhas->Limpar();
    for (size_t n = 0; n < mNos.size() && n < mArvore.nos.size(); ++n) {
        if (!mNos[n]) continue;
        const Compilador::No& no = mArvore.nos[n];
        for (const int f : {no.esq, no.dir}) {
            if (f < 0 || !mNos[static_cast<size_t>(f)]) continue;
            mLinhas->Acrescentar(mNos[n]->GetPosition(), mNos[static_cast<size_t>(f)]->GetPosition(), kAresta);
        }
    }
}

void Vladimir::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O que o compilador ou a fita soltou: a cara de cada um, e quem nasceu.
    if (mDisparandoDe) {
        for (auto& p : projectiles) {
            auto* nascido = dynamic_cast<BossProjectile*>(p.get());
            if (!nascido) continue;
            if (nascido->Posto() == Compilador::kPostoDaArvore && stateName == Compilador::kFaseDoCompilador &&
                mFilaParada) {
                nascido->DefinirEnfeite(true);
            }
            if (auto* anim = nascido->GetComponent<DrawAnimatedComponent>(); anim && !mCaraDoDisparo.empty()) {
                anim->SetAnimation(mCaraDoDisparo);
            }
            mDisparados.push_back(nascido);
        }
        return;
    }
    if (!mFormando) return;

    // O diagrama e os estados sao o mapa, e nao tiro.
    for (auto& p : projectiles) {
        auto* parte = dynamic_cast<BossProjectile*>(p.get());
        if (!parte) continue;
        parte->DefinirEnfeite(true);
        if (auto* anim = parte->GetComponent<DrawAnimatedComponent>(); anim && !mCaraDoFormando.empty()) {
            anim->SetAnimation(mCaraDoFormando);
        }
    }
}

void Vladimir::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O que sai de um lugar (o compilador, a fita) sai de onde ele manda, no rumo dele.
    if (mDisparandoDe) {
        const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
        params.firePosition = mOndeDisparar;
        params.centralAngle = mRumoDoDisparo + (par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f);
        return;
    }

    if (stateName != Automato::kFaseDoAutomato && stateName != Parada::kFaseDaParada) return;

    // O mapa nasce onde fica.
    if (mFormando) {
        params.firePosition = mDisparoDoFormando;
        return;
    }

    // A letra lida sai mirada no aluno, com um tiro no meio do leque.
    const Vector2 d = GetDirectionToPlayer();
    const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
    const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f;
    params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
}
