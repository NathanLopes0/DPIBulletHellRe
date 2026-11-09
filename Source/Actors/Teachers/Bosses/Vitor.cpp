//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#include "Vitor.h"

#include "../../../Attacks/AttackParameters/AttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"
#include "PainelDeCodigo.h"
#include "../../../Attacks/Coercao.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Motor/Random.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

    bool Vivo(const BossProjectile* p) { return p && p->GetState() == ActorState::Active; }

    std::uint32_t Sorteio() {
        return static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    }

    void PorCara(BossProjectile* p, const std::string& cara) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    void Parar(BossProjectile* p) {
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }

    void Lancar(BossProjectile* p, const float graus, const float velocidade) {
        const float a = Math::ToRadians(graus);
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2(std::cos(a), std::sin(a)) * velocidade);
        }
    }

    // A fase 1: as cores das threads, as mesmas de Ferramentas/gerar_vitor.py (CORES_DAS_THREADS):
    // o nome da coluna e o chip da instrucao se casam.
    constexpr SDL_Color kCorDaThread[Semaforo::kMaisThreads] = {
        {240, 160, 60, 255}, {90, 200, 235, 255}, {235, 110, 190, 255},
        {160, 220, 90, 255}, {170, 140, 245, 255}};
    constexpr SDL_Color kFundoDoNome{24, 28, 40, 225};
    constexpr SDL_Color kDentro{255, 214, 72, 255};        // na secao critica
    constexpr SDL_Color kFundoDeDentro{70, 62, 24, 235};
    constexpr SDL_Color kPassou{120, 124, 136, 200};        // ja fez V(s): acabou
    constexpr SDL_Color kMolduraDaSecao{255, 214, 72, 200};
    constexpr SDL_Color kFundoDaSecao{70, 62, 24, 90};
    constexpr SDL_Color kMolduraDoCodigo{200, 176, 96, 230};
    constexpr SDL_Color kFundoDoCodigo{40, 36, 20, 225};
    constexpr SDL_Color kTintaDoCodigo{255, 240, 190, 255};

    SDL_Color ComAlfa(SDL_Color c, const Uint8 a) { c.a = a; return c; }

    // A fase 2: o roteador e o grafico do cwnd.
    constexpr SDL_Color kMolduraDoRoteador{90, 140, 220, 240};
    constexpr SDL_Color kFundoDoRoteador{18, 28, 52, 230};
    constexpr SDL_Color kTintaDoRoteador{200, 222, 255, 255};
    constexpr SDL_Color kMolduraDoGrafico{100, 120, 165, 190};
    constexpr SDL_Color kFundoDoGrafico{14, 18, 30, 170};
    constexpr SDL_Color kEixoDoGrafico{120, 130, 160, 200};
    constexpr SDL_Color kCorDaCapacidade{230, 92, 92, 220};
    constexpr SDL_Color kCorDaSerra{250, 190, 70, 255};
    constexpr int kAlturaDaCaixaDoRoteador = 30;

    // A final: a mesa, os filosofos conforme o que fazem, e as esperas.
    constexpr SDL_Color kCorDaMesa{186, 130, 80, 230};
    constexpr SDL_Color kPensando{150, 170, 210, 255};
    constexpr SDL_Color kComendo{96, 222, 112, 255};
    constexpr SDL_Color kFundoDeQuemCome{20, 60, 36, 235};
    constexpr SDL_Color kEsperando{235, 96, 96, 255};
    constexpr SDL_Color kFundoDeQuemEspera{70, 24, 28, 235};
    constexpr SDL_Color kMorto{110, 110, 118, 220};
    constexpr SDL_Color kDeFora{120, 126, 140, 160};
    constexpr SDL_Color kLinhaDaEspera{235, 96, 96, 230};

    // A fase 3: o canal e os nomes dos dois processos.
    constexpr SDL_Color kCorDoCanal{120, 200, 230, 110};
    constexpr SDL_Color kMolduraDoProcesso{110, 230, 200, 230};
    constexpr SDL_Color kTintaDoProcesso{200, 255, 240, 255};

    /// O nome da coluna e o codigo da faixa: a mesma conta de largura do
    /// canto do Luizinho (9,5 px por letra mais a folga).
    int LarguraDoTexto(const std::string& texto) {
        return static_cast<int>(static_cast<float>(texto.size()) * 9.5f + 18.0f);
    }
    constexpr int kAlturaDoNome = 26;
}

Vitor::Vitor(Scene* scene) : Boss(scene) {
}

std::optional<Vector2> Vitor::AlvoDoMovimento() const {
    return mAlvo;
}

void Vitor::AoTerminarABatalha() {
    // A tela da nota fica limpa: os projeteis a batalha ja levou.
    if (mSecao) mSecao->Mostrar(false);
    if (mCodigoDaSecao) mCodigoDaSecao->Mostrar(false);
    for (const Thread& th : mThreads) if (th.nome) th.nome->Mostrar(false);
    if (mFilaDoRoteador) mFilaDoRoteador->Mostrar(false);
    if (mQuadroDoGrafico) mQuadroDoGrafico->Mostrar(false);
    if (mNomeDoGrafico) mNomeDoGrafico->Mostrar(false);
    for (EtiquetaDeCodigo* nome : mNomesDosFilosofos) if (nome) nome->Mostrar(false);
    for (EtiquetaDeCodigo* nome : mNomesDosProcessos) if (nome) nome->Mostrar(false);
    if (mLinhas) mLinhas->Limpar();
}

std::vector<BossProjectile*> Vitor::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                              const std::optional<float> centro,
                                              const std::optional<float> abertura,
                                              const float rapidez) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mCentroEm = centro;
    mAberturaEm = abertura;
    mCriados.clear();
    Disparar(gatilho, -1, rapidez);
    mDisparandoEm = false;
    mQuantos = 0;
    mCentroEm.reset();
    mAberturaEm.reset();
    return mCriados;
}

void Vitor::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

void Vitor::AcompanharOAluno(const float deltaTime) {
    if (deltaTime <= 0.0f) return;
    const Vector2 agora = PosicaoDoAluno();
    if (mViuOAluno) {
        // A media que esquece: cada quadro pesa deltaTime / kMemoriaDoAndar.
        const Vector2 doQuadro = (agora - mUltimaDoAluno) * (1.0f / deltaTime);
        const float peso = std::min(1.0f, deltaTime / kMemoriaDoAndar);
        mVelocidadeDoAluno = mVelocidadeDoAluno + (doQuadro - mVelocidadeDoAluno) * peso;
    }
    mUltimaDoAluno = agora;
    mViuOAluno = true;
}

// A fase 1: os semaforos.

void Vitor::MontarSemaforos() {
    mNosSemaforos = true;
    mRodadaDoSemaforo = -1;
    mAlunoPreso = false;
    const SDL_FRect campo = Campo();

    // O que esta luta sorteia para a fase, e o degrau do modo.
    Variacao::Sorteio sorteio = SorteioDaFase(Semaforo::kFaseDosSemaforos);
    mPlanoDosSemaforos = Semaforo::Sortear(sorteio);
    mDegrauDosSemaforos = Semaforo::DegrauDo(NivelDeDificuldade());

    // A secao critica: a faixa de borda a borda, com o codigo dela no canto.
    auto secao = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(campo.w - 40.0f),
                                                  static_cast<int>(Semaforo::kAlturaDaSecao), kOrdemDaSecao);
    secao->SetPosition(Vector2(campo.x + campo.w / 2.0f,
                               campo.y + Semaforo::kTopoDaSecao + Semaforo::kAlturaDaSecao / 2.0f));
    secao->Pintar(kMolduraDaSecao, kFundoDaSecao);
    mSecao = secao.get();
    mScene->AddActor(std::move(secao));

    // O canto cabe o codigo mais comprido das quatro rodadas.
    int largura = 0;
    for (int k = 0; k < Semaforo::kRodadas; ++k) {
        largura = std::max(largura, LarguraDoTexto(Semaforo::CodigoDaSecao(Semaforo::RodadaDaVez(k).tipo)));
    }
    auto codigo = std::make_unique<EtiquetaDeCodigo>(mScene, Semaforo::CodigoDaSecao(Semaforo::Tipo::Mutex),
                                                     largura, kAlturaDoNome, kOrdemDosNomes);
    codigo->SetPosition(Vector2(campo.x + 30.0f + static_cast<float>(largura) / 2.0f,
                                campo.y + Semaforo::kTopoDaSecao + Semaforo::kAlturaDaSecao / 2.0f));
    codigo->Pintar(kMolduraDoCodigo, kFundoDoCodigo, kTintaDoCodigo);
    mCodigoDaSecao = codigo.get();
    mScene->AddActor(std::move(codigo));

    mAlvo = Vector2(campo.x + campo.w / 2.0f + kAoLadoDaColuna, campo.y + kAlturaDoProfessor);
}

void Vitor::SoltarAsPilhas(const bool matando) {
    for (Thread& th : mThreads) {
        for (BossProjectile* p : th.paradas) {
            if (!Vivo(p)) continue;
            if (matando) p->Kill();
            else Lancar(p, 90.0f, Semaforo::kVelocidadeDaRajada * mDegrauDosSemaforos.rapidez);
        }
        th.paradas.clear();
    }
}

void Vitor::DesmontarSemaforos() {
    // A faixa some, e com ela a porta: o que estava parado nela some junto.
    SoltarAsPilhas(true);
    for (Thread& th : mThreads) {
        if (th.nome) th.nome->SetState(ActorState::Destroy);
        th.nome = nullptr;
    }
    mThreads.clear();
    mPassando.clear();
    if (mSecao) mSecao->SetState(ActorState::Destroy);
    if (mCodigoDaSecao) mCodigoDaSecao->SetState(ActorState::Destroy);
    mSecao = nullptr;
    mCodigoDaSecao = nullptr;
    RestaurarAnuncio(Semaforo::kFaseDosSemaforos);
    mNosSemaforos = false;
    mRodadaDoSemaforo = -1;
    mAlvo.reset();
}

void Vitor::ComecarRodadaDoSemaforo(const int k) {
    // O que sobrou da rodada anterior passa pela porta.
    SoltarAsPilhas(false);
    for (Thread& th : mThreads) {
        for (const Instrucao& in : th.descendo) mPassando.push_back(in);
        if (th.nome) th.nome->SetState(ActorState::Destroy);
    }
    mThreads.clear();

    mRodadaDoSemaforo = k;
    mInicioDaRodada = Semaforo::kPrimeiro + static_cast<float>(k) * Semaforo::kPorRodada;

    const SDL_FRect campo = Campo();
    const Vector2 aluno = PosicaoDoAluno();
    mColunas = Semaforo::Colunas(aluno.x - campo.x, campo.w, mPlanoDosSemaforos.entreThreads);
    for (float& x : mColunas) x += campo.x;
    const int n = static_cast<int>(mColunas.size());
    mEscalonador = Semaforo::Comecar(mPlanoDosSemaforos.rodadas[static_cast<size_t>(k)], n,
                                     Semaforo::MaisPerto(mColunas, aluno.x), Sorteio());

    mThreads.resize(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        Thread& th = mThreads[static_cast<size_t>(i)];
        th.proximaEm = Semaforo::kComecaAMandar;
        const std::string nome = Semaforo::Nome(i);
        auto etiqueta = std::make_unique<EtiquetaDeCodigo>(mScene, nome, LarguraDoTexto(nome), kAlturaDoNome, kOrdemDosNomes);
        etiqueta->SetPosition(Vector2(mColunas[static_cast<size_t>(i)], campo.y + Semaforo::kAlturaDosNomes));
        th.nome = etiqueta.get();
        mScene->AddActor(std::move(etiqueta));
    }
    if (mCodigoDaSecao) mCodigoDaSecao->Escrever(Semaforo::CodigoDaSecao(mEscalonador.rodada.tipo));
}

void Vitor::Mandar(const int thread) {
    const SDL_FRect campo = Campo();
    const auto criadas = DispararEm(Semaforo::kGatilhoDaInstrucao,
                                    Vector2(mColunas[static_cast<size_t>(thread)], campo.y + Semaforo::kNasce), 1,
                                    std::nullopt, std::nullopt, mDegrauDosSemaforos.rapidez);
    for (BossProjectile* p : criadas) {
        PorCara(p, Semaforo::Nome(thread));
        mThreads[static_cast<size_t>(thread)].descendo.push_back({p, false});
    }
}

void Vitor::Acordar(const int thread) {
    // V(s) de quem saiu, p(s) desta: a pilha inteira desce de uma vez.
    Thread& th = mThreads[static_cast<size_t>(thread)];
    const bool giro = mEscalonador.rodada.tipo == Semaforo::Tipo::EsperaOcupada;
    const Player* aluno = Aluno();
    const bool preso = aluno && aluno->EstaAcorrentado();
    const auto* fsm = GetComponent<FSMComponent>();
    const float t = fsm ? fsm->GetStateTime() : 0.0f;

    for (size_t j = 0; j < th.paradas.size(); ++j) {
        BossProjectile* p = th.paradas[j];
        if (!Vivo(p)) continue;
        if (giro) {
            const Semaforo::Ponto d = Semaforo::NoGiro(static_cast<int>(j), Semaforo::kMaiorPilha, t);
            Lancar(p, Math::ToDegrees(Math::Atan2(d.y, d.x)), Semaforo::kVelocidadeDoAnel * mDegrauDosSemaforos.rapidez);
        } else {
            Lancar(p, Semaforo::AnguloNaRajada(static_cast<int>(j), mDegrauDosSemaforos.aberturaDaRajada),
                   Semaforo::kVelocidadeDaRajada * mDegrauDosSemaforos.rapidez);
        }
        // Preso, o aluno deixa de estar seguro ao lado da coluna.
        if (preso) p->insertMotion<TrackingBehavior>(kAtrasoDaPerseguicao, kForcaDaPerseguicao, kDuracaoDaPerseguicao);
    }
    th.paradas.clear();
}

void Vitor::MoverInstrucoes(const float t) {
    const SDL_FRect campo = Campo();
    const Semaforo::Tipo tipo = mEscalonador.rodada.tipo;
    const bool giro = tipo == Semaforo::Tipo::EsperaOcupada;
    const bool corrida = tipo == Semaforo::Tipo::Corrida;

    for (int i = 0; i < static_cast<int>(mThreads.size()); ++i) {
        Thread& th = mThreads[static_cast<size_t>(i)];
        const bool livre = corrida || Semaforo::Dentro(mEscalonador, i) || Semaforo::Passou(mEscalonador, i);
        if (livre) {
            for (const Instrucao& in : th.descendo) mPassando.push_back(in);
            th.descendo.clear();
            continue;
        }

        // Bloqueada: quem chega a porta para.
        const float x = mColunas[static_cast<size_t>(i)];
        for (auto it = th.descendo.begin(); it != th.descendo.end();) {
            BossProjectile* p = it->p;
            const float limite = campo.y + (giro ? Semaforo::kCentroDoGiro - Semaforo::kRaioDoGiro
                                                 : Semaforo::AlturaNaPilha(static_cast<int>(th.paradas.size())));
            if (p->GetPosition().y < limite) { ++it; continue; }
            Parar(p);
            if (!giro) p->SetPosition(Vector2(x, limite));
            th.paradas.push_back(p);
            it = th.descendo.erase(it);
        }
        if (giro) {
            for (size_t j = 0; j < th.paradas.size(); ++j) {
                const Semaforo::Ponto d = Semaforo::NoGiro(static_cast<int>(j), Semaforo::kMaiorPilha, t);
                th.paradas[j]->SetPosition(Vector2(x + d.x, campo.y + Semaforo::kCentroDoGiro + d.y));
                Parar(th.paradas[j]);
            }
        }
    }

    // Quem ja passou do meio da faixa sai da lista.
    const float meio = campo.y + Semaforo::kTopoDaSecao + Semaforo::kAlturaDaSecao / 2.0f;
    for (Instrucao& in : mPassando) {
        if (in.desviou || in.p->GetPosition().y < meio) continue;
        in.desviou = true;
        if (corrida) {
            Lancar(in.p, 90.0f + Semaforo::DesvioNaCorrida(Sorteio()),
                   Semaforo::kVelocidadeDaInstrucao * mDegrauDosSemaforos.rapidez);
        }
    }
    mPassando.erase(std::remove_if(mPassando.begin(), mPassando.end(),
                                   [](const Instrucao& in) { return in.desviou; }),
                    mPassando.end());
}

void Vitor::PintarThreads() {
    for (int i = 0; i < static_cast<int>(mThreads.size()); ++i) {
        EtiquetaDeCodigo* nome = mThreads[static_cast<size_t>(i)].nome;
        if (!nome) continue;
        const SDL_Color cor = kCorDaThread[i % Semaforo::kMaisThreads];
        if (Semaforo::Dentro(mEscalonador, i)) nome->Pintar(kDentro, kFundoDeDentro, kDentro);
        else if (Semaforo::Passou(mEscalonador, i)) nome->Pintar(kPassou, kFundoDoNome, kPassou);
        else nome->Pintar(cor, kFundoDoNome, cor);
    }
}

void Vitor::DesenharSemaforos() {
    if (!mLinhas) return;
    const SDL_FRect campo = Campo();
    const float de = campo.y + Semaforo::kAlturaDosNomes + static_cast<float>(kAlturaDoNome) / 2.0f;
    const float ate = campo.y + Semaforo::kTopoDaSecao;
    for (int i = 0; i < static_cast<int>(mThreads.size()); ++i) {
        const float x = mColunas[static_cast<size_t>(i)];
        SDL_Color cor = ComAlfa(kCorDaThread[i % Semaforo::kMaisThreads], 110);
        if (Semaforo::Dentro(mEscalonador, i)) cor = kDentro;
        else if (Semaforo::Passou(mEscalonador, i)) cor = ComAlfa(kPassou, 90);
        mLinhas->Acrescentar(Vector2(x, de), Vector2(x, ate), cor);
    }
}

void Vitor::AtualizarSemaforos() {
    for (Thread& th : mThreads) {
        th.descendo.erase(std::remove_if(th.descendo.begin(), th.descendo.end(),
                                         [](const Instrucao& in) { return !Vivo(in.p); }), th.descendo.end());
        th.paradas.erase(std::remove_if(th.paradas.begin(), th.paradas.end(),
                                        [](const BossProjectile* p) { return !Vivo(p); }), th.paradas.end());
    }
    mPassando.erase(std::remove_if(mPassando.begin(), mPassando.end(),
                                   [](const Instrucao& in) { return !Vivo(in.p); }), mPassando.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Semaforo::kFaseDosSemaforos && !EhClone();
    if (!aqui) {
        if (mNosSemaforos) DesmontarSemaforos();
        return;
    }
    if (!mNosSemaforos) MontarSemaforos();

    const float t = fsm->GetStateTime();
    if (t < Semaforo::kPrimeiro) return;
    const int k = std::min(Semaforo::kRodadas - 1,
                           static_cast<int>((t - Semaforo::kPrimeiro) / Semaforo::kPorRodada));
    if (k != mRodadaDoSemaforo) ComecarRodadaDoSemaforo(k);
    const float u = t - mInicioDaRodada;

    // A corrente: o aluno preso esta bloqueado em P(s), e a coluna em cima dele fura a fila.
    const Player* aluno = Aluno();
    const bool preso = aluno && aluno->EstaAcorrentado();
    if (preso && !mAlunoPreso) {
        const int dele = Semaforo::MaisPerto(mColunas, aluno->GetPosition().x);
        if (dele >= 0) Semaforo::Furar(mEscalonador, dele);
    }
    mAlunoPreso = preso;

    for (const Semaforo::Evento& ev : Semaforo::Avancar(mEscalonador, u)) {
        if (ev.entra) Acordar(ev.thread);
    }

    // As threads mandam instrucoes.
    const bool corrida = mEscalonador.rodada.tipo == Semaforo::Tipo::Corrida;
    for (int i = 0; i < static_cast<int>(mThreads.size()); ++i) {
        Thread& th = mThreads[static_cast<size_t>(i)];
        if (th.mandadas >= mDegrauDosSemaforos.instrucoesPorThread || u < th.proximaEm) continue;
        const bool livre = corrida || Semaforo::Dentro(mEscalonador, i) || Semaforo::Passou(mEscalonador, i);
        const int presas = static_cast<int>(th.descendo.size() + th.paradas.size());
        if (!livre && presas >= Semaforo::kMaiorPilha) continue;
        Mandar(i);
        ++th.mandadas;
        th.proximaEm = u + mDegrauDosSemaforos.entreInstrucoes;
    }

    MoverInstrucoes(t);

    // O professor fica ao lado da coluna que ele vai acordar.
    const SDL_FRect campo = Campo();
    int perto = Semaforo::Proxima(mEscalonador);
    if (perto < 0 && !corrida && !mEscalonador.dentro.empty()) perto = mEscalonador.dentro.back();
    if (perto < 0 && !mColunas.empty()) perto = static_cast<int>(mColunas.size()) / 2;
    if (perto >= 0) {
        const float c = mColunas[static_cast<size_t>(perto)];
        const float lado = mPlanoDosSemaforos.entreThreads / 2.0f;
        const float x = c + (c < campo.x + campo.w / 2.0f ? lado : -lado);
        mAlvo = Vector2(x, campo.y + kAlturaDoProfessor);
    }

    PintarThreads();
    DesenharSemaforos();
    TrocarAnuncio(Semaforo::kFaseDosSemaforos, Semaforo::Placar(mEscalonador, preso));
}

void Vitor::OnUpdate(const float deltaTime) {

    // No fork o corpo nao anda sozinho nem ataca.
    if (mTempoNoFork >= 0.0f) {
        mTempoNoFork += deltaTime;
        if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        if (auto* d = GetComponent<DrawAnimatedComponent>()) {
            d->SetIsVisible(Nuvem::Visivel(kFork, mTempoNoFork, mNasceuDentro));
        }
        SetPosition(mSaiDe + (mVaiPara - mSaiDe) * Nuvem::Saida(kFork, mTempoNoFork));
        if (!mNasceuDentro) AtualizarANuvemDoFork();
        if (mTempoNoFork >= kFork.sai) SairDaNuvem();
        return;
    }

    Boss::OnUpdate(deltaTime);

    if (!mLinhas && mScene && !EhClone()) {
        auto ator = std::make_unique<Actor>(mScene);
        mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
        mScene->AddActor(std::move(ator));
    }
    if (mLinhas) mLinhas->Limpar();

    if (!EhClone()) AcompanharOAluno(deltaTime);
    AtualizarNuvens(deltaTime);
    AtualizarSemaforos();
    AtualizarMensagens();
    AtualizarJantar(deltaTime);
}

void Vitor::EntrarNaNuvem(const Vector2 onde, const Vector2 destino, const bool nasceuDentro) {
    SetPosition(onde);
    mSaiDe = onde;
    mVaiPara = destino;
    mNasceuDentro = nasceuDentro;
    mTempoNoFork = 0.0f;
    if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    if (auto* d = GetComponent<DrawAnimatedComponent>()) d->SetIsVisible(Nuvem::Visivel(kFork, 0.0f, nasceuDentro));
}

void Vitor::SairDaNuvem() {
    mTempoNoFork = -1.0f;
    SetPosition(mVaiPara);
    if (auto* d = GetComponent<DrawAnimatedComponent>()) d->SetIsVisible(true);
    if (EhClone()) return;

    // A nuvem se desfaz, e o placar volta ao nome da fase: sairam dois.
    for (BossProjectile* nuvem : Nuvens()) nuvem->Kill();
    if (!mNomeDaFaseDoFork.empty()) RegistrarAnuncio(kFaseDoFork, mNomeDaFaseDoFork);
}

void Vitor::ComecarOFork() {
    auto* batalha = Batalha();
    const auto nuvens = Nuvens();
    if (!batalha || nuvens.empty()) return;   // sem nuvem, o fork de sempre

    // As duas nuvens se juntam em cima dele (AtualizarANuvemDoFork as faz crescer e processar).
    mEscalaDaNuvem = nuvens.front()->GetScale();
    for (BossProjectile* nuvem : nuvens) {
        // Elas vivem a fase 2 inteira, e o fork e na 3.
        nuvem->RenovarTempoDeVida();
        nuvem->insertMotion<AncorarBehavior>(Vector2::Zero, kFork.sai + 0.5f, 200.0f);
    }

    // Ele entra quando a nuvem chega, e fica onde esta.
    const Vector2 aqui = GetPosition();
    EntrarNaNuvem(aqui, aqui, false);
    for (Boss* corpo : batalha->Corpos()) {
        if (corpo == this) continue;
        if (auto* outro = dynamic_cast<Vitor*>(corpo)) outro->EntrarNaNuvem(aqui, outro->GetPosition(), true);
    }

    if (mNomeDaFaseDoFork.empty()) mNomeDaFaseDoFork = AnuncioDaFase(kFaseDoFork);
    RegistrarAnuncio(kFaseDoFork, kNoFork);
}

void Vitor::AtualizarANuvemDoFork() {
    // A nuvem cresce enquanto as duas se juntam, e cobre o professor quando ele entra.
    const float u = kFork.entra > 0.0f ? std::min(1.0f, mTempoNoFork / kFork.entra) : 1.0f;
    const float escala = mEscalaDaNuvem + (kEscalaDaNuvemJunta - mEscalaDaNuvem) * u;
    const char* cara = mTempoNoFork < kFork.abre ? Nuvem::kCaraProcessando : Nuvem::kCaraOciosa;
    for (BossProjectile* nuvem : Nuvens()) {
        nuvem->SetScale(escala);
        if (auto* anim = nuvem->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }
}

std::vector<BossProjectile*> Vitor::Nuvens() const {
    std::vector<BossProjectile*> nuvens;
    const auto* batalha = Batalha();
    if (!batalha || !batalha->GerenteDeProjeteis()) return nuvens;
    for (const auto& p : batalha->GerenteDeProjeteis()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == Nuvem::kPostoDasNuvens) {
            nuvens.push_back(p.get());
        }
    }
    return nuvens;
}

void Vitor::DispararNuvem(const char* gatilho) {
    Disparar(gatilho);
}

float Vitor::Angulo(const Vector2 de, const Vector2 para) {
    const Vector2 d = para - de;
    return Math::ToDegrees(Math::Atan2(d.y, d.x));
}

float Vitor::AnguloAdiante(const Vector2 de, const float rapidez, const float quanto) const {
    const Vector2 aluno = PosicaoDoAluno();
    const Tcp::Ponto alvo = Tcp::Mira({de.x, de.y}, {aluno.x, aluno.y},
                                      {mVelocidadeDoAluno.x, mVelocidadeDoAluno.y}, rapidez, quanto);
    return Angulo(de, Vector2(alvo.x, alvo.y));
}

Vector2 Vitor::PosicaoDoRoteador() const {
    const SDL_FRect campo = Campo();
    const Tcp::Ponto r = Tcp::Roteador(campo.w, mPlanoDaRede.fracaoDoRoteador);
    return Vector2(campo.x + r.x, campo.y + r.y);
}

void Vitor::MontarRede() {
    const SDL_FRect campo = Campo();

    // O roteador: a caixa no meio do campo, com a fila dele escrita.
    const std::string fila = Tcp::Fila(mPlanoDaRede.capacidade, mPlanoDaRede.capacidade);
    auto roteador = std::make_unique<EtiquetaDeCodigo>(mScene, fila, LarguraDoTexto(fila), kAlturaDaCaixaDoRoteador,
                                                       kOrdemDosNomes);
    roteador->SetPosition(PosicaoDoRoteador());
    roteador->Pintar(kMolduraDoRoteador, kFundoDoRoteador, kTintaDoRoteador);
    roteador->Escrever(Tcp::Fila(0, mPlanoDaRede.capacidade));
    mFilaDoRoteador = roteador.get();
    mScene->AddActor(std::move(roteador));

    // O grafico do cwnd, no canto de cima a direita.
    const Tcp::Retangulo g = Tcp::Grafico(campo.w);
    auto quadro = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(g.w), static_cast<int>(g.h), kOrdemDaSecao);
    quadro->SetPosition(Vector2(campo.x + g.x + g.w / 2.0f, campo.y + g.y + g.h / 2.0f));
    quadro->Pintar(kMolduraDoGrafico, kFundoDoGrafico);
    mQuadroDoGrafico = quadro.get();
    mScene->AddActor(std::move(quadro));

    const std::string nome = "cwnd";
    auto etiqueta = std::make_unique<EtiquetaDeCodigo>(mScene, nome, LarguraDoTexto(nome), kAlturaDoNome, kOrdemDosNomes);
    etiqueta->SetPosition(Vector2(campo.x + g.x + 8.0f + static_cast<float>(LarguraDoTexto(nome)) / 2.0f,
                                  campo.y + g.y + 4.0f + static_cast<float>(kAlturaDoNome) / 2.0f));
    etiqueta->Pintar(kMolduraDoGrafico, kFundoDoNome, kCorDaSerra);
    mNomeDoGrafico = etiqueta.get();
    mScene->AddActor(std::move(etiqueta));
}

void Vitor::DesmontarRede() {
    for (Actor* a : {static_cast<Actor*>(mFilaDoRoteador), static_cast<Actor*>(mQuadroDoGrafico),
                     static_cast<Actor*>(mNomeDoGrafico)}) {
        if (a) a->SetState(ActorState::Destroy);
    }
    mFilaDoRoteador = nullptr;
    mQuadroDoGrafico = nullptr;
    mNomeDoGrafico = nullptr;
    mEmVoo.clear();
    mFila.clear();
    mMandando = false;
    mJanelasDoGrafico.clear();
}

void Vitor::MandarSegmentos(const float agora) {
    // A janela sai aos poucos, um segmento a cada kEntreSegmentos, alternando as nuvens.
    const auto nuvens = Nuvens();
    if (nuvens.empty()) {
        mMandando = false;
        return;
    }
    const Vector2 roteador = PosicaoDoRoteador();
    while (mMandados < mJanela.Total()
           && agora >= mInicioDaJanela + static_cast<float>(mMandados) * Tcp::kEntreSegmentos) {
        const Vector2 de = nuvens[static_cast<size_t>(mMandados) % nuvens.size()]->GetPosition();
        for (BossProjectile* p : DispararEm(Tcp::kGatilhoDoSegmento, de, 1, Angulo(de, roteador))) {
            PorCara(p, mJanela.Retransmitido(mMandados) ? Tcp::kCaraDoRetransmitido : Tcp::kCaraDoNovo);
            mEmVoo.push_back({p, mJanela.Perdido(mMandados), mJanela.Retransmitido(mMandados)});
        }
        ++mMandados;
    }
    if (mMandados >= mJanela.Total()) mMandando = false;
}

void Vitor::ChegarNoRoteador(const float agora) {
    const Vector2 r = PosicaoDoRoteador();
    for (auto it = mEmVoo.begin(); it != mEmVoo.end();) {
        BossProjectile* p = it->p;
        const Vector2 falta = r - p->GetPosition();
        Vector2 v = Vector2::Zero;
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) v = rb->GetVelocity();
        const bool chegou = falta.Length() < Tcp::kChegou || Vector2::Dot(falta, v) < 0.0f;
        if (!chegou) { ++it; continue; }

        if (it->perdido) {
            // O roteador estava cheio: o segmento e descartado ali mesmo, e estoura num anel de cacos.
            const int n = mDegrauDaRede.anelDoDescarte;
            const float centro = Coercao::CentroDoAnel(n, Angulo(r, PosicaoDoAluno()) + Tcp::GiroDoDescarte(mDescartes++, n));
            for (BossProjectile* caco : DispararEm(Tcp::kGatilhoDoDescarte, r, n, centro, Coercao::AberturaDoAnel(n),
                                                   mDegrauDaRede.rapidez)) {
                PorCara(caco, Tcp::kCaraDoDescartado);
            }
        } else {
            if (mFila.empty()) mProximoEncaminhado = std::max(mProximoEncaminhado, agora);
            mFila.push_back(it->retransmitido);
            ++mRecebidosNaJanela;
        }
        p->Kill();
        it = mEmVoo.erase(it);
    }
}

void Vitor::Encaminhar(const float agora) {
    // O enlace de saida: um a cada kEntreEncaminhados, mirado adiante do aluno.
    if (mFila.empty() || agora < mProximoEncaminhado) return;
    const bool retransmitido = mFila.front();
    mFila.erase(mFila.begin());

    const Vector2 r = PosicaoDoRoteador();
    const Vector2 aluno = PosicaoDoAluno();
    const Vector2 andando = mVelocidadeDoAluno;
    const Tcp::Ponto alvo = Tcp::Mira({r.x, r.y}, {aluno.x, aluno.y}, {andando.x, andando.y},
                                      Tcp::kVelocidadeDoEncaminhado * mDegrauDaRede.rapidez,
                                      Tcp::Antecipacao(mEncaminhados++, retransmitido));
    for (BossProjectile* p : DispararEm(Tcp::kGatilhoDoEncaminhado, r, 1, Angulo(r, Vector2(alvo.x, alvo.y)),
                                        std::nullopt, mDegrauDaRede.rapidez)) {
        PorCara(p, retransmitido ? Tcp::kCaraDoRetransmitido : Tcp::kCaraDoNovo);
    }
    mProximoEncaminhado = agora + mDegrauDaRede.entreEncaminhados;
}

void Vitor::DesenharGrafico() {
    if (!mLinhas || !mQuadroDoGrafico) return;
    const float w = Campo().w;
    const float x0 = Campo().x;
    const float y0 = Campo().y;
    auto ponto = [w, x0, y0](const int rtt, const int cwnd) {
        const Tcp::Ponto p = Tcp::NoGrafico(rtt, cwnd, w);
        return Vector2(x0 + p.x, y0 + p.y);
    };
    // O eixo e a capacidade do roteador: o que passa da linha vermelha se perde.
    mLinhas->Acrescentar(ponto(0, 0), ponto(Tcp::kRttsNoGrafico - 1, 0), kEixoDoGrafico);
    const int capacidade = mPlanoDaRede.capacidade;
    mLinhas->Acrescentar(ponto(0, capacidade), ponto(Tcp::kRttsNoGrafico - 1, capacidade), kCorDaCapacidade);
    // O dente de serra: o tamanho de cada janela mandada.
    for (size_t i = 1; i < mJanelasDoGrafico.size(); ++i) {
        mLinhas->Acrescentar(ponto(static_cast<int>(i) - 1, mJanelasDoGrafico[i - 1]),
                             ponto(static_cast<int>(i), mJanelasDoGrafico[i]), kCorDaSerra);
    }
    if (!mJanelasDoGrafico.empty()) {
        const Vector2 u = ponto(static_cast<int>(mJanelasDoGrafico.size()) - 1, mJanelasDoGrafico.back());
        mLinhas->Acrescentar(u + Vector2(-3.0f, 0.0f), u + Vector2(3.0f, 0.0f), kCorDaSerra);
        mLinhas->Acrescentar(u + Vector2(0.0f, -3.0f), u + Vector2(0.0f, 3.0f), kCorDaSerra);
    }
}

void Vitor::AtualizarNuvens(const float deltaTime) {
    mEmVoo.erase(std::remove_if(mEmVoo.begin(), mEmVoo.end(),
                                [](const Segmento& s) { return !Vivo(s.p); }), mEmVoo.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasNuvens = fsm && fsm->GetStateName() == kFaseDasNuvens && !EhClone();

    if (!nasNuvens) {
        if (mNasNuvens) {
            // A fase acabou.
            RestaurarAnuncio(kFaseDasNuvens);
            DesmontarRede();
            mNasNuvens = false;
            if (fsm && fsm->GetStateName() == kFaseDoFork && !EhClone()) {
                ComecarOFork();
            } else {
                for (BossProjectile* nuvem : Nuvens()) nuvem->Kill();
            }
        }
        return;
    }
    if (!mNasNuvens) {
        mNasNuvens = true;
        mNuvensFormadas = false;
        mTempoNasNuvens = 0.0f;
        mSemNuvensHa = 0.0f;
        mChuva.t = 0.0f;
        // A rede desta luta: sorteada aqui, antes de o roteador ser montado.
        Variacao::Sorteio sorteio = SorteioDaFase(kFaseDasNuvens);
        mDegrauDaRede = Tcp::DegrauDo(NivelDeDificuldade());
        mPlanoDaRede = Tcp::NoDegrau(Tcp::Sortear(sorteio), mDegrauDaRede);
        mConexao = Tcp::Abrir(mPlanoDaRede);
        mJanela = Tcp::Janela{};
        mJanelaReconhecida = true;
        mAvisoPendente = false;
        mMandando = false;
        mFila.clear();
        mDescartes = 0;
        mTextoDaConexao = Tcp::Placar(mConexao);
        MontarRede();
    }

    mTempoNasNuvens += deltaTime;
    const float agora = mTempoNasNuvens;

    // O que ja esta na rede continua andando, com ou sem nuvem.
    ChegarNoRoteador(agora);
    Encaminhar(agora);
    if (mFilaDoRoteador) mFilaDoRoteador->Escrever(Tcp::Fila(mRecebidosNaJanela, mPlanoDaRede.capacidade));
    DesenharGrafico();

    if (!mNuvensFormadas) {
        if (mTempoNasNuvens < Tcp::kFormam) return;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mNuvensFormadas = true;
        mChuva.t = 0.0f;
        return;     // as nuvens recem-disparadas so entram no campo no quadro seguinte
    }

    // A nuvem fora do ar: o Ponto Extra levou as duas, e o que estava na rede com elas.
    if (Nuvens().empty()) {
        mSemNuvensHa += deltaTime;
        mMandando = false;
        mFila.clear();
        TrocarAnuncio(kFaseDasNuvens, kForaDoAr);
        if (mSemNuvensHa < Nuvem::kVoltaDasNuvens) return;
        Tcp::Timeout(mConexao);
        mJanelaReconhecida = true;
        mTextoDaConexao = Tcp::PlacarDoTimeout(mConexao);
        mAvisoPendente = true;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mChuva.t = 0.0f;
        mSemNuvensHa = 0.0f;
        return;
    }
    mSemNuvensHa = 0.0f;

    if (mMandando) MandarSegmentos(agora);

    // O fim do processamento: a janela sai.
    if (Nuvem::Avancar(mChuva, deltaTime)) {
        if (!mJanelaReconhecida) {
            if (Tcp::Reconhecer(mConexao, mJanela) == Tcp::Desfecho::Cortou) {
                mTextoDaConexao = Tcp::PlacarDaPerda(mConexao);
                mAvisoPendente = true;
            }
        }
        mJanela = Tcp::Enviar(mConexao);
        mJanelasDoGrafico.push_back(mJanela.Total());
        mRecebidosNaJanela = 0;
        mMandando = true;
        mMandados = 0;
        mInicioDaJanela = agora;
        mJanelaReconhecida = false;
        // A perda e o timeout ficam escritos na janela que eles cortaram.
        if (!mAvisoPendente) mTextoDaConexao = Tcp::Placar(mConexao);
        mAvisoPendente = false;
        MandarSegmentos(agora);
    }

    // Os acks voltam quando a janela inteira saiu e chegou ao roteador - ou se perdeu nele.
    if (!mJanelaReconhecida && !mMandando && mEmVoo.empty()) {
        mJanelaReconhecida = true;
        if (Tcp::Reconhecer(mConexao, mJanela) == Tcp::Desfecho::Cortou) {
            mTextoDaConexao = Tcp::PlacarDaPerda(mConexao);
            mAvisoPendente = true;
        }
    }

    TrocarAnuncio(kFaseDasNuvens, mTextoDaConexao);

    const char* cara = Nuvem::Processando(mChuva) ? Nuvem::kCaraProcessando : Nuvem::kCaraOciosa;
    for (BossProjectile* nuvem : Nuvens()) PorCara(nuvem, cara);
}

void Vitor::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (mQuantos > 0) params.numProjectiles = mQuantos;
        if (mCentroEm) params.centralAngle = *mCentroEm;
        if (mAberturaEm) params.angle = *mAberturaEm;
    }
}

// A final: o jantar dos filosofos.

Vector2 Vitor::NoCampo(const Filosofos::Ponto p) const {
    const SDL_FRect campo = Campo();
    return Vector2(campo.x + p.x, campo.y + p.y);
}

int Vitor::FilosofoMaisPerto() const {
    const float w = Campo().w;
    const Vector2 aluno = PosicaoDoAluno();
    int melhor = 0;
    float menor = std::numeric_limits<float>::max();
    for (int f = 0; f < Filosofos::kFilosofos; ++f) {
        const float d = (NoCampo(Filosofos::Lugar(f, w)) - aluno).Length();
        if (d < menor) { menor = d; melhor = f; }
    }
    return melhor;
}

void Vitor::MontarJantar() {
    mNoJantar = true;
    mRodadaDoJantar = -1;
    mRelogioDoJantar = 0.0f;
    mMesa = Filosofos::MesaPosta();
    const float w = Campo().w;

    // O que esta luta sorteia para o jantar, e o degrau do modo.
    Variacao::Sorteio sorteio = SorteioDaFase(Filosofos::kFaseDoJantar);
    mPlanoDoJantar = Filosofos::Sortear(sorteio);
    mDegrauDoJantar = Filosofos::DegrauDo(NivelDeDificuldade());

    // Os filosofos: o nome de cada um, em volta da mesa.
    for (int f = 0; f < Filosofos::kFilosofos; ++f) {
        const std::string nome = mPlanoDoJantar.nomes[static_cast<size_t>(f)];
        auto etiqueta = std::make_unique<EtiquetaDeCodigo>(mScene, nome, LarguraDoTexto(nome), kAlturaDoNome,
                                                           kOrdemDosFilosofos);
        etiqueta->SetPosition(NoCampo(Filosofos::Lugar(f, w)));
        mNomesDosFilosofos[static_cast<size_t>(f)] = etiqueta.get();
        mScene->AddActor(std::move(etiqueta));
    }

    // Os garfos: enfeites - nao machucam, e o Ponto Extra nao os leva -, na mesa, entre cada dois.
    for (int g = 0; g < Filosofos::kFilosofos; ++g) {
        const auto criados = DispararEm(Filosofos::kGatilhoDoGarfo, NoCampo(Filosofos::LugarDoGarfo(g, w)), 1);
        BossProjectile* garfo = criados.empty() ? nullptr : criados.front();
        if (garfo) {
            garfo->DefinirEnfeite(true);
            Parar(garfo);
        }
        mGarfos[static_cast<size_t>(g)] = garfo;
    }
    PintarFilosofos();
}

void Vitor::DesmontarJantar() {
    for (EtiquetaDeCodigo*& nome : mNomesDosFilosofos) {
        if (nome) nome->SetState(ActorState::Destroy);
        nome = nullptr;
    }
    for (BossProjectile*& garfo : mGarfos) {
        if (Vivo(garfo)) garfo->Kill();
        garfo = nullptr;
    }
    for (BossProjectile* p : mCerco) if (Vivo(p)) p->Kill();
    mCerco.clear();
    RestaurarAnuncio(Filosofos::kFaseDoJantar);
    mNoJantar = false;
    mRodadaDoJantar = -1;
    mPassos.clear();
    mAlvo.reset();
}

void Vitor::ComecarRodadaDoJantar(const int k) {
    // A mesa e posta de novo: os garfos voltam ao lugar, e quem morreu ou saiu volta a mesa.
    for (BossProjectile* p : mCerco) if (Vivo(p)) p->Kill();
    mCerco.clear();
    mPostasNoCerco = 0;
    mCercoAberto = false;
    mMesa = Filosofos::MesaPosta();
    mProximaGarfada.fill(0.0f);
    for (BossProjectile* garfo : mGarfos) if (Vivo(garfo)) garfo->RenovarTempoDeVida();

    mRodadaDoJantar = k;
    mInicioDoJantar = Filosofos::kPrimeiro + static_cast<float>(k) * Filosofos::kPorRodada;
    mEscolhido = FilosofoMaisPerto();
    // O jantar e a ordem comecam de outro lugar da mesa a cada luta.
    const Filosofos::Rodada rodada = mPlanoDoJantar.rodadas[static_cast<size_t>(k)];
    mPassos = Filosofos::Roteiro(rodada, mEscolhido);
    if (rodada == Filosofos::Rodada::Jantar || rodada == Filosofos::Rodada::Ordem) {
        mPassos = Filosofos::Rodar(mPassos, mPlanoDoJantar.giro);
    }
    mProximoPasso = 0;
}

void Vitor::ExecutarNoJantar(const Filosofos::Passo& p) {
    if (!Filosofos::Aplicar(mMesa, p)) return;
    switch (p.acao) {
        case Filosofos::Acao::Come:
            mProximaGarfada[static_cast<size_t>(p.filosofo)] =
                mRelogioDoJantar + Filosofos::kPrimeiraGarfada;
            break;
        case Filosofos::Acao::Morre:
            // Kill -9: o ciclo quebra, e o cerco das esperas estoura para fora.
            EstourarCerco();
            break;
        default:
            break;
    }
}

void Vitor::MoverGarfos(const float deltaTime) {
    const float w = Campo().w;
    for (int g = 0; g < Filosofos::kFilosofos; ++g) {
        BossProjectile* garfo = mGarfos[static_cast<size_t>(g)];
        if (!Vivo(garfo)) continue;
        const int dono = mMesa.dono[static_cast<size_t>(g)];
        const Vector2 alvo = NoCampo(dono >= 0 ? Filosofos::NaMao(dono, g, w) : Filosofos::LugarDoGarfo(g, w));
        const Vector2 falta = alvo - garfo->GetPosition();
        const float passo = kVelocidadeDoGarfo * deltaTime;
        garfo->SetPosition(falta.Length() <= passo ? alvo : garfo->GetPosition() + falta * (passo / falta.Length()));
        Parar(garfo);
    }
}

void Vitor::FecharCerco(const float u) {
    // O cerco cresce enquanto ninguem cede.
    if (mCercoAberto || !Filosofos::EmDeadlock(mMesa)) return;
    const float w = Campo().w;
    const auto esperas = Filosofos::Esperas(mMesa);
    const int porAresta = mDegrauDoJantar.porAresta;
    const int total = static_cast<int>(esperas.size()) * porAresta;
    if (mPostasNoCerco == 0) mInicioDoCerco = u;
    while (mPostasNoCerco < total
           && u >= mInicioDoCerco + static_cast<float>(mPostasNoCerco) * Filosofos::kEntreEsperas) {
        const auto& [de, para] = esperas[static_cast<size_t>(mPostasNoCerco / porAresta)];
        const auto pontos = Filosofos::Cerco(de, para, w, porAresta);
        const Vector2 onde = NoCampo(pontos[static_cast<size_t>(mPostasNoCerco % porAresta)]);
        for (BossProjectile* p : DispararEm(Filosofos::kGatilhoDaEspera, onde, 1)) {
            Parar(p);
            mCerco.push_back(p);
        }
        ++mPostasNoCerco;
    }
}

void Vitor::EstourarCerco() {
    mCercoAberto = true;
    const Vector2 c = NoCampo(Filosofos::Centro(Campo().w));
    for (BossProjectile* p : mCerco) {
        if (!Vivo(p)) continue;
        Lancar(p, Angulo(c, p->GetPosition()), mDegrauDoJantar.velocidadeDoEstouro);
    }
    mCerco.clear();
}

void Vitor::PintarFilosofos() {
    for (int f = 0; f < Filosofos::kFilosofos; ++f) {
        EtiquetaDeCodigo* nome = mNomesDosFilosofos[static_cast<size_t>(f)];
        if (!nome) continue;
        switch (mMesa.estado[static_cast<size_t>(f)]) {
            case Filosofos::Estado::Comendo:   nome->Pintar(kComendo, kFundoDeQuemCome, kComendo); break;
            case Filosofos::Estado::Esperando: nome->Pintar(kEsperando, kFundoDeQuemEspera, kEsperando); break;
            case Filosofos::Estado::Morto:     nome->Pintar(kMorto, kFundoDoNome, kMorto); break;
            case Filosofos::Estado::DeFora:    nome->Pintar(kDeFora, kFundoDoNome, kDeFora); break;
            default:                           nome->Pintar(kPensando, kFundoDoNome, kPensando); break;
        }
    }
}

void Vitor::DesenharJantar() {
    if (!mLinhas) return;
    const float w = Campo().w;
    const Vector2 c = NoCampo(Filosofos::Centro(w));

    // A mesa: um circulo de 24 lados.
    constexpr int kLados = 24;
    for (int i = 0; i < kLados; ++i) {
        const float a0 = Math::TwoPi * static_cast<float>(i) / kLados;
        const float a1 = Math::TwoPi * static_cast<float>(i + 1) / kLados;
        mLinhas->Acrescentar(c + Vector2(std::cos(a0), std::sin(a0)) * Filosofos::kRaioDaMesa,
                             c + Vector2(std::cos(a1), std::sin(a1)) * Filosofos::kRaioDaMesa, kCorDaMesa);
    }

    // As esperas: de quem espera ate quem tem o garfo, com a ponta da seta.
    for (const auto& [quem, de] : Filosofos::Esperas(mMesa)) {
        const Vector2 a = NoCampo(Filosofos::Lugar(quem, w));
        const Vector2 b = NoCampo(Filosofos::Lugar(de, w));
        Vector2 d = b - a;
        const float n = d.Length();
        if (n < 1.0f) continue;
        d = d * (1.0f / n);
        const Vector2 de0 = a + d * 30.0f;
        const Vector2 ate = b - d * 30.0f;
        mLinhas->Acrescentar(de0, ate, kLinhaDaEspera);
        const Vector2 lado(-d.y, d.x);
        mLinhas->Acrescentar(ate, ate - d * 12.0f + lado * 7.0f, kLinhaDaEspera);
        mLinhas->Acrescentar(ate, ate - d * 12.0f - lado * 7.0f, kLinhaDaEspera);
    }
}

void Vitor::AtualizarJantar(const float deltaTime) {
    mCerco.erase(std::remove_if(mCerco.begin(), mCerco.end(),
                                [](const BossProjectile* p) { return !Vivo(p); }), mCerco.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Filosofos::kFaseDoJantar && !EhClone();
    if (!aqui) {
        if (mNoJantar) DesmontarJantar();
        return;
    }
    if (!mNoJantar) MontarJantar();

    const float t = fsm->GetStateTime();
    const SDL_FRect campo = Campo();
    const Vector2 centro = NoCampo(Filosofos::Centro(campo.w));
    mAlvo = centro;
    MoverGarfos(deltaTime);
    DesenharJantar();
    if (t < Filosofos::kPrimeiro) return;

    const int k = std::min(Filosofos::kRodadas - 1,
                           static_cast<int>((t - Filosofos::kPrimeiro) / Filosofos::kPorRodada));
    if (k != mRodadaDoJantar) ComecarRodadaDoJantar(k);
    const float u = t - mInicioDoJantar;
    mRelogioDoJantar = u;

    while (mProximoPasso < mPassos.size() && u >= mPassos[mProximoPasso].quando) {
        ExecutarNoJantar(mPassos[mProximoPasso++]);
    }
    FecharCerco(u);

    // Quem come, ataca: garfadas em leque, miradas no aluno.
    const Vector2 aluno = PosicaoDoAluno();
    for (int f = 0; f < Filosofos::kFilosofos; ++f) {
        if (mMesa.estado[static_cast<size_t>(f)] != Filosofos::Estado::Comendo) continue;
        float& proxima = mProximaGarfada[static_cast<size_t>(f)];
        if (u < proxima) continue;
        const Vector2 de = NoCampo(Filosofos::Lugar(f, campo.w));
        DispararEm(Filosofos::kGatilhoDaGarfada, de, Filosofos::kGarfadasPorLeque, Angulo(de, aluno), kAbertoDaGarfada,
                   mDegrauDoJantar.rapidez);
        proxima = u + mDegrauDoJantar.entreGarfadas;
    }

    // O professor e o monitor: no deadlock vai ate a vitima; fora dele, vai ate quem come.
    const Filosofos::Rodada rodada = mPlanoDoJantar.rodadas[static_cast<size_t>(k)];
    Vector2 alvo = centro + Vector2(kBalancoNaMesa * std::sin(kRitmoDoBalanco * t), 0.0f);
    int vigiado = -1;
    if (rodada == Filosofos::Rodada::Deadlock && Filosofos::EmDeadlock(mMesa)) vigiado = mEscolhido;
    else if (const auto comem = Filosofos::Comendo(mMesa); !comem.empty()) vigiado = comem.front();
    else if (rodada == Filosofos::Rodada::Garcom && u >= 0.1f) vigiado = mEscolhido;
    if (vigiado >= 0) {
        const Vector2 la = NoCampo(Filosofos::Lugar(vigiado, campo.w));
        alvo = centro + (la - centro) * kPertoDoEscolhido;
    }
    mAlvo = alvo;

    PintarFilosofos();
    TrocarAnuncio(Filosofos::kFaseDoJantar, Filosofos::Placar(rodada, mMesa, mPlanoDoJantar));
}

// A fase 3: a troca de mensagens.

Vitor* Vitor::Servidor() const {
    const auto* batalha = Batalha();
    if (!batalha) return nullptr;
    for (Boss* corpo : batalha->Corpos()) {
        if (corpo && corpo != this && corpo->EhClone()) return dynamic_cast<Vitor*>(corpo);
    }
    return nullptr;
}

Vector2 Vitor::PosicaoDe(const int processo) const {
    if (processo == Mensagens::kCliente) return GetPosition();
    if (const Vitor* servidor = Servidor()) return servidor->GetPosition();
    return Vector2(Campo().x + Campo().w - (GetPosition().x - Campo().x), GetPosition().y);
}

Mensagens::Rodada Vitor::RodadaDasMensagens(const int k) const {
    return mPlanoDasMensagens.rodadas[static_cast<size_t>(((k % Mensagens::kRodadas) + Mensagens::kRodadas) %
                                                          Mensagens::kRodadas)];
}

void Vitor::MontarMensagens() {
    mNasMensagens = true;
    mRodadaDasMensagens = -1;

    // O que esta luta sorteia para a fase, e o degrau do modo.
    Variacao::Sorteio sorteio = SorteioDaFase(Mensagens::kFaseDasMensagens);
    mPlanoDasMensagens = Mensagens::Sortear(sorteio);
    mDegrauDasMensagens = Mensagens::DegrauDo(NivelDeDificuldade());
    for (int processo = 0; processo < 2; ++processo) {
        const std::string nome = Mensagens::Nome(processo);
        auto etiqueta = std::make_unique<EtiquetaDeCodigo>(mScene, nome, LarguraDoTexto(nome), kAlturaDoNome,
                                                           kOrdemDosFilosofos);
        etiqueta->SetPosition(PosicaoDe(processo) + Vector2(0.0f, kAbaixoDoProcesso));
        etiqueta->Pintar(kMolduraDoProcesso, kFundoDoNome, kTintaDoProcesso);
        mNomesDosProcessos[static_cast<size_t>(processo)] = etiqueta.get();
        mScene->AddActor(std::move(etiqueta));
    }
}

void Vitor::DesmontarMensagens() {
    for (EtiquetaDeCodigo*& nome : mNomesDosProcessos) {
        if (nome) nome->SetState(ActorState::Destroy);
        nome = nullptr;
    }
    for (const Envelope& e : mEnvelopes) if (Vivo(e.p)) e.p->Kill();
    mEnvelopes.clear();
    RestaurarAnuncio(Mensagens::kFaseDasMensagens);
    mNasMensagens = false;
    mRodadaDasMensagens = -1;
    mAlvo.reset();
    if (Vitor* servidor = Servidor()) servidor->mAlvo.reset();
}

void Vitor::ComecarRodadaDasMensagens(const int k) {
    // O que ainda corria no canal some: os dois mudam de lugar, e o canal e outro.
    for (const Envelope& e : mEnvelopes) if (Vivo(e.p)) e.p->Kill();
    mEnvelopes.clear();
    mRodadaDasMensagens = k;
    mInicioDasMensagens = Mensagens::kComeca + static_cast<float>(k) * Mensagens::kPorRodada;
    mRoteiroDasMensagens = Mensagens::Roteiro(RodadaDasMensagens(k), mPlanoDasMensagens, mDegrauDasMensagens);
    mProximaMensagem = 0;
    mChamadaDaRpc = 0;
    mRecebidas = 0;
    mEsperandoRpc = false;
    mEstourou = false;

    // Os dois vao para o lugar da rodada: o original conduz os dois corpos.
    const SDL_FRect campo = Campo();
    const Mensagens::Ponto cliente = Mensagens::Lugar(Mensagens::kCliente, k, campo.w, mPlanoDasMensagens);
    const Mensagens::Ponto servidor = Mensagens::Lugar(Mensagens::kServidor, k, campo.w, mPlanoDasMensagens);
    mAlvo = Vector2(campo.x + cliente.x, campo.y + cliente.y);
    if (Vitor* outro = Servidor()) outro->mAlvo = Vector2(campo.x + servidor.x, campo.y + servidor.y);
}

void Vitor::ExecutarMensagem(const Mensagens::Evento& e) {
    const Vector2 aluno = PosicaoDoAluno();
    const Mensagens::Rodada rodada = RodadaDasMensagens(mRodadaDasMensagens);
    const float rapidez = mDegrauDasMensagens.rapidez;
    const float processado = Mensagens::kVelocidadeDoProcessado * rapidez;
    auto anel = [this, aluno, rapidez](const Vector2 de) {
        const int n = mDegrauDasMensagens.anelDoEncontro;
        DispararEm(Mensagens::kGatilhoDoEncontro, de, n, Coercao::CentroDoAnel(n, Angulo(de, aluno)),
                   Coercao::AberturaDoAnel(n), rapidez);
    };
    switch (e.o) {
        case Mensagens::Acontece::Envia:
        case Mensagens::Acontece::Responde: {
            // O envelope sai e corre pelo canal; quem o move e MoverEnvelopes.
            const auto criados = DispararEm(Mensagens::kGatilhoDoEnvelope, PosicaoDe(e.de), 1);
            for (BossProjectile* p : criados) {
                Parar(p);
                PorCara(p, Mensagens::kCaraDoEnvelope);
                mEnvelopes.push_back({p, e.de, mRelogioDasMensagens, e.voo, e.paraNaPorta});
            }
            if (e.o == Mensagens::Acontece::Envia && rodada == Mensagens::Rodada::Rpc) mEsperandoRpc = true;
            break;
        }
        case Mensagens::Acontece::Recebe: {
            // Receive: a mensagem processada vai para o aluno.
            const Vector2 de = PosicaoDe(e.de);
            const int leque = mDegrauDasMensagens.lequeDoRecebido;
            DispararEm(Mensagens::kGatilhoDoProcessado, de, leque,
                       AnguloAdiante(de, processado, Tcp::Antecipacao(mRecebidas++, false)),
                       leque > 1 ? std::optional<float>(mDegrauDasMensagens.aberturaDoRecebido) : std::nullopt, rapidez);
            break;
        }
        case Mensagens::Acontece::Encontro:
            // O rendezvous: os dois andam juntos, e atiram juntos.
            anel(PosicaoDe(Mensagens::kCliente));
            anel(PosicaoDe(Mensagens::kServidor));
            break;
        case Mensagens::Acontece::Resultado: {
            // O retorno da rpc: um leque com tantos quanto a soma, o do meio no aluno (as somas sao impares).
            const Vector2 de = PosicaoDe(Mensagens::kCliente);
            DispararEm(Mensagens::kGatilhoDoProcessado, de, e.valor,
                       AnguloAdiante(de, processado, 1.0f),
                       kAberturaPorDado * static_cast<float>(e.valor - 1), rapidez);
            mEsperandoRpc = false;
            break;
        }
        case Mensagens::Acontece::Estoura:
            // O timeout: os envelopes parados na porta estouram.
            for (const Envelope& env : mEnvelopes) {
                if (!env.paraNaPorta || !Vivo(env.p)) continue;
                anel(env.p->GetPosition());
                env.p->Kill();
            }
            mEstourou = true;
            break;
    }
}

void Vitor::MoverEnvelopes(const float u) {
    const float fundo = Campo().y + Mensagens::Fundo(RodadaDasMensagens(mRodadaDasMensagens));
    for (auto it = mEnvelopes.begin(); it != mEnvelopes.end();) {
        if (!Vivo(it->p)) { it = mEnvelopes.erase(it); continue; }
        float s = it->voo > 0.0f ? (u - it->saiu) / it->voo : 1.0f;
        if (it->paraNaPorta && s >= Mensagens::kNaPorta) {
            s = Mensagens::kNaPorta;
            PorCara(it->p, Mensagens::kCaraDoEsperando);
        }
        const Vector2 de = PosicaoDe(it->de);
        const Vector2 para = PosicaoDe(it->de == Mensagens::kCliente ? Mensagens::kServidor : Mensagens::kCliente);
        // Chegou: entra na caixa postal de quem recebe - o roteiro conta.
        if (s >= 1.0f) {
            it->p->Kill();
            it = mEnvelopes.erase(it);
            continue;
        }
        const Mensagens::Ponto q = Mensagens::NoArco({de.x, de.y}, {para.x, para.y}, s, fundo);
        it->p->SetPosition(Vector2(q.x, q.y));
        Parar(it->p);
        ++it;
    }
}

void Vitor::DesenharCanal() {
    if (!mLinhas) return;
    const float fundo = Campo().y + Mensagens::Fundo(RodadaDasMensagens(mRodadaDasMensagens));
    const Vector2 de = PosicaoDe(Mensagens::kCliente);
    const Vector2 para = PosicaoDe(Mensagens::kServidor);
    constexpr int kPedacos = 24;
    for (int i = 0; i < kPedacos; ++i) {
        const Mensagens::Ponto a = Mensagens::NoArco({de.x, de.y}, {para.x, para.y},
                                                     static_cast<float>(i) / kPedacos, fundo);
        const Mensagens::Ponto b = Mensagens::NoArco({de.x, de.y}, {para.x, para.y},
                                                     static_cast<float>(i + 1) / kPedacos, fundo);
        if (i % 2 == 0) mLinhas->Acrescentar(Vector2(a.x, a.y), Vector2(b.x, b.y), kCorDoCanal);
    }
}

void Vitor::AtualizarMensagens() {
    mEnvelopes.erase(std::remove_if(mEnvelopes.begin(), mEnvelopes.end(),
                                    [](const Envelope& e) { return !Vivo(e.p); }), mEnvelopes.end());

    const auto* fsm = GetComponent<FSMComponent>();
    const bool aqui = fsm && fsm->GetStateName() == Mensagens::kFaseDasMensagens && !EhClone();
    if (!aqui) {
        if (mNasMensagens) DesmontarMensagens();
        return;
    }
    // No fork ainda nao ha dois: os nomes e o canal so aparecem quando a nuvem se desfaz.
    if (mTempoNoFork >= 0.0f) return;
    if (!mNasMensagens) MontarMensagens();

    for (int processo = 0; processo < 2; ++processo) {
        if (EtiquetaDeCodigo* nome = mNomesDosProcessos[static_cast<size_t>(processo)]) {
            nome->SetPosition(PosicaoDe(processo) + Vector2(0.0f, kAbaixoDoProcesso));
        }
    }

    const float t = fsm->GetStateTime();
    if (t < Mensagens::kComeca) return;
    const int k = std::min(Mensagens::kRodadas - 1,
                           static_cast<int>((t - Mensagens::kComeca) / Mensagens::kPorRodada));
    if (k != mRodadaDasMensagens) ComecarRodadaDasMensagens(k);
    const float u = t - mInicioDasMensagens;
    mRelogioDasMensagens = u;

    while (mProximaMensagem < mRoteiroDasMensagens.size() && u >= mRoteiroDasMensagens[mProximaMensagem].quando) {
        const Mensagens::Evento& e = mRoteiroDasMensagens[mProximaMensagem++];
        if (e.o == Mensagens::Acontece::Envia && RodadaDasMensagens(k) == Mensagens::Rodada::Rpc) {
            mChamadaDaRpc = 0;
            for (size_t i = 0; i + 1 < mProximaMensagem; ++i) {
                if (mRoteiroDasMensagens[i].o == Mensagens::Acontece::Envia) ++mChamadaDaRpc;
            }
        }
        ExecutarMensagem(e);
    }
    MoverEnvelopes(u);
    DesenharCanal();

    // O placar: a caixa do cliente na assincrona.
    const Mensagens::Rodada rodada = RodadaDasMensagens(k);
    bool bloqueado = false;
    if (rodada == Mensagens::Rodada::Sincrona) {
        for (const Envelope& e : mEnvelopes) bloqueado = bloqueado || e.de == Mensagens::kCliente;
    } else if (rodada == Mensagens::Rodada::Rpc) {
        bloqueado = mEsperandoRpc;
    }
    TrocarAnuncio(Mensagens::kFaseDasMensagens,
             Mensagens::Placar(rodada, Mensagens::NaCaixa(mRoteiroDasMensagens, Mensagens::kCliente, u),
                               mChamadaDaRpc, bloqueado, mEstourou, mPlanoDasMensagens));
}
