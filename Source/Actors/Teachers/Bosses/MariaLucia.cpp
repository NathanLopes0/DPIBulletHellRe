//
// A chefe da INF 221 - Engenharia de Software.
//

#include "MariaLucia.h"

#include <algorithm>

#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/DrawComponents/DrawLinhasComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../Player/Player.h"
#include "BossesProjectiles/BossProjectile.h"

namespace {
    const SDL_Color kAssinatura = {130, 200, 250, 70};     // a linha de sempre, quase apagada
    const SDL_Color kNotificando = {210, 240, 255, 210};   // a linha acesa, no aviso
}

MariaLucia::MariaLucia(Scene* scene) : Boss(scene) {
}

void MariaLucia::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarObserver(deltaTime);
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
        mSujeitos.assign(batalha->Jogadores().size(), Observer::Sujeito{});
        mPiscando = 0.0f;
        mNoPlacar = 0.0f;
        if (mNomeDoPadrao.empty()) mNomeDoPadrao = AnuncioDaFase(kFaseDoObserver);
    }
    if (mSujeitos.size() != batalha->Jogadores().size()) {
        mSujeitos.resize(batalha->Jogadores().size());
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
