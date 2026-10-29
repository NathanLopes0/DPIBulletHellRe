//
// Created by nslop on 19/11/2024.
//

#include <algorithm>
#include "Player.h"
#include "../../Game.h"
#include "PlayerProjectile.h"

#include "../../Components/RigidBodyComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Teachers/Boss.h"

#include "../../Core/CaminhosArquivo.h"
#include "../../UI/Hud.h"



PlayerProjectile::PlayerProjectile(Scene *scene, Actor *owner) :
        PlayerProjectile(scene, owner, 0)
{
}

PlayerProjectile::PlayerProjectile(Scene *scene, Actor *owner, const size_t quem) :
        Projectile(scene, owner)

{

    mOwner = owner;
    static std::uint64_t proximo = 0;
    mId = ++proximo;

    // A FOLHA DO ALUNO: PlayerProjectile1 o primeiro, PlayerProjectile2 o segundo.
    const std::string folha = quem == 1 ? "Player/PlayerProjectile2" : "Player/PlayerProjectile1";
    std::string spritePath = Caminhos::Asset(folha + ".png");
    std::string dataPath = Caminhos::Asset(folha + ".json");

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

    // A PROMESSA QUE NAO ACERTOU NADA volta do alto do campo, em vez de sumir.
    if (mPromessa && !mVoltando && GetPosition().y < kAltoDaVolta) Voltar();

    // NA VOLTA ela persegue quem a lancou, e some ao chegar.
    if (mVoltando && mOwner) {
        const Vector2 falta = mOwner->GetPosition() - GetPosition();
        if (falta.Length() < kChegou) {
            DeactivateProjectile();
            return;
        }
        if (auto* rb = GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(falta * (mRapidezDaVolta / falta.Length()));
        }
    }

    if (Murchando()) {
        mMurcha += deltaTime;
        if (mMurcha >= Hud::kMurcha) {
            DeactivateProjectile();
            return;
        }
        SetScale(mEscalaInteira * Hud::Murcho(mMurcha));
    }

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

void PlayerProjectile::Voltar() {
    if (mVoltando) return;
    mVoltando = true;
    if (auto* rb = GetComponent<RigidBodyComponent>()) {
        mRapidezDaVolta = std::max(kRapidezMinimaDaVolta, rb->GetVelocity().Length());
    } else {
        mRapidezDaVolta = kRapidezMinimaDaVolta;
    }
    // RESOLVIDA: o contorno passa do azul dos aliados ao dourado do Ponto
    // Extra, e ela cresce um pouco - agora ela tem valor, e se ve que volta.
    if (auto* d = GetComponent<DrawAnimatedComponent>()) d->DefinirContorno(true, SDL_Color{255, 215, 80, 255});
    SetScale(GetScale() * 1.25f);
}

void PlayerProjectile::Murchar() {
    if (Murchando()) return;
    mMurcha = 0.0f;
    mEscalaInteira = GetScale();

    // CAI DEVAGAR, em vez de seguir subindo: a pergunta perdeu o folego.
    if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(0.0f, 110.0f));
    if (auto* c = GetComponent<CircleColliderComponent>()) c->SetEnabled(false);
    if (auto* d = GetComponent<DrawAnimatedComponent>()) {
        d->SetColor(150, 150, 160);
        d->SetIsPaused(true);
    }
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

