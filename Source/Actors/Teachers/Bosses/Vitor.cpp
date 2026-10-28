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
#include "../../../Random.h"

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

    // A fase 1: as cores das threads, as MESMAS de Ferramentas/gerar_vitor.py
    // (CORES_DAS_THREADS): o nome da coluna e o chip da instrucao se casam.
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
    // A TELA DA NOTA FICA LIMPA: os projeteis a batalha ja levou; a faixa, os
    // nomes das threads e as linhas sao do professor.
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

SDL_FRect Vitor::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, 1200.0f, 746.0f};
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

Player* Vitor::Aluno() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->Alvo();
    return nullptr;
}

void Vitor::AcompanharOAluno(const float deltaTime) {
    if (deltaTime <= 0.0f) return;
    const Vector2 agora = PosicaoDoAluno();
    if (mViuOAluno) {
        // A MEDIA QUE ESQUECE: cada quadro pesa deltaTime / kMemoriaDoAndar.
        const Vector2 doQuadro = (agora - mUltimaDoAluno) * (1.0f / deltaTime);
        const float peso = std::min(1.0f, deltaTime / kMemoriaDoAndar);
        mVelocidadeDoAluno = mVelocidadeDoAluno + (doQuadro - mVelocidadeDoAluno) * peso;
    }
    mUltimaDoAluno = agora;
    mViuOAluno = true;
}

Vector2 Vitor::PosicaoDoAluno() const {
    if (const Player* aluno = Aluno()) return aluno->GetPosition();
    const SDL_FRect campo = Campo();
    return Vector2(campo.x + campo.w / 2.0f, campo.y + campo.h - 60.0f);
}

void Vitor::Anunciar(const std::string& fase, const std::string& texto) {
    if (mAnunciosDoArquivo.count(fase) == 0) mAnunciosDoArquivo[fase] = AnuncioDaFase(fase);
    if (AnuncioDaFase(fase) != texto) RegistrarAnuncio(fase, texto);
}

void Vitor::Restaurar(const std::string& fase) {
    const auto it = mAnunciosDoArquivo.find(fase);
    if (it != mAnunciosDoArquivo.end()) RegistrarAnuncio(fase, it->second);
}

// ---------------------------------------------------------------------------
// A fase 1: os semaforos
// ---------------------------------------------------------------------------

void Vitor::MontarSemaforos() {
    mNosSemaforos = true;
    mRodadaDoSemaforo = -1;
    mAlunoPreso = false;
    const SDL_FRect campo = Campo();

    // O QUE ESTA LUTA SORTEIA para a fase, e o degrau do modo.
    Variacao::Sorteio sorteio = SorteioDaFase(Semaforo::kFaseDosSemaforos);
    mPlanoDosSemaforos = Semaforo::Sortear(sorteio);
    mDegrauDosSemaforos = Semaforo::DegrauDo(NivelDeDificuldade());

    // A SECAO CRITICA: a faixa de borda a borda, com o codigo dela no canto.
    auto secao = std::make_unique<PainelDeCodigo>(mScene, static_cast<int>(campo.w - 40.0f),
                                                  static_cast<int>(Semaforo::kAlturaDaSecao), kOrdemDaSecao);
    secao->SetPosition(Vector2(campo.x + campo.w / 2.0f,
                               campo.y + Semaforo::kTopoDaSecao + Semaforo::kAlturaDaSecao / 2.0f));
    secao->Pintar(kMolduraDaSecao, kFundoDaSecao);
    mSecao = secao.get();
    mScene->AddActor(std::move(secao));

    // O CANTO cabe o codigo mais comprido das quatro rodadas.
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
    // A FAIXA SOME, e com ela a porta: o que estava parado nela some junto. O
    // que ainda descia continua descendo - ja eram instrucoes no ar.
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
    Restaurar(Semaforo::kFaseDosSemaforos);
    mNosSemaforos = false;
    mRodadaDoSemaforo = -1;
    mAlvo.reset();
}

void Vitor::ComecarRodadaDoSemaforo(const int k) {
    // O QUE SOBROU DA RODADA ANTERIOR passa pela porta - a faixa continua -,
    // e as colunas sao refeitas em cima do aluno.
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
    // V(s) DE QUEM SAIU, P(s) DESTA: a pilha inteira desce de uma vez. No
    // mutex e no semaforo ela abre em leque; na espera ocupada o giro estoura
    // pelo raio, num anel.
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
        // PRESO, o aluno deixa de estar seguro ao lado da coluna.
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

        // BLOQUEADA: quem chega a porta para. Na pilha, uma em cima da outra;
        // na espera ocupada, entra no giro.
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

    // A CORRIDA: as instrucoes se intercalam sem trava, e cada uma sai da
    // faixa torta, para um lado sorteado. Quem ja passou do meio da faixa sai
    // da lista.
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
    // O que o campo levou (o Ponto Extra, a borda da tela) sai das listas
    // antes de qualquer coisa deste quadro.
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

    // A CORRENTE: o aluno preso esta bloqueado em P(s), e a coluna em cima
    // dele fura a fila - uma vez por corrente, na hora em que ele e pego.
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

    // AS THREADS MANDAM INSTRUCOES. A bloqueada com a pilha cheia nao anda: e
    // isso que bloquear quer dizer.
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

    // O PROFESSOR fica ao lado da coluna que ele vai acordar - ou da que esta
    // dentro, quando nao se sabe quem e a proxima -, entre ela e a vizinha.
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
    Anunciar(Semaforo::kFaseDosSemaforos, Semaforo::Placar(mEscalonador, preso));
}

void Vitor::OnUpdate(const float deltaTime) {

    // NO FORK o corpo nao anda sozinho nem ataca: o relogio da fase corre, mas
    // os ataques e o movimento dele esperam a nuvem se desfazer. E ele continua
    // levando tiro - o colisor fica onde ele esta -, porque o fork nao e um
    // intervalo da batalha.
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

    // A NUVEM SE DESFAZ, e o placar volta ao nome da fase: sairam dois.
    for (BossProjectile* nuvem : Nuvens()) nuvem->Kill();
    if (!mNomeDaFaseDoFork.empty()) RegistrarAnuncio(kFaseDoFork, mNomeDaFaseDoFork);
}

void Vitor::ComecarOFork() {
    auto* batalha = dynamic_cast<Battle*>(mScene);
    const auto nuvens = Nuvens();
    if (!batalha || nuvens.empty()) return;   // sem nuvem, o fork de sempre

    // AS DUAS NUVENS SE JUNTAM EM CIMA DELE (AtualizarANuvemDoFork as faz
    // crescer e processar). A Ancorar nova toma o lugar da que as prendia dos
    // lados, e o fim dela fica depois do fim do fork: quem as desfaz e
    // SairDaNuvem, e nao a queda.
    mEscalaDaNuvem = nuvens.front()->GetScale();
    for (BossProjectile* nuvem : nuvens) {
        // ELAS VIVEM A FASE 2 INTEIRA, e o fork e na 3: sem renovar, a rede
        // de seguranca de 20 s as mataria no meio dele (Projectile.h).
        nuvem->RenovarTempoDeVida();
        nuvem->insertMotion<AncorarBehavior>(Vector2::Zero, kFork.sai + 0.5f, 200.0f);
    }

    // ELE ENTRA quando a nuvem chega, e fica onde esta. O OUTRO CORPO nasce
    // dentro dela - os dois processos de um fork saem do mesmo lugar - e,
    // quando ela abre, anda ate o lugar em que Battle::DividirChefe o pos, o
    // espelho do original: dali em diante a fase e a de sempre.
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
    // A NUVEM CRESCE enquanto as duas se juntam, e cobre o professor quando ele
    // entra; processa ate abrir, e abre ociosa - o fork acabou.
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
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return nuvens;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
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

    // O ROTEADOR: a caixa no meio do campo, com a fila dele escrita.
    const std::string fila = Tcp::Fila(mPlanoDaRede.capacidade, mPlanoDaRede.capacidade);
    auto roteador = std::make_unique<EtiquetaDeCodigo>(mScene, fila, LarguraDoTexto(fila), kAlturaDaCaixaDoRoteador,
                                                       kOrdemDosNomes);
    roteador->SetPosition(PosicaoDoRoteador());
    roteador->Pintar(kMolduraDoRoteador, kFundoDoRoteador, kTintaDoRoteador);
    roteador->Escrever(Tcp::Fila(0, mPlanoDaRede.capacidade));
    mFilaDoRoteador = roteador.get();
    mScene->AddActor(std::move(roteador));

    // O GRAFICO DO cwnd, no canto de cima a direita.
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
    // A JANELA SAI AOS POUCOS, um segmento a cada kEntreSegmentos, alternando
    // as nuvens - e cada um voa reto ate o roteador.
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
            // O ROTEADOR ESTAVA CHEIO: o segmento e descartado ali mesmo, e
            // estoura num anel de cacos - com um deles no aluno.
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
    // O ENLACE DE SAIDA: um a cada kEntreEncaminhados, mirado ADIANTE do aluno
    // - para onde ele estara se continuar andando como anda (Tcp::Mira).
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
    // O EIXO e a CAPACIDADE do roteador: o que passa da linha vermelha se perde.
    mLinhas->Acrescentar(ponto(0, 0), ponto(Tcp::kRttsNoGrafico - 1, 0), kEixoDoGrafico);
    const int capacidade = mPlanoDaRede.capacidade;
    mLinhas->Acrescentar(ponto(0, capacidade), ponto(Tcp::kRttsNoGrafico - 1, capacidade), kCorDaCapacidade);
    // O DENTE DE SERRA: o tamanho de cada janela mandada.
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
            // A FASE ACABOU. Se a seguinte e o fork, as nuvens sao a porta dele;
            // se nao, elas se desfazem. O placar volta ao nome.
            Restaurar(kFaseDasNuvens);
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
        // A REDE DESTA LUTA: sorteada aqui, antes de o roteador ser montado.
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

    // O QUE JA ESTA NA REDE continua andando, com ou sem nuvem.
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

    // A NUVEM FORA DO AR: o Ponto Extra levou as duas, e o que estava na rede
    // com elas. Quando elas voltam, e um TIMEOUT: a conexao recomeca do slow
    // start, com cwnd = 1.
    if (Nuvens().empty()) {
        mSemNuvensHa += deltaTime;
        mMandando = false;
        mFila.clear();
        Anunciar(kFaseDasNuvens, kForaDoAr);
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

    // O FIM DO PROCESSAMENTO: a janela sai. Os ACKs da anterior que ainda nao
    // tinham voltado voltam agora.
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
        // A PERDA E O TIMEOUT FICAM ESCRITOS na janela que eles cortaram.
        if (!mAvisoPendente) mTextoDaConexao = Tcp::Placar(mConexao);
        mAvisoPendente = false;
        MandarSegmentos(agora);
    }

    // OS ACKS VOLTAM quando a janela inteira saiu e chegou ao roteador - ou se
    // perdeu nele.
    if (!mJanelaReconhecida && !mMandando && mEmVoo.empty()) {
        mJanelaReconhecida = true;
        if (Tcp::Reconhecer(mConexao, mJanela) == Tcp::Desfecho::Cortou) {
            mTextoDaConexao = Tcp::PlacarDaPerda(mConexao);
            mAvisoPendente = true;
        }
    }

    Anunciar(kFaseDasNuvens, mTextoDaConexao);

    const char* cara = Nuvem::Processando(mChuva) ? Nuvem::kCaraProcessando : Nuvem::kCaraOciosa;
    for (BossProjectile* nuvem : Nuvens()) PorCara(nuvem, cara);
}

void Vitor::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    // O ATAQUE DE GATILHO nasce onde o professor disse, no numero que ele
    // pediu, para o lado que ele pediu.
    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (mQuantos > 0) params.numProjectiles = mQuantos;
        if (mCentroEm) params.centralAngle = *mCentroEm;
        if (mAberturaEm) params.angle = *mAberturaEm;
    }
}

// ---------------------------------------------------------------------------
// A final: o jantar dos filosofos
// ---------------------------------------------------------------------------

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

    // O QUE ESTA LUTA SORTEIA para o jantar, e o degrau do modo.
    Variacao::Sorteio sorteio = SorteioDaFase(Filosofos::kFaseDoJantar);
    mPlanoDoJantar = Filosofos::Sortear(sorteio);
    mDegrauDoJantar = Filosofos::DegrauDo(NivelDeDificuldade());

    // OS FILOSOFOS: o nome de cada um, em volta da mesa.
    for (int f = 0; f < Filosofos::kFilosofos; ++f) {
        const std::string nome = mPlanoDoJantar.nomes[static_cast<size_t>(f)];
        auto etiqueta = std::make_unique<EtiquetaDeCodigo>(mScene, nome, LarguraDoTexto(nome), kAlturaDoNome,
                                                           kOrdemDosFilosofos);
        etiqueta->SetPosition(NoCampo(Filosofos::Lugar(f, w)));
        mNomesDosFilosofos[static_cast<size_t>(f)] = etiqueta.get();
        mScene->AddActor(std::move(etiqueta));
    }

    // OS GARFOS: enfeites - nao machucam, e o Ponto Extra nao os leva -, na
    // mesa, entre cada dois.
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
    Restaurar(Filosofos::kFaseDoJantar);
    mNoJantar = false;
    mRodadaDoJantar = -1;
    mPassos.clear();
    mAlvo.reset();
}

void Vitor::ComecarRodadaDoJantar(const int k) {
    // A MESA E POSTA DE NOVO: os garfos voltam ao lugar, e quem morreu ou
    // saiu volta a mesa - e outra rodada, outro jantar.
    for (BossProjectile* p : mCerco) if (Vivo(p)) p->Kill();
    mCerco.clear();
    mPostasNoCerco = 0;
    mCercoAberto = false;
    mMesa = Filosofos::MesaPosta();
    mProximaGarfada.fill(0.0f);
    // OS GARFOS SAO DA FASE INTEIRA: a rede de seguranca de 20 s nao os leva
    // nos ultimos quadros dela (Projectile::RenovarTempoDeVida).
    for (BossProjectile* garfo : mGarfos) if (Vivo(garfo)) garfo->RenovarTempoDeVida();

    mRodadaDoJantar = k;
    mInicioDoJantar = Filosofos::kPrimeiro + static_cast<float>(k) * Filosofos::kPorRodada;
    mEscolhido = FilosofoMaisPerto();
    // O JANTAR E A ORDEM COMECAM DE OUTRO LUGAR DA MESA a cada luta; no
    // deadlock e no garcom quem decide e o escolhido.
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
            // KILL -9: o ciclo quebra, e o cerco das esperas estoura para fora.
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
    // O CERCO CRESCE enquanto ninguem cede: uma espera a cada kEntreEsperas,
    // aresta por aresta do pentagono.
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

    // A MESA: um circulo de 24 lados.
    constexpr int kLados = 24;
    for (int i = 0; i < kLados; ++i) {
        const float a0 = Math::TwoPi * static_cast<float>(i) / kLados;
        const float a1 = Math::TwoPi * static_cast<float>(i + 1) / kLados;
        mLinhas->Acrescentar(c + Vector2(std::cos(a0), std::sin(a0)) * Filosofos::kRaioDaMesa,
                             c + Vector2(std::cos(a1), std::sin(a1)) * Filosofos::kRaioDaMesa, kCorDaMesa);
    }

    // AS ESPERAS: de quem espera ate quem tem o garfo, com a ponta da seta -
    // encurtadas para nao entrar nos nomes.
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

    // QUEM COME, ATACA: garfadas em leque, miradas no aluno.
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

    // O PROFESSOR E O MONITOR: no deadlock vai ate a vitima; fora dele, vai
    // ate quem come - e quem pega os dois garfos que ele vigia -; e o garcom
    // fica ao lado de quem ele tirou da mesa. Sem nada disso, balanca no meio.
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
    Anunciar(Filosofos::kFaseDoJantar, Filosofos::Placar(rodada, mMesa, mPlanoDoJantar));
}

// ---------------------------------------------------------------------------
// A fase 3: a troca de mensagens
// ---------------------------------------------------------------------------

Vitor* Vitor::Servidor() const {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha) return nullptr;
    for (Boss* corpo : batalha->Corpos()) {
        if (corpo && corpo != this && corpo->EhClone()) return dynamic_cast<Vitor*>(corpo);
    }
    return nullptr;
}

Vector2 Vitor::PosicaoDe(const int processo) const {
    if (processo == Mensagens::kCliente) return GetPosition();
    if (const Vitor* servidor = Servidor()) return servidor->GetPosition();
    // SEM SERVIDOR (quem comeca direto nesta fase e se divide no primeiro
    // quadro, ou um corpo que a batalha ja levou), o espelho do cliente.
    return Vector2(Campo().x + Campo().w - (GetPosition().x - Campo().x), GetPosition().y);
}

Mensagens::Rodada Vitor::RodadaDasMensagens(const int k) const {
    return mPlanoDasMensagens.rodadas[static_cast<size_t>(((k % Mensagens::kRodadas) + Mensagens::kRodadas) %
                                                          Mensagens::kRodadas)];
}

void Vitor::MontarMensagens() {
    mNasMensagens = true;
    mRodadaDasMensagens = -1;

    // O QUE ESTA LUTA SORTEIA para a fase, e o degrau do modo.
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
    Restaurar(Mensagens::kFaseDasMensagens);
    mNasMensagens = false;
    mRodadaDasMensagens = -1;
    mAlvo.reset();
    if (Vitor* servidor = Servidor()) servidor->mAlvo.reset();
}

void Vitor::ComecarRodadaDasMensagens(const int k) {
    // O QUE AINDA CORRIA NO CANAL some: os dois mudam de lugar, e o canal
    // e outro.
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

    // OS DOIS VAO PARA O LUGAR DA RODADA: o original conduz os dois corpos.
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
            // O ENVELOPE SAI e corre pelo canal; quem o move e MoverEnvelopes.
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
            // RECEIVE: a mensagem processada vai para o aluno - uma onde ele
            // esta, a seguinte onde ele estara (como o roteador da fase 2):
            // quem fica parado leva uma, quem anda sempre igual leva a outra.
            const Vector2 de = PosicaoDe(e.de);
            const int leque = mDegrauDasMensagens.lequeDoRecebido;
            DispararEm(Mensagens::kGatilhoDoProcessado, de, leque,
                       AnguloAdiante(de, processado, Tcp::Antecipacao(mRecebidas++, false)),
                       leque > 1 ? std::optional<float>(mDegrauDasMensagens.aberturaDoRecebido) : std::nullopt, rapidez);
            break;
        }
        case Mensagens::Acontece::Encontro:
            // O RENDEZVOUS: os dois andam juntos, e atiram juntos.
            anel(PosicaoDe(Mensagens::kCliente));
            anel(PosicaoDe(Mensagens::kServidor));
            break;
        case Mensagens::Acontece::Resultado: {
            // O RETORNO DA RPC: um leque com tantos quanto a soma, o do meio no
            // aluno (as somas sao impares).
            const Vector2 de = PosicaoDe(Mensagens::kCliente);
            DispararEm(Mensagens::kGatilhoDoProcessado, de, e.valor,
                       AnguloAdiante(de, processado, 1.0f),
                       kAberturaPorDado * static_cast<float>(e.valor - 1), rapidez);
            mEsperandoRpc = false;
            break;
        }
        case Mensagens::Acontece::Estoura:
            // O TIMEOUT: os envelopes parados na porta estouram.
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
        // CHEGOU: entra na caixa postal de quem recebe - o roteiro conta.
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
    // NO FORK AINDA NAO HA DOIS: os nomes e o canal so aparecem quando a nuvem
    // se desfaz. (O fork comeca neste mesmo quadro, em AtualizarNuvens.)
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

    // O PLACAR: a caixa do cliente na assincrona; o cliente bloqueado enquanto o
    // envelope dele corre, na sincrona; a chamada da rpc; o impasse.
    const Mensagens::Rodada rodada = RodadaDasMensagens(k);
    bool bloqueado = false;
    if (rodada == Mensagens::Rodada::Sincrona) {
        for (const Envelope& e : mEnvelopes) bloqueado = bloqueado || e.de == Mensagens::kCliente;
    } else if (rodada == Mensagens::Rodada::Rpc) {
        bloqueado = mEsperandoRpc;
    }
    Anunciar(Mensagens::kFaseDasMensagens,
             Mensagens::Placar(rodada, Mensagens::NaCaixa(mRoteiroDasMensagens, Mensagens::kCliente, u),
                               mChamadaDaRpc, bloqueado, mEstourou, mPlanoDasMensagens));
}
