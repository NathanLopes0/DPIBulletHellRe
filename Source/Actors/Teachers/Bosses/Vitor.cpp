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
    if (mLinhas) mLinhas->Limpar();
}

SDL_FRect Vitor::Campo() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene)) return battle->GetPlayfieldBounds();
    return SDL_FRect{0.0f, 0.0f, 1200.0f, 746.0f};
}

std::vector<BossProjectile*> Vitor::DispararEm(const char* gatilho, const Vector2 onde, const int quantos,
                                              const std::optional<float> centro,
                                              const std::optional<float> abertura) {
    mDisparandoEm = true;
    mOndeDisparar = onde;
    mQuantos = quantos;
    mCentroEm = centro;
    mAberturaEm = abertura;
    mCriados.clear();
    Disparar(gatilho);
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
            else Lancar(p, 90.0f, Semaforo::kVelocidadeDaRajada);
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
    mColunas = Semaforo::Colunas(aluno.x - campo.x, campo.w);
    for (float& x : mColunas) x += campo.x;
    const int n = static_cast<int>(mColunas.size());
    mEscalonador = Semaforo::Comecar(Semaforo::RodadaDaVez(k), n, Semaforo::MaisPerto(mColunas, aluno.x), Sorteio());

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
                                    Vector2(mColunas[static_cast<size_t>(thread)], campo.y + Semaforo::kNasce), 1);
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
            Lancar(p, Math::ToDegrees(Math::Atan2(d.y, d.x)), Semaforo::kVelocidadeDoAnel);
        } else {
            Lancar(p, Semaforo::AnguloNaRajada(static_cast<int>(j)), Semaforo::kVelocidadeDaRajada);
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
        if (corrida) Lancar(in.p, 90.0f + Semaforo::DesvioNaCorrida(Sorteio()), Semaforo::kVelocidadeDaInstrucao);
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
        if (th.mandadas >= Semaforo::kInstrucoesPorThread || u < th.proximaEm) continue;
        const bool livre = corrida || Semaforo::Dentro(mEscalonador, i) || Semaforo::Passou(mEscalonador, i);
        const int presas = static_cast<int>(th.descendo.size() + th.paradas.size());
        if (!livre && presas >= Semaforo::kMaiorPilha) continue;
        Mandar(i);
        ++th.mandadas;
        th.proximaEm = u + Semaforo::kEntreInstrucoes;
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
        const float x = c + (c < campo.x + campo.w / 2.0f ? kAoLadoDaColuna : -kAoLadoDaColuna);
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

    AtualizarNuvens(deltaTime);
    AtualizarSemaforos();
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
    mDisparandoNuvem = true;
    Disparar(gatilho);
    mDisparandoNuvem = false;
}

void Vitor::AtualizarNuvens(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasNuvens = fsm && fsm->GetStateName() == kFaseDasNuvens && !EhClone();

    if (!nasNuvens) {
        if (mNasNuvens) {
            // A FASE ACABOU. Se a seguinte e o fork, as nuvens sao a porta dele;
            // se nao, elas se desfazem. O placar volta ao nome.
            if (!mNomeDaFaseDasNuvens.empty()) RegistrarAnuncio(kFaseDasNuvens, mNomeDaFaseDasNuvens);
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
        if (mNomeDaFaseDasNuvens.empty()) mNomeDaFaseDasNuvens = AnuncioDaFase(kFaseDasNuvens);
    }

    mTempoNasNuvens += deltaTime;
    if (!mNuvensFormadas) {
        if (mTempoNasNuvens < kFormaAsNuvens) return;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mNuvensFormadas = true;
        mChuva.t = 0.0f;
        return;     // as nuvens recem-disparadas so entram no campo no quadro seguinte
    }

    // A NUVEM FORA DO AR: o Ponto Extra levou as duas. O placar avisa, e elas
    // voltam kVoltaDasNuvens depois, com o ciclo do comeco.
    if (Nuvens().empty()) {
        mSemNuvensHa += deltaTime;
        if (AnuncioDaFase(kFaseDasNuvens) != kForaDoAr) RegistrarAnuncio(kFaseDasNuvens, kForaDoAr);
        if (mSemNuvensHa < Nuvem::kVoltaDasNuvens) return;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mChuva.t = 0.0f;
        mSemNuvensHa = 0.0f;
        return;
    }
    mSemNuvensHa = 0.0f;

    // O CICLO: processa, e no fim do processamento chove.
    if (Nuvem::Avancar(mChuva, deltaTime)) DispararNuvem(Nuvem::kGatilhoDaChuva);

    const bool processando = Nuvem::Processando(mChuva);
    const char* texto = processando ? kProcessando : kEnviando;
    if (AnuncioDaFase(kFaseDasNuvens) != texto) RegistrarAnuncio(kFaseDasNuvens, texto);

    const char* cara = processando ? Nuvem::kCaraProcessando : Nuvem::kCaraOciosa;
    for (BossProjectile* nuvem : Nuvens()) {
        if (auto* anim = nuvem->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }
}

void Vitor::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    // O ATAQUE DE GATILHO nasce onde o professor disse, no numero que ele
    // pediu - e so ele: os de relogio das outras fases seguem abaixo.
    if (mDisparandoEm) {
        params.firePosition = mOndeDisparar;
        if (mQuantos > 0) params.numProjectiles = mQuantos;
        if (mCentroEm) params.centralAngle = *mCentroEm;
        if (mAberturaEm) params.angle = *mAberturaEm;
        return;
    }

    // AS NUVENS NAO SAO O ANEL: nem a formacao delas nem a chuva mexem na
    // fresta. Sem isto cada chuva empurraria a fresta um passo, e a sequencia
    // de portas do anel mudaria por causa de uma coisa que nao e anel.
    if (mDisparandoNuvem) return;

    // A FRESTA ANDA, MAS SO PELA METADE DE BAIXO DA TELA.
    //
    // E a unica coisa do Vitor que nao cabe em dados: "para onde a abertura
    // aponta DESTA vez" depende da rajada anterior, e fases.json nao tem
    // memoria. O TAMANHO da fresta continua no arquivo ("angulo": 330 deixa 30
    // graus abertos); daqui sai so a DIRECAO.
    //
    // O clone da fase 3 tambem passa por aqui: ele roda os ataques de
    // StateTwo, e e outro Vitor, com a propria fresta comecando do meio.
    if (stateName == "StateTwo") {
        const float fresta = kFrestaMaisADireita + mDesvioDaFresta;

        // O leque e desenhado EM TORNO de centralAngle, entao a abertura fica
        // oposta a ele: somar 180 poe a fresta onde a variavel diz. E escrever
        // o valor inteiro, e nao somar sobre o disparo anterior - params e o
        // mesmo objeto a cada disparo (ver Boss.h).
        params.centralAngle = fresta + 180.0f;

        mDesvioDaFresta = std::fmod(mDesvioDaFresta + kPassoDaFresta, kJanelaDaFresta);
    }
}
