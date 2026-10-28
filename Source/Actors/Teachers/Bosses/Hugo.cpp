//
// O chefe do TCC - Trabalho de Conclusao de Curso.
//

#include "Hugo.h"

#include <algorithm>
#include <cmath>

#include "../../../CaminhosArquivo.h"
#include "../../../Game.h"
#include "../../../MateriasArquivo.h"
#include "../../../Attacks/Flex.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Font.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../../Components/DrawComponents/DrawTextComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/DrawComponents/DrawSpriteComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Orientador/Orientador.h"
#include "../../Player/Player.h"
#include "../../Player/PlayerProjectile.h"
#include "../BossFactory/IBossFactory.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// O tiro guardado ainda e o mesmo se a idade dele bate com a esperada: o
    /// pool devolve o projetil com o relogio zerado.
    constexpr float kFolgaDaIdade = 0.1f;

    constexpr int kOrdemDasLinhas = 84;
    constexpr int kOrdemDoCadeado = 101;
    constexpr SDL_Color kGuia{150, 140, 190, 60};
    constexpr SDL_Color kBarra{196, 170, 230, 150};
    constexpr SDL_Color kSemana{150, 140, 190, 120};
    constexpr SDL_Color kHoje{235, 75, 75, 230};

    /// Na proposta, ele fica em cima da barra que vai cair.
    constexpr float kYNaProposta = 80.0f;

    /// No desenvolvimento, ele para para escrever cada capitulo - o capitulo
    /// sai dele parado -, e depois volta a varrer o alto.
    constexpr float kEscrevendo = 0.6f;
    constexpr float kVarreduraDoTema = 120.0f;
    constexpr float kVarreduraDoTemaNoDificil = 120.0f;

    /// A banca, sentada nos cantos de cima.
    constexpr float kXDaBanca = 190.0f;
    constexpr float kYDaBanca = 120.0f;
    constexpr float kEscalaDaBanca = 0.8f;
    constexpr int kOrdemDaBanca = 88;
}

Hugo::Hugo(Scene* scene) : Boss(scene) {
}

// A FONTE E DELE, e os rotulos a usam: o destrutor nao mexe nos atores (a cena
// os apaga junto), so precisa existir onde Font e um tipo completo.
Hugo::~Hugo() = default;

// ---------------------------------------------------------------------------

void Hugo::SortearALuta() {
    mLutaSorteada = true;
    // O TEMA E A MATERIA DO ORIENTADOR; sem orientador (uma batalha medida sem
    // ele), o Hugo atira as paginas dele.
    if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetOrientador()) {
        mTema = batalha->GetOrientador()->Materia();
        mOrientador = batalha->GetOrientador()->Professor();
    }
    Variacao::Sorteio banca = SorteioDaFase(Tcc::kFaseDaDefesa);
    mBanca = Tcc::Banca(mTema, banca);
    mSorteioDoCronograma = SorteioDaFase(Cronograma::kFaseDaProposta);
    mSorteioDosPadroes = SorteioDaFase(Tcc::kFaseDoTema);
    mDegrauDoCronograma = Cronograma::DegrauDo(NivelDeDificuldade());
    mDegrau = Tcc::DegrauDo(NivelDeDificuldade());
    mSorteioDoObjetivo = SorteioDaFase("objetivo");
    mFonte = std::make_unique<Font>();
    mFonte->Load(Caminhos::Asset("Fonts/Carlito-Regular.ttf"));

    // O DESENHO DO ESCUDO, por um ator a parte, em coordenadas de tela.
    auto ator = std::make_unique<Actor>(mScene);
    mEscudoLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas + 2);
    // O CADEADO vai POR CIMA do cartao dele (que e desenhado a 100).
    mCadeado = ator->AddComponent<DrawLinhasComponent>(kOrdemDoCadeado);
    mScene->AddActor(std::move(ator));
}

void Hugo::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    if (EhClone() || !mScene || !mScene->GetGame()) return;

    mRelogio += deltaTime;
    // COMO O ALUNO ANDA, suavizado em uns 0,2 s: e o que a mira adianta.
    const Vector2 aluno = PosicaoDoAluno();
    if (mViuOAluno && deltaTime > 0.0f) {
        const float peso = std::min(1.0f, deltaTime / 0.2f);
        mAndando = mAndando + ((aluno - mAlunoAntes) * (1.0f / deltaTime) - mAndando) * peso;
    }
    mAlunoAntes = aluno;
    mViuOAluno = true;

    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm) return;
    const std::string fase = fsm->GetStateName();
    if (fase != mFase) TrocarDeFase(fase);
    const float agora = fsm->GetStateTime();

    // NO HLT, o que tem hora espera, e so o objetivo continua - o aluno ainda
    // pode cumpri-lo, e o escudo e a janela estao parados com o resto.
    if (Congelado()) {
        Adiar(deltaTime);
        AtualizarObjetivo(deltaTime);
        return;
    }

    if (fase == Cronograma::kFaseDaProposta) AtualizarCronograma();
    else if (fase == Tcc::kFaseDoTema) {
        if (agora - mUltimoTema >= kEscrevendo) mTempoVarrendo += deltaTime;
        AtualizarTema(agora);
    }
    else if (fase == Tcc::kFaseDaDefesa) { AtualizarDefesa(agora); Anotar(agora); }
    else if (fase == Tcc::kFaseDasCorrecoes) { AtualizarCorrecoes(agora); Anotar(agora); }

    AtualizarObjetivo(deltaTime);
    AtualizarPadroes(deltaTime);
}

void Hugo::TrocarDeFase(const std::string& nova) {
    // SAINDO: o que era da fase some, e o placar volta ao nome dela.
    if (mFase == Cronograma::kFaseDaProposta) {
        MatarPosto(Cronograma::kPostoDoCronograma);
        mBarras.clear();
    }
    if (mFase == Tcc::kFaseDaDefesa) DispensarABanca();
    MatarAlvos();
    if (mLinhas) mLinhas->Limpar();
    mExecucoes.clear();
    mAlvo.reset();
    if (const auto it = mAnunciosDoArquivo.find(mFase); it != mAnunciosDoArquivo.end()) {
        RegistrarAnuncio(mFase, it->second);
    }

    mFase = nova;
    mProximaAnotacao = Tcc::kPrimeiro;
    mPrincipal.clear();
    mPlacarFeito.clear();

    // CADA ETAPA COMECA BLINDADA, e o primeiro objetivo vem logo.
    mEtapa = Objetivo::DaFase(nova);
    mEscudo = mEtapa != Objetivo::Etapa::Nenhuma;
    mProximoObjetivo = mRelogio + Tcc::kPrimeiro;
    mFechouEm = mRelogio;
    // AS DUAS MANEIRAS SE ALTERNAM, e qual vem primeiro e da luta.
    mVezDoObjetivo = 0;
    mPrimeiraManeira = mSorteioDoObjetivo.Inteiro(0, 1);
    if (!mAnunciosDoArquivo.count(nova)) mAnunciosDoArquivo[nova] = AnuncioDaFase(nova);

    // ENTRANDO.
    if (nova == Cronograma::kFaseDaProposta) {
        CriarLinhas();
        NovoCronograma();
    } else if (nova == Tcc::kFaseDoTema) {
        mProximoTema = Tcc::kPrimeiro;
        mCapitulo = 0;
        mTempoVarrendo = 0.0f;
        mUltimoTema = -10.0f;
        mRascunhos = 1000;   // nenhum capitulo ainda
        mXDoComeco = GetPosition().x;
        Anunciar(nova, mTema == Golpe::Materia::Generico
                       ? std::string("TCC: tema livre\nsem orientador")
                       : "TCC em " + Tcc::Area(mTema) + "\norientador: " + mOrientador);
    } else if (nova == Tcc::kFaseDaDefesa) {
        ChamarABanca();
        mProximaPergunta = Tcc::kPrimeiro;
        mPerguntas = 0;
        Anunciar(nova, "Defesa\nbanca: " + Tcc::Professor(mBanca[0]) + " e " + Tcc::Professor(mBanca[1]));
    } else if (nova == Tcc::kFaseDasCorrecoes) {
        mProximaCorrecao = Tcc::kPrimeiro;
        mCorrecoes = 0;
    }
}

// ---------------------------------------------------------------------------

Vector2 Hugo::PosicaoDoAluno() const {
    if (const auto* battle = dynamic_cast<Battle*>(mScene); battle && battle->Alvo()) {
        return battle->Alvo()->GetPosition();
    }
    return GetPosition() + GetDirectionToPlayer() * GetDistanceToPlayer();
}

Vector2 Hugo::MiraDe(const Vector2& de, const float velocidade, const float adiantamento) const {
    return Tcc::Mira(de, velocidade, PosicaoDoAluno(), mAndando, adiantamento);
}

float Hugo::Largura() const {
    return static_cast<float>(mScene->GetGame()->GetWindowWidth());
}

bool Hugo::Vivo(const BossProjectile* p, const float idade) {
    return p && p->GetState() == ActorState::Active && std::fabs(p->TempoDeVida() - idade) <= kFolgaDaIdade;
}

void Hugo::MatarPosto(const std::string& posto) {
    if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetProjectileManager()) {
        for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
            if (p && p->GetState() == ActorState::Active && p->Posto() == posto) p->Kill();
        }
    }
}

void Hugo::Anunciar(const std::string& fase, const std::string& texto) {
    (void)fase;
    mPrincipal = texto;
    RefazerPlacar();
}

void Hugo::CriarLinhas() {
    if (mLinhas || !mScene) return;
    auto ator = std::make_unique<Actor>(mScene);
    mLinhas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasLinhas);
    mScene->AddActor(std::move(ator));
}

std::vector<BossProjectile*> Hugo::DispararDe(const char* gatilho, const Vector2 onde, const float rumo) {
    mDisparandoDe = true;
    mOndeDisparar = onde;
    mRumoDoDisparo = rumo;
    mDisparados.clear();
    Disparar(gatilho);
    mDisparandoDe = false;
    return mDisparados;
}

void Hugo::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoDe) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mDisparados.push_back(b);
    }
}

void Hugo::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
    if (mDisparandoDe) {
        params.firePosition = mOndeDisparar;
        params.centralAngle = mRumoDoDisparo;
        return;
    }
    // O QUE VAI PELO RELOGIO (as paginas, a monografia, as perguntas) SAI
    // MIRADO NO ALUNO: a banca nao pergunta para o teto.
    const Vector2 d = PosicaoDoAluno() - GetPosition();
    params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
}

// ---------------------------------------------------------------------------
// OS PADROES DAS MATERIAS
// ---------------------------------------------------------------------------

Vector2 Hugo::PosicaoDa(const Fonte f) const {
    switch (f) {
        case Fonte::Banca0: return Vector2(kXDaBanca, kYDaBanca);
        case Fonte::Banca1: return Vector2(Largura() - kXDaBanca, kYDaBanca);
        default:            return GetPosition();
    }
}

float Hugo::Adiantamento(const Fonte f) const {
    // A BANCA, DE LONGE E DE LADO, ADIANTA MENOS: mirando onde o aluno vai estar
    // do canto da tela, ela pegava justamente quem desvia andando.
    return f == Fonte::Hugo ? mDegrau.adiantamento : mDegrau.adiantamento * Tcc::kAdiantamentoDaBanca;
}

void Hugo::Lancar(const Golpe::Materia m, const Fonte f) {
    Execucao e;
    e.materia = m;
    e.fonte = f;
    e.rodada = Tcc::Padrao(m, mVezes++, mSorteioDosPadroes, NivelDeDificuldade());
    e.inicio = mRelogio;
    mExecucoes.push_back(std::move(e));
}

void Hugo::Soltar(const Golpe::Materia m, const Fonte f, const Golpe::Tiro& tiro,
                  const Vector2& de, const Vector2& direcao) {
    const float rumo = Math::ToDegrees(Math::Atan2(direcao.y, direcao.x));
    for (BossProjectile* p : DispararDe(Tcc::kGatilhoDaQuestao, de, rumo)) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Tcc::Animacao(m));
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(tiro.espera > 0.0f ? Vector2::Zero : direcao * tiro.velocidade);
        }
        if (tiro.Guiado()) mGuiados.push_back({p, mRelogio, m, f, Golpe::Comecar(tiro, direcao)});
    }
}

void Hugo::AtualizarPadroes(const float deltaTime) {

    // CADA TIRO SAI NA HORA DELE, de onde a fonte esta agora.
    for (auto it = mExecucoes.begin(); it != mExecucoes.end();) {
        const auto& tiros = it->rodada.tiros;
        while (it->proximo < tiros.size() && mRelogio - it->inicio >= tiros[it->proximo].quando) {
            const Golpe::Tiro& tiro = tiros[it->proximo++];
            const Golpe::Saida saida = Golpe::Sair(tiro, PosicaoDa(it->fonte),
                                                   MiraDe(PosicaoDa(it->fonte), tiro.velocidade, Adiantamento(it->fonte)));
            Soltar(it->materia, it->fonte, tiro, saida.de, saida.direcao);
        }
        it = (it->proximo >= tiros.size()) ? mExecucoes.erase(it) : it + 1;
    }

    // OS GUIADOS: a espera, a curva, a volta, a onda, a divisao.
    struct Filho {
        Golpe::Materia materia;
        Fonte fonte;
        Golpe::Tiro tiro;
        Vector2 de;
        Vector2 direcao;
    };
    std::vector<Filho> filhos;
    for (auto it = mGuiados.begin(); it != mGuiados.end();) {
        BossProjectile* p = it->p;
        auto* rb = Vivo(p, mRelogio - it->nasceu) ? p->GetComponent<RigidBodyComponent>() : nullptr;
        if (!rb) {
            it = mGuiados.erase(it);
            continue;
        }
        // O QUE CURVA PROCURA O ALUNO ONDE ELE ESTA: adiantar uma curva a faz
        // passar do ponto quando ele para.
        const float adianta = it->voo.tiro.curva > 0.0f ? 0.0f : Adiantamento(it->fonte);
        const Golpe::Passo passo = Golpe::Avancar(it->voo, p->GetPosition(), rb->GetVelocity(), PosicaoDa(it->fonte),
                                                  MiraDe(p->GetPosition(), it->voo.tiro.velocidade, adianta), deltaTime);
        p->SetPosition(passo.posicao);
        rb->SetVelocity(passo.velocidade);
        if (passo.divide) {
            for (const Golpe::Filho& f : Golpe::Filhos(it->voo, passo.posicao, passo.velocidade,
                                                       MiraDe(passo.posicao, it->voo.tiro.velocidade,
                                                              Adiantamento(it->fonte)))) {
                filhos.push_back({it->materia, it->fonte, f.tiro, passo.posicao, f.direcao});
            }
            p->Kill();
            it = mGuiados.erase(it);
            continue;
        }
        ++it;
    }
    // DEPOIS DO LACO: soltar agora mexeria na lista que ele percorre.
    for (const Filho& f : filhos) Soltar(f.materia, f.fonte, f.tiro, f.de, f.direcao);
}

// ---------------------------------------------------------------------------
// FASE 1 - A PROPOSTA: O CRONOGRAMA
// ---------------------------------------------------------------------------

void Hugo::NovoCronograma() {
    MatarPosto(Cronograma::kPostoDoCronograma);
    mBarras.clear();
    for (const Cronograma::Tarefa& t : Cronograma::Gerar(mSorteioDoCronograma, Largura(), mDegrauDoCronograma)) {
        Barra b;
        b.tarefa = t;
        b.nasceu = mRelogio;
        for (const float x : Cronograma::Paginas(t, mDegrauDoCronograma)) {
            for (BossProjectile* p : DispararDe(Cronograma::kGatilhoDaTarefa, Vector2(x, Cronograma::YDaLinha(t.linha)), 90.0f)) {
                b.paginas.push_back(p);
            }
        }
        mBarras.push_back(std::move(b));
    }
    mInicioDoCronograma = mRelogio;
}

void Hugo::AtualizarCronograma() {
    const float largura = Largura();
    const float t = mRelogio - mInicioDoCronograma - Cronograma::kPlanejando;
    const float hoje = Cronograma::XDoHoje(t, largura, mDegrauDoCronograma);

    // O PRAZO VENCEU: a barra inteira cai.
    for (Barra& b : mBarras) {
        if (b.caiu || t < 0.0f || hoje < b.tarefa.ate) continue;
        b.caiu = true;
        for (BossProjectile* p : b.paginas) {
            if (!Vivo(p, mRelogio - b.nasceu) || p->Posto() != Cronograma::kPostoDoCronograma) continue;
            p->DefinirPosto("");
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, mDegrauDoCronograma.queda));
        }
        Anunciar(Cronograma::kFaseDaProposta, Cronograma::Placar(b.tarefa, largura));
    }

    // O GRAFICO ACABOU: outro.
    if (t >= mDegrauDoCronograma.varredura + Cronograma::kEntreCronogramas) {
        NovoCronograma();
        return;
    }

    // ELE FICA EM CIMA DA BARRA QUE VAI CAIR.
    mAlvo = Vector2(largura / 2.0f, kYNaProposta);
    for (const Barra& b : mBarras) {
        if (b.caiu) continue;
        mAlvo = Vector2((b.tarefa.de + b.tarefa.ate) / 2.0f, kYNaProposta);
        break;
    }
    DesenharCronograma(t < 0.0f ? Cronograma::kMargem : hoje);
}

void Hugo::DesenharCronograma(const float hoje) {
    if (!mLinhas) return;
    mLinhas->Limpar();
    const float largura = Largura();
    const float esquerda = Cronograma::kMargem, direita = largura - Cronograma::kMargem;
    const float topo = Cronograma::YDaLinha(0) - 30.0f;
    const float base = Cronograma::YDaLinha(Cronograma::kLinhas - 1) + 30.0f;

    // O GRAFICO: as linhas das tarefas e as semanas.
    for (int l = 0; l < Cronograma::kLinhas; ++l) {
        const float y = Cronograma::YDaLinha(l) + Cronograma::kEntreLinhas / 2.0f;
        mLinhas->Acrescentar(Vector2(esquerda, y), Vector2(direita, y), kGuia);
    }
    for (int s = 0; s <= Cronograma::kSemanas; ++s) {
        const float x = esquerda + (direita - esquerda) * static_cast<float>(s) / Cronograma::kSemanas;
        mLinhas->Acrescentar(Vector2(x, topo), Vector2(x, topo + 8.0f), kSemana);
    }
    // AS BARRAS QUE AINDA NAO CAIRAM, com o contorno.
    for (const Barra& b : mBarras) {
        if (b.caiu) continue;
        const float y = Cronograma::YDaLinha(b.tarefa.linha);
        const Vector2 a(b.tarefa.de - 16.0f, y - 18.0f), c(b.tarefa.ate + 16.0f, y + 18.0f);
        mLinhas->Acrescentar(a, Vector2(c.x, a.y), kBarra);
        mLinhas->Acrescentar(Vector2(c.x, a.y), c, kBarra);
        mLinhas->Acrescentar(c, Vector2(a.x, c.y), kBarra);
        mLinhas->Acrescentar(Vector2(a.x, c.y), a, kBarra);
    }
    // O HOJE.
    mLinhas->Acrescentar(Vector2(hoje, topo), Vector2(hoje, base), kHoje);
}

// ---------------------------------------------------------------------------
// FASE 2 - O DESENVOLVIMENTO
// ---------------------------------------------------------------------------

void Hugo::AtualizarTema(const float agora) {
    // ELE VARRE O ALTO, e para para escrever cada capitulo.
    mAlvo = Vector2(Flex::XDaVarredura(mTempoVarrendo, mXDoComeco, NoNivel(kVarreduraDoTema, kVarreduraDoTemaNoDificil),
                                       Largura()),
                    Flex::kYDaVarredura);
    // O RASCUNHO escorre embaixo dele enquanto escreve.
    while (mRascunhos < mDegrau.rascunhos && agora - mUltimoTema >= static_cast<float>(mRascunhos) * Tcc::kEntreRascunhos) {
        DispararDe(Tcc::kGatilhoDoRascunho, GetPosition() + Vector2(0.0f, 50.0f), 90.0f);
        ++mRascunhos;
    }
    if (agora < mProximoTema) return;
    mProximoTema += mDegrau.entreTemas;
    mUltimoTema = agora;
    mRascunhos = 0;
    ++mCapitulo;
    Lancar(mTema, Fonte::Hugo);
    Anunciar(Tcc::kFaseDoTema, "Cap. " + std::to_string(mCapitulo) + ": " + Tcc::Capitulo(mTema));
}

// ---------------------------------------------------------------------------
// FASE 3 - A DEFESA
// ---------------------------------------------------------------------------

void Hugo::ChamarABanca() {
    DispensarABanca();
    const Materias::Lista& materias = Materias::Carregadas();
    for (int k = 0; k < 2; ++k) {
        // O RETRATO DE CADA UM E O CARTAO DE CHEFE DELE, como o do orientador.
        std::string retrato;
        for (int i = 0; i < materias.Quantas(); ++i) {
            const Materias::Materia* m = materias.Por(i);
            if (!m || Golpe::DoProfessor(m->professor) != mBanca[static_cast<size_t>(k)]) continue;
            if (IBossFactory* fabrica = mScene->GetGame()->GetFactory(static_cast<size_t>(i))) retrato = fabrica->Retrato();
        }
        if (retrato.empty()) continue;
        auto cadeira = std::make_unique<Actor>(mScene);
        cadeira->SetScale(kEscalaDaBanca);
        cadeira->AddComponent<DrawSpriteComponent>(Caminhos::Asset(retrato + ".png"), kOrdemDaBanca);
        cadeira->SetPosition(PosicaoDa(k == 0 ? Fonte::Banca0 : Fonte::Banca1));
        mCadeiras[static_cast<size_t>(k)] = mScene->AddActor(std::move(cadeira));
    }
}

void Hugo::DispensarABanca() {
    for (Actor*& c : mCadeiras) {
        if (c) c->SetState(ActorState::Destroy);
        c = nullptr;
    }
}

void Hugo::AtualizarDefesa(const float agora) {
    if (agora < mProximaPergunta) return;
    mProximaPergunta += mDegrau.entrePerguntas;
    // A BANCA SE ALTERNA: cada pergunta e a materia de quem pergunta.
    // A PERGUNTA EM SI e o objetivo (NovoObjetivo): o placar e dela.
    const int k = mPerguntas++ % 2;
    const Golpe::Materia quem = mBanca[static_cast<size_t>(k)];
    Lancar(quem, k == 0 ? Fonte::Banca0 : Fonte::Banca1);
}

void Hugo::Anotar(const float agora) {
    while (agora >= mProximaAnotacao) {
        mProximaAnotacao += mDegrau.entreAnotacoes;
        DispararDe(Tcc::kGatilhoDoRascunho, GetPosition() + Vector2(0.0f, 50.0f), 90.0f);
    }
}

// ---------------------------------------------------------------------------
// FINAL - AS CORRECOES
// ---------------------------------------------------------------------------

void Hugo::AtualizarCorrecoes(const float agora) {
    if (agora < mProximaCorrecao) return;
    mProximaCorrecao += mDegrau.entreCorrecoes;
    // O TEMA E AS DUAS PERGUNTAS DA BANCA, em volta.
    const Golpe::Materia vez[] = {mTema, mBanca[0], mBanca[1]};
    const Golpe::Materia m = vez[mCorrecoes++ % 3];
    Lancar(m, Fonte::Hugo);
    Anunciar(Tcc::kFaseDasCorrecoes,
             "Correção:\n" + (m == mTema ? Tcc::Capitulo(m) : Tcc::Pergunta(m)));
}

// ---------------------------------------------------------------------------
// O ESCUDO E O OBJETIVO DA ETAPA
// ---------------------------------------------------------------------------

void Hugo::NovoObjetivo() {
    MatarAlvos();
    mTipo = Objetivo::DaVez(mEtapa, mVezDoObjetivo++, mPrimeiraManeira);
    mProgresso = 0.0f;
    mObjetivoEmCurso = true;
    const Dificuldade::Nivel nivel = NivelDeDificuldade();

    // UMA COISA QUE O ALUNO PEGA: enfeite, sem contorno, que o tiro atravessa.
    const auto pegavel = [this](const char* animacao, const Vector2& onde, const int k, const bool certo) {
        for (BossProjectile* p : DispararDe(Objetivo::kGatilhoDoAlvo, onde, 90.0f)) {
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(animacao);
            p->DefinirEnfeite(true);
            if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            AlvoEmCampo e;
            e.p = p;
            e.nasceu = mRelogio;
            e.alvo = {animacao, onde, certo};
            e.k = k;
            mAlvos.push_back(e);
        }
    };
    switch (mTipo) {
        case Objetivo::Tipo::Reuniao:
            mReuniao = Objetivo::LugarDaReuniao(mSorteioDoObjetivo, Largura(), PosicaoDoAluno().x);
            mNaReuniao = 0.0f;
            if (auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetOrientador()) {
                batalha->GetOrientador()->MarcarReuniao(mReuniao);
            }
            break;
        case Objetivo::Tipo::Referencias: {
            int k = 0;
            for (const Vector2& livro : Objetivo::Referencias(mSorteioDoObjetivo, Largura(), nivel)) {
                pegavel("Referencia", livro, k++, true);
            }
            break;
        }
        case Objetivo::Tipo::Arguicao:
            mRaspou = 0;
            mRaspados.clear();
            break;
        case Objetivo::Tipo::Entrega: {
            const Objetivo::Entrega e = Objetivo::Entregar(mSorteioDoObjetivo, Largura());
            mSecretaria = e.secretaria;
            mCarregando = nullptr;
            pegavel("Versao", e.versao, 0, true);
            pegavel("Caixa", e.secretaria, 1, false);
            break;
        }
        default:
            break;
    }
    if (mTipo != Objetivo::Tipo::Derrubar) {
        mPlacas = Objetivo::Placas(mTipo, nivel, 0);
        RefazerPlacar();
        return;
    }

    if (mEtapa == Objetivo::Etapa::Defesa) {
        // A BANCA SE ALTERNA nas perguntas, como nos ataques.
        mQuemPergunta = mBanca[static_cast<size_t>(mQuestoes++ % 2)];
        mQuestao = Objetivo::Perguntar(mQuemPergunta, mSorteioDoObjetivo);
    }
    ApagarRotulos();
    const auto alvos = Objetivo::Gerar(mEtapa, mSorteioDoObjetivo, Largura(), NivelDeDificuldade(), mQuestao.certa);
    for (size_t k = 0; k < alvos.size(); ++k) {
        const Objetivo::Alvo& a = alvos[k];
        const char* gatilho = a.animacao == "Bug" ? Objetivo::kGatilhoDoBug : Objetivo::kGatilhoDoAlvo;
        for (BossProjectile* p : DispararDe(gatilho, a.lugar, 90.0f)) {
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(a.animacao);
            AlvoEmCampo e;
            e.p = p;
            e.nasceu = mRelogio;
            e.alvo = a;
            e.paragem = Objetivo::Paragem(mSorteioDoObjetivo, Largura());
            e.k = static_cast<int>(k);
            mAlvos.push_back(e);
        }
    }
    if (mEtapa == Objetivo::Etapa::Defesa) Rotular();
    int certos = 0;
    for (const Objetivo::Alvo& a : alvos) certos += a.certo ? 1 : 0;
    mPlacas = Objetivo::Placas(mTipo, nivel, certos);
    RefazerPlacar();
}

void Hugo::Rotular() {
    ApagarRotulos();
    if (!mFonte) return;
    constexpr int kLargura = 210, kAltura = 26;
    for (int i = 0; i < 3; ++i) {
        auto rotulo = std::make_unique<Actor>(mScene);
        rotulo->AddComponent<DrawCaixaComponent>(kLargura, kAltura, SDL_Color{110, 170, 240, 200},
                                                SDL_Color{12, 20, 40, 190}, 1, kOrdemDasLinhas + 3);
        auto* texto = rotulo->AddComponent<DrawTextComponent>(
            std::string(1, static_cast<char>('A' + i)) + ") " + mQuestao.respostas[static_cast<size_t>(i)],
            mFonte.get(), kLargura - 10, kAltura - 4, 18, kOrdemDasLinhas + 4);
        texto->SetAjustarAoTexto(true);
        rotulo->SetPosition(Vector2(-500.0f, -500.0f));
        mRotulos[static_cast<size_t>(i)] = mScene->AddActor(std::move(rotulo));
    }
}

void Hugo::ApagarRotulos() {
    for (Actor*& r : mRotulos) {
        if (r) r->SetState(ActorState::Destroy);
        r = nullptr;
    }
}

void Hugo::MatarAlvos() {
    for (const AlvoEmCampo& a : mAlvos) {
        if (!a.caiu && Vivo(a.p, mRelogio - a.nasceu)) a.p->Kill();
    }
    mAlvos.clear();
    ApagarRotulos();
    EncerrarAndar();
    mObjetivoEmCurso = false;
}

void Hugo::AbrirJanela() {
    MatarAlvos();
    if (auto* batalha = dynamic_cast<Battle*>(mScene)) batalha->Conquistar(Objetivo::PorJanela(NivelDeDificuldade()));
    mEscudo = false;
    mAbriuEm = mRelogio;
    mJanelaAte = mRelogio + Objetivo::Janela(mEtapa, NivelDeDificuldade());
    mProximaPaginaSolta = mRelogio + 0.4f;
    switch (mTipo) {
        case Objetivo::Tipo::Reuniao:     mPrincipal = "Reunião feita!"; break;
        case Objetivo::Tipo::Referencias: mPrincipal = "Referências em dia!"; break;
        case Objetivo::Tipo::Arguicao:    mPrincipal = "A banca gostou!"; break;
        case Objetivo::Tipo::Entrega:     mPrincipal = "Versão final entregue!"; break;
        default:
            switch (mEtapa) {
                case Objetivo::Etapa::Proposta:        mPrincipal = "Proposta aprovada!"; break;
                case Objetivo::Etapa::Desenvolvimento: mPrincipal = "Os testes passaram!"; break;
                case Objetivo::Etapa::Defesa:          mPrincipal = "Resposta aceita!"; break;
                case Objetivo::Etapa::Correcoes:       mPrincipal = "Correções feitas!"; break;
                default: break;
            }
    }
    RefazerPlacar();
}

bool Hugo::QuebrarEscudo() {
    if (!mEscudo || mEtapa == Objetivo::Etapa::Nenhuma) return false;
    AbrirJanela();
    mPrincipal = "A bala de prata!";
    RefazerPlacar();
    return true;
}

void Hugo::OnDerrubou(BossProjectile* qual) {
    if (mTipo != Objetivo::Tipo::Derrubar) return;
    for (AlvoEmCampo& a : mAlvos) {
        if (a.p != qual || a.caiu) continue;
        a.caiu = true;
        if (a.alvo.certo) {
            if (auto* batalha = dynamic_cast<Battle*>(mScene)) batalha->Conquistar(Objetivo::PorAlvo(NivelDeDificuldade()));
        }
        if (!a.alvo.certo) {
            // A RESPOSTA ERRADA: quem perguntou insiste, e a pergunta continua.
            if (a.k < 3 && mRotulos[static_cast<size_t>(a.k)]) {
                mRotulos[static_cast<size_t>(a.k)]->SetState(ActorState::Destroy);
                mRotulos[static_cast<size_t>(a.k)] = nullptr;
            }
            const Fonte f = mQuemPergunta == mBanca[0] ? Fonte::Banca0 : Fonte::Banca1;
            Lancar(mQuemPergunta, f);
            mPrincipal = "Resposta errada!";
            RefazerPlacar();
            return;
        }
        break;
    }
    // TODO ALVO CERTO NO CHAO: o objetivo esta cumprido.
    bool falta = false;
    int caidos = 0;
    for (const AlvoEmCampo& a : mAlvos) {
        falta = falta || (a.alvo.certo && !a.caiu);
        caidos += (a.alvo.certo && a.caiu) ? 1 : 0;
    }
    mProgresso = static_cast<float>(caidos) / static_cast<float>(std::max(1, mPlacas));
    if (!falta && !mAlvos.empty()) AbrirJanela();
    else RefazerPlacar();
}

void Hugo::AtualizarObjetivo(const float deltaTime) {
    if (mEtapa == Objetivo::Etapa::Nenhuma) {
        if (mEscudoLinhas) mEscudoLinhas->Limpar();
        return;
    }
    // A JANELA FECHA: o escudo volta, e logo vem outro objetivo.
    if (!mEscudo && mRelogio >= mJanelaAte) {
        mEscudo = true;
        mFechouEm = mRelogio;
        mProximoObjetivo = mRelogio + Objetivo::kRecomeco;
        mPrincipal = "O Hugo fechou de novo";
    }
    if (mEscudo && !mObjetivoEmCurso && mRelogio >= mProximoObjetivo) NovoObjetivo();

    // ABERTO, A MONOGRAFIA SE DESFAZ embaixo dele.
    while (!mEscudo && mRelogio >= mProximaPaginaSolta) {
        mProximaPaginaSolta += Objetivo::EntrePaginasSoltas(NivelDeDificuldade());
        DispararDe(Tcc::kGatilhoDoRascunho, GetPosition() + Vector2(0.0f, 50.0f), 90.0f);
    }

    if (mEscudo && mObjetivoEmCurso && mTipo != Objetivo::Tipo::Derrubar) AtualizarAndar(deltaTime);

    // OS ALVOS ANDAM: os bugs vao de parada em parada, o resto balanca.
    for (AlvoEmCampo& a : mAlvos) {
        if (mTipo != Objetivo::Tipo::Derrubar) break;
        if (a.caiu || !Vivo(a.p, mRelogio - a.nasceu)) continue;
        Vector2 onde;
        if (mEtapa == Objetivo::Etapa::Desenvolvimento) {
            const Vector2 falta = a.paragem - a.p->GetPosition();
            if (falta.Length() < 10.0f) a.paragem = Objetivo::Paragem(mSorteioDoObjetivo, Largura());
            const float passo = std::min(falta.Length(), Objetivo::kRapidezDoBug * deltaTime);
            onde = a.p->GetPosition() + (falta.Length() > 0.0f ? falta * (passo / falta.Length()) : Vector2::Zero);
        } else {
            onde = Objetivo::Balanco(mEtapa, a.alvo.lugar, a.k, mRelogio - a.nasceu);
        }
        a.p->SetPosition(onde);
        if (auto* rb = a.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        if (mEtapa == Objetivo::Etapa::Defesa && a.k < 3 && mRotulos[static_cast<size_t>(a.k)]) {
            mRotulos[static_cast<size_t>(a.k)]->SetPosition(onde + Vector2(0.0f, 40.0f));
        }
    }
    // UM ALVO QUE SUMIU SEM SER DERRUBADO (o pool o reciclou): sai da conta.
    bool sumiu = false;
    for (const AlvoEmCampo& a : mAlvos) sumiu = sumiu || (!a.caiu && !Vivo(a.p, mRelogio - a.nasceu));
    if (sumiu && mEscudo) {
        MatarAlvos();
        mProximoObjetivo = mRelogio + Objetivo::kRecomeco;
    }
    RefazerPlacar();
    DesenharEscudo();
}

void Hugo::Adiar(const float deltaTime) {
    mJanelaAte += deltaTime;
    mProximaPaginaSolta += deltaTime;
    mProximoObjetivo += deltaTime;
    mInicioDoCronograma += deltaTime;
    for (Execucao& e : mExecucoes) e.inicio += deltaTime;
}

std::vector<Vector2> Hugo::AlvosDestrutiveis() const {
    std::vector<Vector2> alvos;
    if (!mEscudo || mTipo != Objetivo::Tipo::Derrubar) return alvos;
    for (const AlvoEmCampo& a : mAlvos) {
        if (!a.caiu && a.alvo.certo && Vivo(a.p, mRelogio - a.nasceu)) alvos.push_back(a.p->GetPosition());
    }
    return alvos;
}

std::optional<Vector2> Hugo::ObjetivoAtual() const {
    if (!mEscudo || mTipo != Objetivo::Tipo::Derrubar) return std::nullopt;
    for (const AlvoEmCampo& a : mAlvos) {
        if (!a.caiu && a.alvo.certo && Vivo(a.p, mRelogio - a.nasceu)) return a.p->GetPosition();
    }
    return std::nullopt;
}

void Hugo::RefazerPlacar() {
    if (mFase.empty()) return;
    std::string texto;
    if (mEtapa == Objetivo::Etapa::Nenhuma) {
        texto = mPrincipal;
    } else if (!mEscudo) {
        const int falta = static_cast<int>(std::ceil(std::max(0.0f, mJanelaAte - mRelogio)));
        texto = (mPrincipal.empty() ? std::string() : mPrincipal + "\n") + "o Hugo está aberto: " +
                std::to_string(falta) + " s";
    } else if (mTipo != Objetivo::Tipo::Derrubar && mObjetivoEmCurso) {
        int faltam = 0;
        switch (mTipo) {
            case Objetivo::Tipo::Referencias:
                for (const AlvoEmCampo& a : mAlvos) faltam += (a.alvo.certo && !a.caiu) ? 1 : 0;
                break;
            case Objetivo::Tipo::Arguicao: faltam = Objetivo::Raspadas(NivelDeDificuldade()) - mRaspou; break;
            case Objetivo::Tipo::Entrega:  faltam = mCarregando ? 0 : 1; break;
            default: break;
        }
        texto = (mPrincipal.empty() ? std::string() : mPrincipal + "\n") + Objetivo::Pede(mTipo, faltam);
    } else if (mEtapa == Objetivo::Etapa::Defesa && !mAlvos.empty()) {
        texto = Tcc::Professor(mQuemPergunta) + ": " + mQuestao.pergunta + "\n" +
                (mPrincipal == "Resposta errada!" ? "errada! acerte a certa" : "acerte a resposta certa");
    } else {
        int faltam = 0;
        for (const AlvoEmCampo& a : mAlvos) faltam += (a.alvo.certo && !a.caiu) ? 1 : 0;
        const std::string objetivo = mAlvos.empty() ? std::string("o Hugo está blindado")
                                                    : Objetivo::Falta(mEtapa, faltam);
        texto = (mPrincipal.empty() ? std::string() : mPrincipal + "\n") + objetivo;
    }
    if (texto.empty()) {
        if (const auto it = mAnunciosDoArquivo.find(mFase); it != mAnunciosDoArquivo.end()) texto = it->second;
    }
    if (texto == mPlacarFeito) return;
    mPlacarFeito = texto;
    RegistrarAnuncio(mFase, texto);
}

// ---------------------------------------------------------------------------
// AS OUTRAS MANEIRAS DE ABRIR: andar, e nao atirar
// ---------------------------------------------------------------------------

std::vector<Player*> Hugo::Alunos() const {
    std::vector<Player*> alunos;
    if (const auto* batalha = dynamic_cast<Battle*>(mScene)) {
        for (Player* p : batalha->Jogadores()) {
            if (p && p->GetState() == ActorState::Active) alunos.push_back(p);
        }
    }
    return alunos;
}

namespace {
    /// O centro do corpo do aluno: o colisor, e nao o desenho.
    Vector2 CentroDe(const Actor* a) {
        if (const auto* c = a->GetComponent<CircleColliderComponent>()) return c->GetCenter();
        return a->GetPosition();
    }
    float RaioDe(const Actor* a) {
        if (const auto* c = a->GetComponent<CircleColliderComponent>()) return c->GetRadius();
        return 10.0f;
    }

    /// A VERSAO FINAL, NA MAO, vai acima da cabeca de quem a leva.
    constexpr float kAcimaDaCabeca = 56.0f;
}

void Hugo::Pagar(const float bruto) {
    if (auto* batalha = dynamic_cast<Battle*>(mScene)) batalha->Conquistar(bruto);
}

void Hugo::EncerrarAndar() {
    if (auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->GetOrientador()) {
        batalha->GetOrientador()->MarcarReuniao(std::nullopt);
    }
    mCarregando = nullptr;
    mRaspados.clear();
    mAlguemNaReuniao = false;
}

void Hugo::AtualizarAndar(const float deltaTime) {
    const Dificuldade::Nivel nivel = NivelDeDificuldade();
    const std::vector<Player*> alunos = Alunos();
    const auto perto = [&alunos](const Vector2& de, const float raio) -> Player* {
        for (Player* a : alunos) {
            if ((CentroDe(a) - de).Length() <= raio) return a;
        }
        return nullptr;
    };

    switch (mTipo) {
        case Objetivo::Tipo::Reuniao: {
            // O ANEL ENCHE com alguem dentro, e nao esvazia quando ele sai.
            mAlguemNaReuniao = perto(mReuniao, Objetivo::kRaioDaReuniao) != nullptr;
            if (mAlguemNaReuniao) mNaReuniao += deltaTime;
            mProgresso = mNaReuniao / Objetivo::TempoDeReuniao(nivel);
            if (mProgresso >= 1.0f) {
                Pagar(Objetivo::PorPasso(mTipo, nivel));
                AbrirJanela();
            }
            return;
        }
        case Objetivo::Tipo::Referencias: {
            int pegos = 0;
            for (AlvoEmCampo& a : mAlvos) {
                if (!a.caiu && Vivo(a.p, mRelogio - a.nasceu)) {
                    // OS LIVROS BOIAM no lugar, cada um no seu ritmo.
                    const Vector2 onde = Objetivo::Balanco(Objetivo::Etapa::Correcoes, a.alvo.lugar, a.k, mRelogio - a.nasceu);
                    a.p->SetPosition(onde);
                    if (perto(onde, Objetivo::kRaioDaColeta)) {
                        a.caiu = true;
                        a.p->Kill();
                        Pagar(Objetivo::PorPasso(mTipo, nivel));
                        mFaiscas.push_back({onde, mRelogio, true});
                    }
                }
                pegos += a.caiu ? 1 : 0;
            }
            mProgresso = static_cast<float>(pegos) / static_cast<float>(std::max(1, mPlacas));
            if (pegos >= static_cast<int>(mAlvos.size()) && !mAlvos.empty()) AbrirJanela();
            return;
        }
        case Objetivo::Tipo::Arguicao: {
            const auto* batalha = dynamic_cast<Battle*>(mScene);
            if (!batalha || !batalha->GetProjectileManager()) return;
            // OS QUE JA CONTARAM e sumiram saem da lista: o pool os devolve.
            mRaspados.erase(std::remove_if(mRaspados.begin(), mRaspados.end(),
                                           [this](const Raspado& r) { return !Vivo(r.p, mRelogio - r.nasceu); }),
                            mRaspados.end());
            for (const auto& bp : batalha->GetProjectileManager()->GetBossProjectiles()) {
                if (!bp || bp->GetState() != ActorState::Active || bp->Enfeite() || bp->Abrigo()) continue;
                if (!bp->Posto().empty()) continue;
                const auto* colisor = bp->GetComponent<CircleColliderComponent>();
                if (!colisor) continue;
                const bool contou = std::any_of(mRaspados.begin(), mRaspados.end(),
                                                [&bp](const Raspado& r) { return r.p == bp.get(); });
                if (contou) continue;
                for (Player* a : alunos) {
                    // ACERTADO NAO RASPA: o piscar depois do acerto nao vale.
                    if (a->GetIsInvincible()) continue;
                    const float d = (colisor->GetCenter() - CentroDe(a)).Length();
                    if (!Objetivo::Raspou(d, colisor->GetRadius() + RaioDe(a))) continue;
                    mRaspados.push_back({bp.get(), mRelogio - bp->TempoDeVida()});
                    ++mRaspou;
                    Pagar(Objetivo::PorPasso(mTipo, nivel));
                    mFaiscas.push_back({CentroDe(a), mRelogio, true});
                    break;
                }
            }
            mProgresso = static_cast<float>(mRaspou) / static_cast<float>(Objetivo::Raspadas(nivel));
            if (mRaspou >= Objetivo::Raspadas(nivel)) AbrirJanela();
            return;
        }
        case Objetivo::Tipo::Entrega: {
            if (mAlvos.empty()) return;
            AlvoEmCampo& versao = mAlvos.front();
            if (!Vivo(versao.p, mRelogio - versao.nasceu)) return;
            if (mCarregando && mCarregando->GetIsInvincible()) {
                // ACERTADO NO CAMINHO: a versao cai ali, e tem de ser pega de novo.
                versao.alvo.lugar = versao.p->GetPosition();
                mCarregando = nullptr;
                mFaiscas.push_back({versao.alvo.lugar, mRelogio, false});
            }
            if (!mCarregando) {
                const Vector2 onde = Objetivo::Balanco(Objetivo::Etapa::Correcoes, versao.alvo.lugar, 0, mRelogio - versao.nasceu);
                versao.p->SetPosition(onde);
                for (Player* a : alunos) {
                    if (a->GetIsInvincible() || (CentroDe(a) - onde).Length() > Objetivo::kRaioDaColeta) continue;
                    mCarregando = a;
                    // PAGA SO A PRIMEIRA VEZ: derrubar e pegar de novo nao rende.
                    if (mProgresso < 0.5f) Pagar(Objetivo::PorPasso(mTipo, nivel));
                    mFaiscas.push_back({onde, mRelogio, true});
                    break;
                }
            }
            if (mCarregando) {
                versao.p->SetPosition(CentroDe(mCarregando) + Vector2(0.0f, -kAcimaDaCabeca));
                mProgresso = 0.5f;
                if ((CentroDe(mCarregando) - mSecretaria).Length() <= Objetivo::kRaioDaSecretaria) {
                    Pagar(Objetivo::PorPasso(mTipo, nivel));
                    AbrirJanela();
                }
            }
            return;
        }
        default:
            return;
    }
}

std::optional<Vector2> Hugo::LugarDeIr() const {
    if (!mEscudo || !mObjetivoEmCurso) return std::nullopt;
    switch (mTipo) {
        case Objetivo::Tipo::Reuniao:
            return mReuniao;
        case Objetivo::Tipo::Referencias: {
            std::optional<Vector2> melhor;
            const Vector2 aluno = PosicaoDoAluno();
            for (const AlvoEmCampo& a : mAlvos) {
                if (a.caiu || !Vivo(a.p, mRelogio - a.nasceu)) continue;
                if (!melhor || (a.p->GetPosition() - aluno).Length() < (*melhor - aluno).Length()) melhor = a.p->GetPosition();
            }
            return melhor;
        }
        case Objetivo::Tipo::Entrega:
            if (mCarregando) return mSecretaria;
            if (!mAlvos.empty() && Vivo(mAlvos.front().p, mRelogio - mAlvos.front().nasceu)) return mAlvos.front().p->GetPosition();
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

// ---------------------------------------------------------------------------
// O ESCUDO, DESENHADO: blindado ou aberto se le sem o placar
// ---------------------------------------------------------------------------

namespace {
    constexpr float kRaioDoEscudo = 84.0f;
    constexpr SDL_Color kLilas{196, 170, 230, 255};
    constexpr SDL_Color kOuro{250, 210, 80, 255};
    constexpr SDL_Color kVerde{110, 220, 140, 255};
    constexpr SDL_Color kFaisca{215, 225, 255, 255};

    SDL_Color ComAlfa(SDL_Color c, const float a) {
        c.a = static_cast<Uint8>(std::clamp(a, 0.0f, 255.0f));
        return c;
    }

    void Arco(DrawLinhasComponent& l, const Vector2& c, const float r, const float a0, const float a1,
              const SDL_Color cor, const int passos = 10) {
        for (int k = 0; k < passos; ++k) {
            const float x0 = a0 + (a1 - a0) * static_cast<float>(k) / static_cast<float>(passos);
            const float x1 = a0 + (a1 - a0) * static_cast<float>(k + 1) / static_cast<float>(passos);
            l.Acrescentar(c + Vector2(Math::Cos(x0), Math::Sin(x0)) * r, c + Vector2(Math::Cos(x1), Math::Sin(x1)) * r, cor);
        }
    }

    /// Um FIO tracejado, com os tracos correndo de 'de' para 'ate' ('fase' em px).
    void Tracejado(DrawLinhasComponent& l, const Vector2& de, const Vector2& ate, const SDL_Color cor,
                   const float fase, const float traco = 10.0f, const float vao = 8.0f) {
        const Vector2 d = ate - de;
        const float total = d.Length();
        if (total < 1.0f) return;
        const Vector2 u = d * (1.0f / total);
        const float periodo = traco + vao;
        for (float t = std::fmod(fase, periodo) - periodo; t < total; t += periodo) {
            const float a = std::max(0.0f, t), b = std::min(total, t + traco);
            if (b > a) l.Acrescentar(de + u * a, de + u * b, cor);
        }
    }
}

void Hugo::OnCollision(Actor* other) {
    Boss::OnCollision(other);
    if (mEtapa == Objetivo::Etapa::Nenhuma || !dynamic_cast<PlayerProjectile*>(other)) return;
    if (!mEscudo) {
        mApanhouEm = mRelogio;
        return;
    }
    // A FAISCA NA BORDA DO ESCUDO, do lado de onde o tiro veio.
    Vector2 d = other->GetPosition() - GetPosition();
    if (d.Length() < 1.0f) d = Vector2(0.0f, 1.0f);
    mFaiscas.push_back({GetPosition() + d * (kRaioDoEscudo / d.Length()), mRelogio, false});
}

void Hugo::Tingir() {
    auto* sprite = GetComponent<DrawSpriteComponent>();
    if (!sprite) return;
    // BLINDADO, ELE FICA FRIO E APAGADO; aberto, com as cores dele - e um
    // clarao quente a cada acerto.
    if (mEtapa == Objetivo::Etapa::Nenhuma) sprite->SetColor(255, 255, 255);
    else if (mEscudo) sprite->SetColor(85, 92, 135);
    else if (mRelogio - mApanhouEm < 0.08f) sprite->SetColor(255, 200, 110);
    else sprite->SetColor(255, 255, 255);
}

void Hugo::DesenharFios() {
    // UM FIO DO ESCUDO ATE CADA COISA QUE O SEGURA: o que falta derrubar, pegar
    // ou onde ir. Os tracos correm para o Hugo - e dali que vem a forca dele.
    std::vector<Vector2> pontas;
    switch (mTipo) {
        case Objetivo::Tipo::Derrubar:
            for (const AlvoEmCampo& a : mAlvos) {
                // NA DEFESA, as tres respostas: o fio nao entrega a certa.
                const bool conta = mEtapa == Objetivo::Etapa::Defesa || a.alvo.certo;
                if (conta && !a.caiu && Vivo(a.p, mRelogio - a.nasceu)) pontas.push_back(a.p->GetPosition());
            }
            break;
        case Objetivo::Tipo::Referencias:
            for (const AlvoEmCampo& a : mAlvos) {
                if (!a.caiu && Vivo(a.p, mRelogio - a.nasceu)) pontas.push_back(a.p->GetPosition());
            }
            break;
        case Objetivo::Tipo::Reuniao:
            pontas.push_back(mReuniao);
            break;
        case Objetivo::Tipo::Entrega:
            if (mCarregando) pontas.push_back(mSecretaria);
            else if (!mAlvos.empty() && Vivo(mAlvos.front().p, mRelogio - mAlvos.front().nasceu)) {
                pontas.push_back(mAlvos.front().p->GetPosition());
            }
            break;
        case Objetivo::Tipo::Arguicao:
            for (const Player* a : Alunos()) pontas.push_back(CentroDe(a));
            break;
    }
    const Vector2 c = GetPosition();
    for (const Vector2& ponta : pontas) {
        Vector2 d = ponta - c;
        const float n = d.Length();
        if (n < kRaioDoEscudo + 40.0f) continue;
        d = d * (1.0f / n);
        // DA PONTA PARA O HUGO: o tracejado comeca la e anda para ca.
        Tracejado(*mEscudoLinhas, ponta - d * 26.0f, c + d * (kRaioDoEscudo + 8.0f), ComAlfa(kLilas, 120), mRelogio * 60.0f);
    }
}

void Hugo::DesenharAndar() {
    const float pulso = 0.5f + 0.5f * std::sin(mRelogio * 6.0f);
    switch (mTipo) {
        case Objetivo::Tipo::Reuniao: {
            // O LUGAR DA REUNIAO: o anel tracejado, e o progresso por cima, grosso.
            const float r = Objetivo::kRaioDaReuniao;
            for (int k = 0; k < 24; k += 2) {
                const float a0 = 2.0f * Math::Pi * static_cast<float>(k) / 24.0f + mRelogio * 0.4f;
                Arco(*mEscudoLinhas, mReuniao, r, a0, a0 + 2.0f * Math::Pi / 24.0f,
                     ComAlfa(kVerde, mAlguemNaReuniao ? 230.0f : 110.0f + 90.0f * pulso), 3);
            }
            const float fim = -Math::Pi / 2.0f + 2.0f * Math::Pi * std::clamp(mProgresso, 0.0f, 1.0f);
            for (const float dr : {-9.0f, -6.0f, -3.0f}) {
                Arco(*mEscudoLinhas, mReuniao, r + dr, -Math::Pi / 2.0f, fim, ComAlfa(kVerde, 240), 36);
            }
            break;
        }
        case Objetivo::Tipo::Referencias:
            for (const AlvoEmCampo& a : mAlvos) {
                if (a.caiu || !Vivo(a.p, mRelogio - a.nasceu)) continue;
                Arco(*mEscudoLinhas, a.p->GetPosition(), 30.0f + 4.0f * pulso, 0.0f, 2.0f * Math::Pi,
                     ComAlfa(kVerde, 140.0f + 100.0f * pulso), 16);
            }
            break;
        case Objetivo::Tipo::Entrega: {
            const float r = Objetivo::kRaioDaSecretaria;
            const float forca = mCarregando ? 230.0f : 90.0f + 60.0f * pulso;
            Arco(*mEscudoLinhas, mSecretaria, r, 0.0f, 2.0f * Math::Pi, ComAlfa(kVerde, forca), 28);
            Arco(*mEscudoLinhas, mSecretaria, r - 4.0f, 0.0f, 2.0f * Math::Pi, ComAlfa(kVerde, forca * 0.6f), 28);
            if (mAlvos.empty() || !Vivo(mAlvos.front().p, mRelogio - mAlvos.front().nasceu)) break;
            const Vector2 versao = mAlvos.front().p->GetPosition();
            if (mCarregando) {
                // O CAMINHO ATE A SECRETARIA, correndo para la.
                Tracejado(*mEscudoLinhas, versao, mSecretaria, ComAlfa(kVerde, 200), -mRelogio * 80.0f, 12.0f, 10.0f);
            } else {
                Arco(*mEscudoLinhas, versao, 30.0f + 4.0f * pulso, 0.0f, 2.0f * Math::Pi,
                     ComAlfa(kVerde, 140.0f + 100.0f * pulso), 16);
            }
            break;
        }
        case Objetivo::Tipo::Arguicao: {
            // UM ANEL EM VOLTA DO ALUNO, uma conta por raspada: as feitas em ouro.
            const int n = Objetivo::Raspadas(NivelDeDificuldade());
            const float passo = 2.0f * Math::Pi / static_cast<float>(n);
            for (const Player* a : Alunos()) {
                const Vector2 c = CentroDe(a);
                for (int k = 0; k < n; ++k) {
                    const float a0 = -Math::Pi / 2.0f + passo * static_cast<float>(k) + 0.12f;
                    const bool feita = k < mRaspou;
                    for (const float r : {44.0f, 47.0f, 50.0f}) {
                        Arco(*mEscudoLinhas, c, r, a0, a0 + passo - 0.24f, feita ? ComAlfa(kOuro, 240) : ComAlfa(kLilas, 150), 5);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void Hugo::DesenharEscudo() {
    if (!mEscudoLinhas || !mCadeado) return;
    mEscudoLinhas->Limpar();
    mCadeado->Limpar();
    Tingir();
    if (mEtapa == Objetivo::Etapa::Nenhuma) return;
    const Vector2 c = GetPosition();

    // AS FAISCAS: o tiro que bateu no escudo (azul) e o que o aluno conseguiu
    // (ouro). Duram um terco de segundo.
    mFaiscas.erase(std::remove_if(mFaiscas.begin(), mFaiscas.end(),
                                  [this](const Faisca& f) { return mRelogio - f.quando > 0.3f; }),
                   mFaiscas.end());
    for (const Faisca& f : mFaiscas) {
        const float t = (mRelogio - f.quando) / 0.3f;
        const SDL_Color cor = ComAlfa(f.dourada ? kOuro : kFaisca, 255.0f * (1.0f - t));
        for (int k = 0; k < 6; ++k) {
            const float a = Math::Pi / 3.0f * static_cast<float>(k) + f.quando * 7.0f;
            const Vector2 u(Math::Cos(a), Math::Sin(a));
            mEscudoLinhas->Acrescentar(f.onde + u * (4.0f + 10.0f * t), f.onde + u * (10.0f + 18.0f * t), cor);
        }
    }

    if (mEscudo) {
        // BLINDADO: o escudo e feito de PLACAS, uma por coisa a fazer. Cada uma
        // que cai deixa so o vulto - da para contar o que falta sem ler nada.
        // Ao fechar, ele se refaz de fora para dentro.
        const float forma = std::clamp((mRelogio - mFechouEm) / 0.3f, 0.0f, 1.0f);
        const float r = kRaioDoEscudo + (1.0f - forma) * 80.0f;
        const int total = std::max(1, mPlacas);
        const int dePe = mObjetivoEmCurso ? Objetivo::PlacasDePe(total, mProgresso) : total;
        const float giro = mRelogio * 0.35f;
        const float passo = 2.0f * Math::Pi / static_cast<float>(total);
        const float vao = total > 1 ? 0.14f : 0.0f;
        const float brilho = (150.0f + 80.0f * std::fabs(std::sin(mRelogio * 2.5f))) * forma;
        for (int i = 0; i < total; ++i) {
            const float a0 = giro + passo * static_cast<float>(i) + vao / 2.0f;
            const float a1 = a0 + passo - vao;
            const int passos = std::max(3, static_cast<int>(40.0f / static_cast<float>(total)));
            if (i < dePe) {
                for (const float dr : {-5.0f, -2.0f, 1.0f, 4.0f}) Arco(*mEscudoLinhas, c, r + dr, a0, a1, ComAlfa(kLilas, brilho), passos);
                for (const float a : {a0, a1}) {
                    const Vector2 u(Math::Cos(a), Math::Sin(a));
                    mEscudoLinhas->Acrescentar(c + u * (r - 5.0f), c + u * (r + 4.0f), ComAlfa(kLilas, brilho));
                }
            } else {
                Arco(*mEscudoLinhas, c, r, a0, a1, ComAlfa(kLilas, 45.0f * forma), passos);
            }
        }
        // E O CADEADO, no peito dele: o sinal que todo mundo le.
        const Vector2 cad = c + Vector2(0.0f, 6.0f);
        const SDL_Color corpo = ComAlfa(kLilas, 235.0f * forma);
        Arco(*mCadeado, cad + Vector2(0.0f, -10.0f), 10.0f, Math::Pi, 2.0f * Math::Pi, corpo, 8);
        Arco(*mCadeado, cad + Vector2(0.0f, -10.0f), 8.0f, Math::Pi, 2.0f * Math::Pi, corpo, 8);
        for (const float x : {-10.0f, -8.0f, 8.0f, 10.0f}) {
            mCadeado->Acrescentar(cad + Vector2(x, -10.0f), cad + Vector2(x, -2.0f), corpo);
        }
        for (float y = -2.0f; y <= 18.0f; y += 2.0f) {
            mCadeado->Acrescentar(cad + Vector2(-15.0f, y), cad + Vector2(15.0f, y), corpo);
        }
        const SDL_Color furo = ComAlfa(SDL_Color{40, 30, 60, 255}, 255.0f * forma);
        mCadeado->Acrescentar(cad + Vector2(-1.0f, 4.0f), cad + Vector2(-1.0f, 12.0f), furo);
        mCadeado->Acrescentar(cad + Vector2(1.0f, 4.0f), cad + Vector2(1.0f, 12.0f), furo);
        if (mObjetivoEmCurso) {
            DesenharFios();
            DesenharAndar();
        }
        return;
    }

    // ABRIU: as placas voam para fora e somem.
    const float t = mRelogio - mAbriuEm;
    if (t < 0.45f) {
        const int total = std::max(1, mPlacas);
        const float passo = 2.0f * Math::Pi / static_cast<float>(total);
        for (int i = 0; i < total; ++i) {
            const float meio = passo * (static_cast<float>(i) + 0.5f) + mAbriuEm * 0.35f;
            const Vector2 u(Math::Cos(meio), Math::Sin(meio));
            const Vector2 centro = c + u * (t * 380.0f);
            Arco(*mEscudoLinhas, centro, kRaioDoEscudo, meio - passo * 0.35f, meio + passo * 0.35f,
                 ComAlfa(kOuro, 255.0f * (1.0f - t / 0.45f)), 6);
        }
    }

    // ABERTO: a MIRA dourada em volta dele - quatro cantos que respiram - e o
    // arco do tempo que falta, piscando nos dois ultimos segundos.
    const float falta = mJanelaAte - mRelogio;
    const float d = 72.0f + 6.0f * std::sin(mRelogio * 8.0f);
    const float braco = 22.0f;
    for (const float sx : {-1.0f, 1.0f}) {
        for (const float sy : {-1.0f, 1.0f}) {
            const Vector2 canto = c + Vector2(sx * d, sy * d);
            for (const float o : {0.0f, 2.0f}) {
                mEscudoLinhas->Acrescentar(canto + Vector2(-sx * o, -sy * o), canto + Vector2(-sx * braco, -sy * o), ComAlfa(kOuro, 230));
                mEscudoLinhas->Acrescentar(canto + Vector2(-sx * o, -sy * o), canto + Vector2(-sx * o, -sy * braco), ComAlfa(kOuro, 230));
            }
        }
    }
    if (falta < 2.0f && std::fmod(mRelogio, 0.3f) < 0.15f) return;
    const float total = Objetivo::Janela(mEtapa, NivelDeDificuldade());
    const float fracao = std::clamp(falta / total, 0.0f, 1.0f);
    for (const float r : {88.0f, 91.0f}) {
        Arco(*mEscudoLinhas, c, r, -Math::Pi / 2.0f, -Math::Pi / 2.0f + 2.0f * Math::Pi * fracao, ComAlfa(kOuro, 220), 40);
    }
}

void Hugo::AoTerminarABatalha() {
    DispensarABanca();
    MatarAlvos();
    mEtapa = Objetivo::Etapa::Nenhuma;
    mEscudo = false;
    Tingir();
    if (mEscudoLinhas) mEscudoLinhas->Limpar();
    if (mCadeado) mCadeado->Limpar();
    if (mLinhas) mLinhas->Limpar();
}
