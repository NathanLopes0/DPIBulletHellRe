//
// O chefe do TCC - Trabalho de Conclusao de Curso.
//

#include "Hugo.h"

#include <cmath>

#include "../../../CaminhosArquivo.h"
#include "../../../Game.h"
#include "../../../MateriasArquivo.h"
#include "../../../Attacks/Flex.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/DrawComponents/DrawSpriteComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Orientador/Orientador.h"
#include "../../Player/Player.h"
#include "../BossFactory/IBossFactory.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    /// O tiro guardado ainda e o mesmo se a idade dele bate com a esperada: o
    /// pool devolve o projetil com o relogio zerado.
    constexpr float kFolgaDaIdade = 0.1f;

    constexpr int kOrdemDasLinhas = 84;
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

    if (fase == Cronograma::kFaseDaProposta) AtualizarCronograma();
    else if (fase == Tcc::kFaseDoTema) {
        if (agora - mUltimoTema >= kEscrevendo) mTempoVarrendo += deltaTime;
        AtualizarTema(agora);
    }
    else if (fase == Tcc::kFaseDaDefesa) { AtualizarDefesa(agora); Anotar(agora); }
    else if (fase == Tcc::kFaseDasCorrecoes) { AtualizarCorrecoes(agora); Anotar(agora); }

    AtualizarPadroes(deltaTime);
}

void Hugo::TrocarDeFase(const std::string& nova) {
    // SAINDO: o que era da fase some, e o placar volta ao nome dela.
    if (mFase == Cronograma::kFaseDaProposta) {
        MatarPosto(Cronograma::kPostoDoCronograma);
        mBarras.clear();
    }
    if (mFase == Tcc::kFaseDaDefesa) DispensarABanca();
    if (mLinhas) mLinhas->Limpar();
    mExecucoes.clear();
    mAlvo.reset();
    if (const auto it = mAnunciosDoArquivo.find(mFase); it != mAnunciosDoArquivo.end()) {
        RegistrarAnuncio(mFase, it->second);
    }

    mFase = nova;
    mProximaAnotacao = Tcc::kPrimeiro;
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
    RegistrarAnuncio(fase, texto);
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
    const int k = mPerguntas++ % 2;
    const Golpe::Materia quem = mBanca[static_cast<size_t>(k)];
    Lancar(quem, k == 0 ? Fonte::Banca0 : Fonte::Banca1);
    Anunciar(Tcc::kFaseDaDefesa, Tcc::Professor(quem) + ":\n" + Tcc::Pergunta(quem));
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
