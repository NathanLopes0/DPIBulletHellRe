//
// O orientador do TCC, em campo.
//

#include "Orientador.h"

#include <cmath>
#include <vector>

#include "../../Escolta.h"
#include "../../Math.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Components/DrawComponents/DrawSpriteComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Scenes/Battle/ProjectileManager.h"

Orientador::Orientador(Scene* scene, std::string professor, const std::string& retrato)
    : Actor(scene),
      mProfessor(std::move(professor))
{
    SetScale(kEscala);
    AddComponent<DrawSpriteComponent>(retrato, kOrdemDeDesenho);
}

Battle* Orientador::Batalha() {
    return dynamic_cast<Battle*>(GetScene());
}

Vector2 Orientador::PostoAgora() {
    Battle* batalha = Batalha();
    if (!batalha) return GetPosition();

    std::vector<Vector2> alunos;
    for (const Player* p : batalha->Jogadores()) {
        if (p) alunos.push_back(p->GetPosition());
    }

    const SDL_FRect campo = batalha->GetPlayfieldBounds();
    const float esquerda = campo.x;
    const float direita = campo.x + campo.w;
    if (alunos.size() == 1) mLado = Escolta::Lado(alunos.front().x, mLado, esquerda, direita);
    return Escolta::Posto(alunos, mLado, esquerda, direita);
}

void Orientador::Pousar() {
    mPouso = PostoAgora();
    SetPosition(mPouso);
}

void Orientador::OnUpdate(const float deltaTime) {

    Battle* batalha = Batalha();
    if (!batalha) return;

    // ELE VOA SEMPRE, MAS SO ATIRA NA LUTA: na tela de carregamento o chefe
    // ainda nao comecou, e na de fim a nota ja foi fechada.
    mTempo += deltaTime;
    mPouso = Escolta::Seguir(mPouso, PostoAgora(), deltaTime);
    const float balanco = kBalanco * std::sin(mTempo * kBalancoPorSegundo * 2.0f * Math::Pi);
    SetPosition(mPouso + Vector2(0.0f, balanco));

    if (!batalha->EmCombate()) return;

    mRecarga -= deltaTime;
    if (mRecarga > 0.0f) return;
    mRecarga += kIntervalo;
    Atirar(*batalha);
}

void Orientador::Atirar(Battle& batalha) {

    const Boss* chefe = batalha.GetBoss();
    ProjectileManager* tiros = batalha.GetProjectileManager();
    if (!chefe || !tiros) return;

    for (const Vector2& direcao : Escolta::Leque(GetPosition(), chefe->GetPosition(), kTiros, kAbertura)) {
        auto tiro = std::make_unique<PlayerProjectile>(GetScene(), this);
        tiro->ActivateProjectile();   // nasce onde o orientador esta
        if (auto* rb = tiro->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(direcao * kVelocidadeDoTiro);
        }
        tiros->AddPlayerProjectile(std::move(tiro));
        ++mDisparos;
    }
}
