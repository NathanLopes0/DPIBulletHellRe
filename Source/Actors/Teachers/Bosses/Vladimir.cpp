//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#include "Vladimir.h"

#include <algorithm>
#include <limits>

#include "../../../Random.h"
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
    AtualizarFita(deltaTime);
    AtualizarParada(deltaTime);
}

void Vladimir::AtualizarFita(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naFita = fsm && fsm->GetStateName() == Fita::kFaseDaFita && !EhClone();
    if (!naFita) {
        if (mNaFita) {
            if (!mAnuncioDaFita.empty()) RegistrarAnuncio(Fita::kFaseDaFita, mAnuncioDaFita);
            mAlvo.reset();
        }
        mNaFita = false;
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto ponta = [&](const int sentido) {
        return Vector2(sentido > 0 ? largura - Fita::kMargem : Fita::kMargem, Fita::kAlturaDaCabeca);
    };

    if (!mNaFita) {
        // A CABECA COMECA INDO PARA A DIREITA, em q0, escrevendo desde ja.
        mNaFita = true;
        mSentidoDaFita = 1;
        mPausaNaPonta = 0.0f;
        mProximaEscrita = mPlanoDaFita.primeiraEscrita;
        if (mAnuncioDaFita.empty()) mAnuncioDaFita = AnuncioDaFase(Fita::kFaseDaFita);
        mAlvo = ponta(mSentidoDaFita);
        RegistrarAnuncio(Fita::kFaseDaFita, Fita::Transicao(true));
    }

    // NA PONTA: parada, sem escrever, ate a pausa acabar - e entao volta.
    if (mPausaNaPonta > 0.0f) {
        mPausaNaPonta -= deltaTime;
        if (mPausaNaPonta <= 0.0f) {
            mSentidoDaFita = -mSentidoDaFita;
            mAlvo = ponta(mSentidoDaFita);
            mProximaEscrita = mPlanoDaFita.primeiraEscrita;
            RegistrarAnuncio(Fita::kFaseDaFita, Fita::Transicao(mSentidoDaFita > 0));
        }
        return;
    }

    // CHEGOU A PONTA: le o branco e troca de estado.
    if ((GetPosition() - ponta(mSentidoDaFita)).LengthSq() <= 1.0f) {
        mPausaNaPonta = mPlanoDaFita.pausaNaPonta;
        RegistrarAnuncio(Fita::kFaseDaFita, Fita::Branco(mSentidoDaFita > 0));
        return;
    }

    // ANDANDO, ESCREVE: uma casa a cada escrita do degrau (Fita::kEscrita no
    // Normal).
    mProximaEscrita -= deltaTime;
    if (mProximaEscrita <= 0.0f) {
        Disparar(Fita::kGatilhoDaEscrita);
        mProximaEscrita += mDegrauDaFita.escrita;
    }
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

    // DE LADO A LADO, devagar, e na ALTURA DO VALOR DE X: o granizo.
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
        // UM PROGRAMA NOVO, da proxima semente da luta.
        mX = Parada::SementeDaVez(mPlanoDaParada, mProgramaDaVez++);
        mProximoPassoDaParada += mDegrauDaParada.passo;
    } else if (!Parada::Parou(mX)) {
        // UM PASSO: atira conforme a paridade - a metade sao "a" retos, o
        // 3x + 1 sao "b" que perseguem -, e da o passo.
        Disparar(mX % 2u == 0u ? Parada::kGatilhoDaMetade : Parada::kGatilhoDoTriplo, -1, mDegrauDaParada.rapidez);
        mX = Parada::Proximo(mX);
        mProximoPassoDaParada += mDegrauDaParada.passo;
    } else {
        // PAROU: o anel, e depois de um tempo, outro programa.
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
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

void Vladimir::OnIntercambio() {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return;

    // TODA LETRA EM VOO TROCA DE LETRA E DE COMPORTAMENTO, no quadro em que os
    // alunos trocam de lugar. O "a" ganha o Tracking do "b", e o "b" perde o
    // dele e segue reto no rumo em que estava - o modulo nao muda nos dois
    // casos, entao nenhuma letra acelera de surpresa.
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
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

    // E O QUE O AUTOMATO AINDA VAI LER TAMBEM TROCA: o placar muda, e com ele
    // o caminho do professor. O que ja foi lido fica como foi.
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
        // A FASE ACABOU: o automato se desfaz, o placar volta ao que o arquivo
        // diz, e o professor deixa de ter para onde ir.
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
            // UMA PALAVRA NOVA, sempre do estado inicial: o professor volta ao
            // q0 sem atirar, e o placar mostra o que ele vai ler.
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            mLeitura = Automato::Leitura{Automato::Sortear(sorteio), 0, Automato::kInicial};
            mMomento = Momento::Lendo;
            mAlvo = CentroDoEstado(Automato::kInicial);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else if (!Automato::Terminou(mLeitura)) {
            // UM SIMBOLO: a letra sai do estado em que ele esta, e ele voa.
            const Automato::Passo passo = Automato::Ler(mLeitura);
            Disparar(passo.letra == 'a' ? Automato::kGatilhoDoA : Automato::kGatilhoDoB);
            mAlvo = CentroDoEstado(passo.para);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else {
            // O VEREDITO: aceita, e o estado de aceitacao estoura num anel; ou
            // rejeita, e saem perseguidores de onde ele parou.
            Disparar(Automato::Aceita(mLeitura.estado) ? Automato::kGatilhoDeAceitar
                                                       : Automato::kGatilhoDeRejeitar);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Veredito(mLeitura));
            mMomento = Momento::Veredito;
        }
    }

    // O ESTADO EM QUE ELE ESTA FICA ACESO - lendo, ou no veredito.
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

namespace {
    /// As arestas da arvore: o azul do diagrama do automato, esmaecido.
    constexpr SDL_Color kAresta{150, 170, 230, 150};
    constexpr int kOrdemDasArestas = 84;   // embaixo dos nos (ordemDeDesenho 88)

    /// O que a legenda diz antes da primeira linha.
    const char* const kAberturaDoCompilador =
        "Constru\xc3\xa7\xc3\xa3o de Compiladores: tr\xc3\xaas linhas de c\xc3\xb3" "digo, "
        "e cada uma passa pelo l\xc3\xa9xico, pelo sint\xc3\xa1tico e pela gera\xc3\xa7\xc3\xa3o de c\xc3\xb3" "digo.";
}

Vector2 Vladimir::PosicaoDoAluno() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene); battle && battle->Alvo()) {
        return battle->Alvo()->GetPosition();
    }
    return GetPosition() + GetDirectionToPlayer() * GetDistanceToPlayer();
}

std::vector<BossProjectile*> Vladimir::Compilar(const char* gatilho, const Vector2 onde,
                                                const std::optional<float> rumo, const std::string& cara) {
    mCompilando = true;
    mOndeCompilar = onde;
    if (rumo) {
        mRumoDoCompilado = *rumo;
    } else {
        const Vector2 d = PosicaoDoAluno() - onde;
        mRumoDoCompilado = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
    mCaraDoCompilado = cara;
    mCompilados.clear();
    Disparar(gatilho);
    mCompilando = false;
    return mCompilados;
}

void Vladimir::ComecarRodada(const int rodada, const float inicio) {
    // O QUE SOBROU DA LINHA ANTERIOR SAI: a rodada so termina quando o STORE
    // sai, mas um no que o relogio nao alcancou nao fica no campo.
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
    RegistrarLegenda(Compilador::kFaseDoCompilador, Compilador::Legenda(mEtapa));
}

void Vladimir::AtualizarCompilador(const float deltaTime) {
    using namespace Compilador;
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noCompilador = fsm && fsm->GetStateName() == kFaseDoCompilador && !EhClone();

    if (!noCompilador) {
        // A FASE ACABOU: a arvore se desfaz, e o placar e a legenda voltam ao
        // que eram.
        if (mNoCompilador) {
            for (BossProjectile* no : NoPosto(kPostoDaArvore)) no->Kill();
            mNos.clear();
            mLexemas.clear();
            if (mArestas) mArestas->Limpar();
            if (!mAnuncioDoCompilador.empty()) RegistrarAnuncio(kFaseDoCompilador, mAnuncioDoCompilador);
            RegistrarLegenda(kFaseDoCompilador, "");
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
        RegistrarLegenda(kFaseDoCompilador, kAberturaDoCompilador);
        if (!mArestas && mScene) {
            auto ator = std::make_unique<Actor>(mScene);
            mArestas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasArestas);
            mScene->AddActor(std::move(ator));
        }
        const auto largura = static_cast<float>(mScene && mScene->GetGame() ? mScene->GetGame()->GetWindowWidth() : 1200);
        mAlvo = Vector2(largura / 2.0f, kYDaLinha);
    }

    // UMA RODADA NOVA a cada kRodada, e nao mais que kRodadas.
    const float agora = fsm->GetStateTime();
    if (agora >= kPrimeira) {
        const int rodada = std::min(kRodadas - 1, static_cast<int>((agora - kPrimeira) / kRodada));
        if (rodada > mRodada) ComecarRodada(rodada, kPrimeira + static_cast<float>(rodada) * kRodada);
    }
    if (mRodada < 0) return;
    const float t = agora - mInicioDaRodada;

    // A FICHA OU O NO QUE NAO ESTA MAIS EM CAMPO sai da conta (o pool pode
    // te-lo dado a outro tiro).
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

    // O LEXICO: cada token reconhecido cai em coluna, do lugar dele na linha. O
    // a e o b caem como as letras caem - o "b" persegue, e o intercambio os
    // troca em voo.
    while (mLidos < static_cast<int>(mTokens.size()) && t >= static_cast<float>(mLidos) * kPorToken) {
        const Token& tk = mTokens[static_cast<size_t>(mLidos)];
        const Vector2 onde(mNaLinha[static_cast<size_t>(mLidos)].x, mNaLinha[static_cast<size_t>(mLidos)].y);
        if (tk.texto == "a") Compilar(kGatilhoDoA, onde, 90.0f, "");
        else if (tk.texto == "b") Compilar(kGatilhoDoB, onde, 90.0f, "");
        else {
            const auto nascidos = Compilar(kGatilhoDoLexema, onde, 90.0f, CaraDoLexema(tk.texto));
            if (!nascidos.empty()) mLexemas[static_cast<size_t>(mLidos)] = nascidos.front();
        }
        ++mLidos;
    }

    // O PARSER COMECA: a fila de tokens para onde esta, para ele a pegar - e o
    // ; , que so termina a linha, sai.
    if (!mFilaParada && t >= mMarcos.sintatico) {
        mFilaParada = true;
        for (size_t k = 0; k < mLexemas.size(); ++k) {
            BossProjectile* p = mLexemas[k];
            if (!p) continue;
            if (mTokens[k].tipo == Tipo::Fim) { p->Kill(); mLexemas[k] = nullptr; continue; }
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }

    // O SINTATICO: os nos em pos-ordem, cada um voando de onde o token dele
    // parou ate o lugar dele na arvore; o operador, reduzido, solta um leque de
    // si mesmo no aluno.
    while (mMontados < nos && t >= mMarcos.sintatico + static_cast<float>(mMontados) * kPorNo) {
        const int n = mOrdem[static_cast<size_t>(mMontados++)];
        const No& no = mArvore.nos[static_cast<size_t>(n)];
        const auto k = static_cast<size_t>(no.token);
        BossProjectile*& ficha = mLexemas[k];
        if (ficha) {
            // A FICHA DO TOKEN VIRA O NO: e o mapa agora, e nao tiro.
            ficha->DefinirEnfeite(true);
            if (auto* anim = ficha->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(CaraDoNo(mArvore, n));
            mNos[static_cast<size_t>(n)] = ficha;
            ficha = nullptr;
        } else {
            // O a E O b seguiram caindo como letras: o no deles nasce na linha.
            const auto nascidos = Compilar(kGatilhoDoNo, Vector2(mNaLinha[k].x, mNaLinha[k].y), 90.0f, CaraDoNo(mArvore, n));
            mNos[static_cast<size_t>(n)] = nascidos.empty() ? nullptr : nascidos.front();
        }

        if (no.tipo == Tipo::Op) Compilar(kGatilhoDaReducao, lugar(n), std::nullopt, CaraDoLexema(no.texto));
    }

    // A GERACAO DE CODIGO: a arvore percorrida de novo, e cada no some virando
    // a instrucao dele, mirada no aluno. O alvo do = nao gera nada e espera o
    // =; o = sai no anel do STORE.
    while (mEmitidos < nos && t >= mMarcos.codigo + static_cast<float>(mEmitidos) * mDegrauDoCompilador.porInstrucao) {
        const int n = mOrdem[static_cast<size_t>(mEmitidos++)];
        const std::string instrucao = Instrucao(mArvore, n);
        if (instrucao.empty()) continue;
        const Vector2 onde = mNos[static_cast<size_t>(n)] ? mNos[static_cast<size_t>(n)]->GetPosition() : lugar(n);
        for (const int k : {n, n == mArvore.raiz ? mArvore.nos[static_cast<size_t>(n)].esq : -1}) {
            if (k < 0) continue;
            if (BossProjectile*& p = mNos[static_cast<size_t>(k)]) { p->Kill(); p = nullptr; }
        }
        Compilar(n == mArvore.raiz ? kGatilhoDoStore : kGatilhoDaInstrucao, onde, std::nullopt, CaraDaInstrucao(instrucao));
        RegistrarAnuncio(kFaseDoCompilador, Placar(mLinha, Etapa::Codigo, instrucao));
    }

    // A ETAPA MUDOU: a legenda diz qual e, e o placar tambem (na geracao de
    // codigo, o placar e a instrucao que acabou de sair).
    const Etapa etapa = EtapaEm(t, mMarcos);
    if (etapa != mEtapa) {
        mEtapa = etapa;
        if (etapa != Etapa::Codigo) RegistrarAnuncio(kFaseDoCompilador, Placar(mLinha, etapa));
        RegistrarLegenda(kFaseDoCompilador, Legenda(etapa));
    }

    // O PROFESSOR VARRE A LINHA no lexico, em cima do token que le, e depois
    // fica em cima do meio da arvore que monta - a arvore e centrada, e a raiz
    // fica sempre a esquerda, embaixo da legenda. Pulando de no em no, como
    // era, ele andava a 450 px/s a rodada inteira, e o tiro do aluno - que leva
    // quase 1 s ate ele - nao o achava mais: o piloto que persegue terminava a
    // fase com a nota menor do que comecou.
    float x = mScene && mScene->GetGame() ? static_cast<float>(mScene->GetGame()->GetWindowWidth()) / 2.0f
                                          : GetPosition().x;

    if (mLidos < static_cast<int>(mTokens.size())) x = mNaLinha[static_cast<size_t>(mLidos)].x;
    mAlvo = Vector2(x, kYDaLinha);


    // CADA NO ANDA ATE O LUGAR DELE, e as arestas o seguem.
    for (int n = 0; n < static_cast<int>(mNos.size()); ++n) {
        BossProjectile* no = mNos[static_cast<size_t>(n)];
        if (!no) continue;
        no->SetPosition(Ancora::Aproximar(no->GetPosition(), lugar(n), kRapidezDoNo, deltaTime));
        if (auto* rb = no->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    DesenharArestas();
}

void Vladimir::DesenharArestas() {
    if (!mArestas) return;
    mArestas->Limpar();
    for (size_t n = 0; n < mNos.size() && n < mArvore.nos.size(); ++n) {
        if (!mNos[n]) continue;
        const Compilador::No& no = mArvore.nos[n];
        for (const int f : {no.esq, no.dir}) {
            if (f < 0 || !mNos[static_cast<size_t>(f)]) continue;
            mArestas->Acrescentar(mNos[n]->GetPosition(), mNos[static_cast<size_t>(f)]->GetPosition(), kAresta);
        }
    }
}

void Vladimir::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O QUE O COMPILADOR SOLTOU: a cara de cada um, e quem nasceu. O no da
    // arvore e o mapa, como os estados da fase 1: ninguem se machuca nele.
    if (mCompilando) {
        for (auto& p : projectiles) {
            auto* nascido = dynamic_cast<BossProjectile*>(p.get());
            if (!nascido) continue;
            if (nascido->Posto() == Compilador::kPostoDaArvore) nascido->DefinirEnfeite(true);
            if (auto* anim = nascido->GetComponent<DrawAnimatedComponent>(); anim && !mCaraDoCompilado.empty()) {
                anim->SetAnimation(mCaraDoCompilado);
            }
            mCompilados.push_back(nascido);
        }
        return;
    }
    if (!mFormando) return;

    // O DIAGRAMA E OS ESTADOS SAO O MAPA, e nao tiro: ninguem se machuca neles,
    // e o tiro do aluno os atravessa.
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

    // NO COMPILADOR, tudo sai de onde o compilador manda, no rumo dele - com
    // um tiro no meio do leque, como a letra lida abaixo.
    if (stateName == Compilador::kFaseDoCompilador && mCompilando) {
        const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
        params.firePosition = mOndeCompilar;
        params.centralAngle = mRumoDoCompilado + (par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f);
        return;
    }

    if (stateName != Automato::kFaseDoAutomato && stateName != Parada::kFaseDaParada) return;

    // O MAPA NASCE ONDE FICA.
    if (mFormando) {
        params.firePosition = mDisparoDoFormando;
        return;
    }

    // A LETRA LIDA SAI MIRADA NO ALUNO, com um tiro no meio do leque: num leque
    // par o meio cai entre dois tiros, e quem fica parado ficaria na fresta
    // (ver Ricardo::CustomizeAttackParams). O anel da aceitacao ignora o angulo.
    // Vale igual para os passos do programa da repescagem.
    const Vector2 d = GetDirectionToPlayer();
    const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
    const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f;
    params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
}
