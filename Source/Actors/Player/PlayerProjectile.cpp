//
// Created by nslop on 19/11/2024.
//

#include "Player.h"
#include "../../Game.h"
#include "PlayerProjectile.h"

#include "../../Components/RigidBodyComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Teachers/Boss.h"
#include "../../Scenes/Battle/Battle.h"

#include "../../CaminhosArquivo.h"



PlayerProjectile::PlayerProjectile(Scene *scene, Actor *owner) :
        Projectile(scene, owner)

{

    mOwner = owner;
    static std::uint64_t proximo = 0;
    mId = ++proximo;

    std::string spritePath = Caminhos::Asset("Player/PlayerProjectile1.png");
    std::string dataPath = Caminhos::Asset("Player/PlayerProjectile1.json");

    auto drawComp = AddComponent<DrawAnimatedComponent>(spritePath, dataPath, 50);

    drawComp->AddAnimation("Active", {0,1,2,3,4,5,6});
    drawComp->SetAnimation("Active");

    // TODO 100.0 - Modo dificil o projétil só vai pra cima, Modo normal ela segue o Boss
    auto rigidComp = AddComponent<RigidBodyComponent>();
    drawComp->SetIsVisible(false);

    auto collider = AddComponent<CircleColliderComponent>(static_cast<float>(drawComp->GetSpriteWidth()) / 2.f);
    collider->SetEnabled(false);
    collider->SetTag(ColliderTag::PlayerProjectile);

}

PlayerProjectile::PlayerProjectile(Scene *scene, Actor *owner, const std::string& folha, const int quadro,
                                   const float escala, const SDL_Color contorno) :
        Projectile(scene, owner)
{
    mOwner = owner;
    static std::uint64_t proximo = 1ull << 40;   // longe dos do outro construtor
    mId = ++proximo;

    // O TIRO DO ORIENTADOR TEM A CARA DA MATERIA DELE, e um contorno da cor
    // dos aliados - o mesmo projetil, do outro lado, e do Hugo.
    auto drawComp = AddComponent<DrawAnimatedComponent>(Caminhos::Asset(folha + ".png"),
                                                        Caminhos::Asset(folha + ".json"), 51);
    drawComp->AddAnimation("Active", {quadro});
    drawComp->SetAnimation("Active");
    drawComp->DefinirContorno(true, contorno);
    drawComp->SetIsVisible(false);
    SetScale(escala);

    AddComponent<RigidBodyComponent>();

    // O ALCANCE E O DO TIRO DE SEMPRE: a cara muda, o acerto nao.
    auto collider = AddComponent<CircleColliderComponent>(kRaioDoTiro);
    collider->SetEnabled(false);
    collider->SetTag(ColliderTag::PlayerProjectile);
}

bool PlayerProjectile::IsOffScreen() const
{
    const auto w = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto h = static_cast<float>(mScene->GetGame()->GetWindowHeight());

    if (GetPosition().x > w * 2 || GetPosition().x < -w)
        return true;
    if (GetPosition().y > h * 2 || GetPosition().y < -h)
        return true;

    return false;
}


void PlayerProjectile::OnUpdate(float deltaTime) {
    Projectile::OnUpdate(deltaTime);

    //----- ULTIMA COISA DO UPDATE: Ver colisão. Pois com colisão o objeto pode ser destruído.
}

void PlayerProjectile::ActivateProjectile() {
    SetState(ActorState::Active);

    // Liga os componentes
    if (auto comp = GetComponent<DrawAnimatedComponent>()) {
        comp->SetIsVisible(true);
        comp->SetEnabled(true);
    }
    if (auto comp = GetComponent<CircleColliderComponent>()) {
        comp->SetEnabled(true);
    }

    SetPosition(mOwner->GetPosition());
}

void PlayerProjectile::DeactivateProjectile() {
    SetState(ActorState::Destroy);
}

void PlayerProjectile::OnCollision(Actor *other) {

    if (!other) return;

    auto otherCollider = other->GetComponent<CircleColliderComponent>();
    if (!otherCollider) return;

    // O CHEFE, OU UM PROJETIL DELE QUE O TIRO DERRUBOU. Battle so chama isto com
    // um projetil de chefe quando ele e derrubavel (os processos do Lucas): o
    // tiro gasta-se nele e nao segue ate o chefe.
    if (otherCollider->GetColliderTag() == ColliderTag::Boss
        || otherCollider->GetColliderTag() == ColliderTag::BossProjectile) {
        DeactivateProjectile();
    }
}

