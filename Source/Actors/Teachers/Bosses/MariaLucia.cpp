//
// A chefe da INF 221 - Engenharia de Software.
//

#include "MariaLucia.h"

#include <algorithm>

#include "../../../Attacks/Ancora.h"
#include "../../../Attacks/AttackParameters/SprintAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Movements/Escada.h"
#include "../../../Random.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

#include <limits>

namespace {
    const SDL_Color kAssinatura = {130, 200, 250, 70};     // a linha de sempre, quase apagada
    const SDL_Color kNotificando = {210, 240, 255, 210};   // a linha acesa, no aviso
}

MariaLucia::MariaLucia(Scene* scene) : Boss(scene) {
}

void MariaLucia::SortearALuta() {
    mLutaSorteada = true;

    Variacao::Sorteio cascata = SorteioDaFase(Cascata::kFaseDaCascata);
    mPlanoDaCascata = Cascata::Sortear(cascata);

    // O PLANNING POKER: cada sprint com a carta dela, e o placar dizendo os
    // pontos - o placar e o compromisso. A primeira etapa e o backlog.
    Variacao::Sorteio metodos = SorteioDaFase(kFaseDosMetodos);
    mPoker = Sprint::Sortear(metodos);
    auto etapas = EtapasDa(kFaseDosMetodos);
    if (etapas.size() > static_cast<size_t>(Sprint::kSprints)) {
        for (int k = 0; k < Sprint::kSprints; ++k) {
            etapas[static_cast<size_t>(k) + 1].second = Sprint::Anuncio(k + 1, mPoker[static_cast<size_t>(k)]);
        }
        RegistrarEtapas(kFaseDosMetodos, etapas);
    }

    Variacao::Sorteio observer = SorteioDaFase(kFaseDoObserver);
    mSujeitoDaLuta = Observer::Sortear(observer, Observer::DegrauDo(NivelDeDificuldade()));
}

int MariaLucia::SprintDaVez() const {
    const auto* fsm = GetComponent<FSMComponent>();
    const float t = fsm ? fsm->GetStateTime() : 0.0f;
    const auto etapas = EtapasDa(kFaseDosMetodos);
    int k = static_cast<int>(etapas.size());
    for (size_t i = 0; i < etapas.size(); ++i) {
        if (t < etapas[i].first) { k = static_cast<int>(i); break; }
    }
    return std::clamp(k - 1, 0, Sprint::kSprints - 1);
}

void MariaLucia::OnUpdate(const float deltaTime) {
    if (!mLutaSorteada && !EhClone()) SortearALuta();
    Boss::OnUpdate(deltaTime);
    AtualizarCascata(deltaTime);
    AtualizarDividas(deltaTime);
    AtualizarObserver(deltaTime);
}

// ---------------------------------------------------------------------------
// A CASCATA
// ---------------------------------------------------------------------------

namespace {
    const SDL_Color kSeta = {120, 160, 230, 150};   // a associacao entre duas classes
}

std::vector<BossProjectile*> MariaLucia::DispararEm(const char* gatilho, const Vector2 onde, const bool mirar) {
    mDisparandoEm = true;
    mMirandoEm = mirar;
    mOndeDisparar = onde;
    mCriados.clear();
    Disparar(gatilho);
    mDisparandoEm = false;
    mMirandoEm = false;
    return mCriados;
}

void MariaLucia::DesmontarCascata() {
    for (BossProjectile*& p : mPlacas) {
        if (p && p->GetState() == ActorState::Active) p->Kill();
        p = nullptr;
    }
    for (const Pedido& pd : mPedidos) if (pd.p->GetState() == ActorState::Active) pd.p->Kill();
    for (BossProjectile* p : mBacklog) if (p->GetState() == ActorState::Active) p->Kill();
    for (const Classe& c : mClasses) if (c.p->GetState() == ActorState::Active) c.p->Kill();
    mPedidos.clear();
    mBacklog.clear();
    mClasses.clear();
    mBugs.clear();
    if (mDiagrama) mDiagrama->Limpar();
}

void MariaLucia::AtualizarDividas(const float deltaTime) {
    mRelogio += deltaTime;
    for (auto it = mDividas.begin(); it != mDividas.end();) {
        if (it->p->GetState() == ActorState::Active && mRelogio < it->ate) {
            if (auto* rb = it->p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            ++it;
            continue;
        }
        if (it->p->GetState() == ActorState::Active) it->p->Kill();
        it = mDividas.erase(it);
    }
}

void MariaLucia::AtualizarCascata(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naCascata = fsm && fsm->GetStateName() == Cascata::kFaseDaCascata && !EhClone();
    if (!naCascata) {
        if (mNaCascata) {
            DesmontarCascata();
            mNaCascata = false;
        }
        return;
    }
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    const float altura = batalha ? batalha->GetPlayfieldBounds().h : 746.0f;
    const float agora = fsm->GetStateTime();
    const int etapa = Cascata::EtapaNoTempo(agora);

    if (!mNaCascata) {
        mNaCascata = true;
        mEtapaDaCascata = -1;
        mProximoPedido = Cascata::kEntrePedidos;
        // AS CAIXAS DO DIAGRAMA DA CASCATA, uma debaixo de cada degrau.
        Escada::Geometria g;
        g.largura = largura;
        for (int k = 0; k < static_cast<int>(mPlacas.size()); ++k) {
            const Vector2 degrau = Escada::Degrau(k, static_cast<int>(mPlacas.size()), g);
            const auto criadas = DispararEm(Cascata::kGatilhoDaPlaca, degrau + Vector2(0.0f, Cascata::kAbaixoDoDegrau));
            mPlacas[static_cast<size_t>(k)] = criadas.empty() ? nullptr : criadas.front();
            if (auto* p = mPlacas[static_cast<size_t>(k)]) p->DefinirEnfeite(true);
        }
        if (!mDiagrama) {
            auto ator = std::make_unique<Actor>(mScene);
            mDiagrama = ator->AddComponent<DrawLinhasComponent>(kOrdemDasAssinaturas);
            mScene->AddActor(std::move(ator));
        }
    }

    // A ETAPA NOVA: a caixa dela acende, e o que a anterior entregou passa.
    if (etapa != mEtapaDaCascata) {
        mEtapaDaCascata = etapa;
        for (int k = 0; k < static_cast<int>(mPlacas.size()); ++k) {
            BossProjectile* p = mPlacas[static_cast<size_t>(k)];
            if (!p || p->GetState() != ActorState::Active) continue;
            if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) {
                anim->SetAnimation((k == etapa ? "A" : "P") + std::to_string(k));
            }
        }
        if (etapa == 1) {
            // PROJETO: cada post-it do backlog voa para o lugar da classe dele.
            int k = 0;
            for (BossProjectile* p : mBacklog) {
                if (p->GetState() != ActorState::Active) continue;
                if (k >= Cascata::kMaxClasses) { p->Kill(); continue; }
                mClasses.push_back(Classe{p, Cascata::LugarDaClasse(k++, largura), false});
            }
            mBacklog.clear();
            mProximaMensagem = agora + Cascata::kDiagramaPronto;
            mClasseDaVez = 0;
        } else if (etapa == 2) {
            mProximaCompilacao = agora;
            mClasseDaVez = 0;
        } else if (etapa == 3) {
            // TESTES: cada classe solta o bug dela.
            for (const Classe& c : mClasses) {
                if (!c.pronta || c.p->GetState() != ActorState::Active) continue;
                for (BossProjectile* b : DispararEm(Cascata::kGatilhoDoBug, c.p->GetPosition())) mBugs.push_back(b);
            }
        } else if (etapa == 4) {
            // MANUTENCAO: o sistema e entregue - as classes somem -, e cada bug
            // que o teste nao achou vira divida, parada onde estava.
            for (const Classe& c : mClasses) if (c.p->GetState() == ActorState::Active) c.p->Kill();
            mClasses.clear();
            if (mDiagrama) mDiagrama->Limpar();
            for (BossProjectile* b : mBugs) {
                if (b->GetState() != ActorState::Active) continue;
                const Vector2 onde = b->GetPosition();
                b->Kill();
                for (BossProjectile* d : DispararEm(Cascata::kGatilhoDaDivida, onde)) {
                    mDividas.push_back(Divida{d, mRelogio + Cascata::kDivida});
                }
            }
            mBugs.clear();
        }
    }

    // REQUISITOS: o cliente pede, de fora do campo, ate o backlog encher.
    if (etapa == 0 && agora >= mProximoPedido) {
        mProximoPedido = agora + Cascata::kEntrePedidos;
        const int lugar = static_cast<int>(mBacklog.size() + mPedidos.size());
        if (lugar < Cascata::kMaxPedidos) {
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            const Vector2 origem = Cascata::OrigemDoPedido(sorteio, largura, altura);
            for (BossProjectile* p : DispararEm(Cascata::kGatilhoDoPedido, origem)) {
                Vector2 ida = Cascata::LugarNoBacklog(lugar) - origem;
                if (ida.LengthSq() > 0.0f) ida.Normalize();
                if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(ida * Cascata::kVelocidadeDoPedido);
                mPedidos.push_back(Pedido{p, lugar});
            }
        }
    }
    // O PEDIDO QUE CHEGA cola no quadro (e deixa de machucar); o derrubado no
    // caminho e escopo cortado, e o lugar dele fica para o proximo.
    for (auto it = mPedidos.begin(); it != mPedidos.end();) {
        BossProjectile* p = it->p;
        if (p->GetState() != ActorState::Active) { it = mPedidos.erase(it); continue; }
        const Vector2 lugar = Cascata::LugarNoBacklog(it->lugar);
        const Vector2 falta = lugar - p->GetPosition();
        const auto* rb = p->GetComponent<RigidBodyComponent>();
        const bool chegou = falta.LengthSq() < 14.0f * 14.0f || (rb && Vector2::Dot(rb->GetVelocity(), falta) <= 0.0f);
        if (!chegou) { ++it; continue; }
        if (etapa != 0) { p->Kill(); it = mPedidos.erase(it); continue; }
        p->SetPosition(lugar);
        p->DefinirEnfeite(true);
        if (auto* corpo = p->GetComponent<RigidBodyComponent>()) corpo->SetVelocity(Vector2::Zero);
        mBacklog.push_back(p);
        it = mPedidos.erase(it);
    }
    // O backlog fica no lugar, refeito na ordem de chegada.
    for (size_t k = 0; k < mBacklog.size(); ++k) {
        BossProjectile* p = mBacklog[k];
        if (p->GetState() != ActorState::Active) continue;
        p->SetPosition(Ancora::Aproximar(p->GetPosition(), Cascata::LugarNoBacklog(static_cast<int>(k)), kRapidezDoPostIt, deltaTime));
        if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }

    // PROJETO: o post-it que chega ao lugar dele VIRA A CLASSE - sai o post-it,
    // entra a caixa de UML - e a classe machuca: o diagrama ocupa o meio do
    // campo. Ela aguenta tiros (o tipo diz quantos): cortar uma classe do
    // projeto ja nao e tao barato quanto cortar um pedido.
    for (Classe& c : mClasses) {
        if (c.p->GetState() != ActorState::Active) continue;
        if (!c.pronta) {
            c.p->SetPosition(Ancora::Aproximar(c.p->GetPosition(), c.lugar, kRapidezDoPostIt, deltaTime));
            if ((c.lugar - c.p->GetPosition()).LengthSq() < 6.0f * 6.0f) {
                c.p->Kill();
                const auto criadas = DispararEm(Cascata::kGatilhoDaClasse, c.lugar);
                if (criadas.empty()) continue;
                c.p = criadas.front();
                c.pronta = true;
            }
        }
        c.p->SetPosition(c.pronta ? c.lugar : c.p->GetPosition());
        if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
    // As setas entre as classes prontas.
    if (mDiagrama) {
        mDiagrama->Limpar();
        for (const auto& [de, para] : Cascata::Associacoes(static_cast<int>(mClasses.size()))) {
            const Classe& a = mClasses[static_cast<size_t>(de)];
            const Classe& b = mClasses[static_cast<size_t>(para)];
            if (!a.pronta || !b.pronta || a.p->GetState() != ActorState::Active || b.p->GetState() != ActorState::Active) continue;
            mDiagrama->Acrescentar(a.p->GetPosition(), b.p->GetPosition(), kSeta);
        }
    }

    // UMA CLASSE POR VEZ: no Projeto, solta as mensagens pelas setas; na
    // Implementacao, compila - o codigo sai mirado no aluno.
    std::vector<const Classe*> prontas;
    for (const Classe& c : mClasses) if (c.pronta && c.p->GetState() == ActorState::Active) prontas.push_back(&c);
    if (!prontas.empty()) {
        if (etapa == 1 && agora >= mProximaMensagem) {
            mProximaMensagem = agora + Cascata::kEntreMensagens;
            const int vez = Cascata::DaVez(mPlanoDaCascata, static_cast<int>(mClasseDaVez++), static_cast<int>(prontas.size()));
            DispararEm(Cascata::kGatilhoDaMensagem, prontas[static_cast<size_t>(vez)]->p->GetPosition());
        } else if (etapa == 2 && agora >= mProximaCompilacao) {
            mProximaCompilacao = agora + Cascata::kEntreCompilacoes;
            const int vez = Cascata::DaVez(mPlanoDaCascata, static_cast<int>(mClasseDaVez++), static_cast<int>(prontas.size()));
            DispararEm(Cascata::kGatilhoDoCodigo, prontas[static_cast<size_t>(vez)]->p->GetPosition(), true);
        }
    }
}

void MariaLucia::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                      const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoEm) return;
    for (auto& p : projectiles) {
        if (auto* b = dynamic_cast<BossProjectile*>(p.get())) mCriados.push_back(b);
    }
}

void MariaLucia::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // A CARTA DA SPRINT DESTA LUTA: o arquivo escreve o baralho em ordem, e o
    // poker sorteado diz quantas historias o cursor arrasta.
    if (stateName == kFaseDosMetodos && dynamic_cast<SprintAttackParams*>(&params)) {
        params.numProjectiles = mPoker[static_cast<size_t>(SprintDaVez())];
    }

    if (!mDisparandoEm) return;
    params.firePosition = mOndeDisparar;
    if (mMirandoEm) {
        Vector2 d(0.0f, 1.0f);
        if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
            d = batalha->Alvo()->GetPosition() - mOndeDisparar;
            if (d.LengthSq() > 0.0f) d.Normalize();
        }
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
}

std::vector<BossProjectile*> MariaLucia::Observadores() const {
    std::vector<BossProjectile*> olhos;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return olhos;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == Observer::kPostoDosObservadores) {
            olhos.push_back(p.get());
        }
    }
    return olhos;
}

void MariaLucia::AnunciarOObserver() {
    // O NOME DO PADRAO, e o que ela diz no aviso. O placar le o anuncio da
    // fase a cada quadro e so redesenha quando o texto muda.
    std::string texto = mNomeDoPadrao;
    if (mNoPlacar > 0.0f) texto = mUltimoFoiConsulta ? kNoPlacarConsulta : kNoPlacarNotifica;
    if (AnuncioDaFase(kFaseDoObserver) != texto) RegistrarAnuncio(kFaseDoObserver, texto);
}

void MariaLucia::AtualizarObserver(const float deltaTime) {

    const auto* fsm = GetComponent<FSMComponent>();
    auto* batalha = dynamic_cast<Battle*>(mScene);
    const bool noObserver = fsm && batalha && fsm->GetStateName() == kFaseDoObserver;

    if (!noObserver) {
        if (mNoObserver) {
            // A FASE ACABOU: as assinaturas somem, e o placar volta ao nome.
            if (mAssinaturas) mAssinaturas->Limpar();
            mNoPlacar = 0.0f;
            AnunciarOObserver();
            mNoObserver = false;
        }
        return;
    }

    if (!mNoObserver) {
        mNoObserver = true;
        mSujeitos.assign(batalha->Jogadores().size(), mSujeitoDaLuta);
        mPiscando = 0.0f;
        mNoPlacar = 0.0f;
        if (mNomeDoPadrao.empty()) mNomeDoPadrao = AnuncioDaFase(kFaseDoObserver);
    }
    if (mSujeitos.size() != batalha->Jogadores().size()) {
        mSujeitos.resize(batalha->Jogadores().size(), mSujeitoDaLuta);

    }

    if (!mAssinaturas) {
        auto ator = std::make_unique<Actor>(mScene);
        mAssinaturas = ator->AddComponent<DrawLinhasComponent>(kOrdemDasAssinaturas);
        mScene->AddActor(std::move(ator));
    }

    // O SUJEITO: cada aluno avisa quando o estado dele muda, e e a ele que os
    // avisos vao. Sem aviso por um tempo, ela consulta - o polling.
    mPiscando = std::max(0.0f, mPiscando - deltaTime);
    mNoPlacar = std::max(0.0f, mNoPlacar - deltaTime);
    const auto& alunos = batalha->Jogadores();
    for (size_t i = 0; i < alunos.size(); ++i) {
        const Player* aluno = alunos[i];
        if (!aluno) continue;
        const auto* rb = aluno->GetComponent<RigidBodyComponent>();
        const Vector2 v = rb ? rb->GetVelocity() : Vector2::Zero;

        const auto aviso = Observer::Avancar(mSujeitos[i], v, deltaTime);
        if (aviso == Observer::Aviso::Nenhum) continue;

        const bool consulta = (aviso == Observer::Aviso::Consulta);
        Disparar(consulta ? Observer::kGatilhoDaConsulta : Observer::kGatilhoDaNotificacao,
                 static_cast<int>(i));
        mPiscando = kPiscar;
        mNoPlacar = kAvisoNoPlacar;
        mUltimoFoiConsulta = consulta;
    }
    AnunciarOObserver();

    // OS OLHOS E AS ASSINATURAS: cada olho olha para o aluno mais perto e
    // pisca no aviso, e uma linha liga cada olho a cada aluno.
    mAssinaturas->Limpar();
    for (BossProjectile* olho : Observadores()) {
        const Player* maisPerto = nullptr;
        float d2 = 0.0f;
        for (const Player* aluno : alunos) {
            if (!aluno) continue;
            const float d = (aluno->GetPosition() - olho->GetPosition()).LengthSq();
            if (!maisPerto || d < d2) { maisPerto = aluno; d2 = d; }
            mAssinaturas->Acrescentar(olho->GetPosition(), aluno->GetPosition(),
                                      mPiscando > 0.0f ? kNotificando : kAssinatura);
        }
        if (!maisPerto) continue;

        const char* cara = Observer::kPisca;
        if (mPiscando <= 0.0f) {
            const int lado = Observer::Olhar(olho->GetPosition(), maisPerto->GetPosition());
            cara = (lado < 0) ? Observer::kOlhaEsquerda : (lado > 0) ? Observer::kOlhaDireita : Observer::kOlhaFrente;
        }
        if (auto* anim = olho->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }
}
