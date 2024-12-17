//
// Created by nslop on 21/11/2024.
//


#include <algorithm>
#include "../../Boss.h"
#include "BossProjectile.h"

BossProjectile::BossProjectile(Scene *scene, Boss *owner)
:Projectile(scene),
mOwner(owner)
{

}

void BossProjectile::OnUpdate(float deltaTime) {

    //O OnUpdate do Projectile já vê se ele ta dentro dos limites da tela, e se nao tiver, transforma o State em Destroy.
    Projectile::OnUpdate(deltaTime);
    updateBehaviors(deltaTime);

    auto tempVec = Vector2::Normalize(mRigidBodyComponent->GetVelocity());
    //mRigidBodyComponent->SetVelocity(tempVec * mFowardSpeed);


    // TODO COLLISION 10 - CheckCollision de Boss retorna true/1 se colidir com jogador
    //  Por enquanto ta como BOOL, pode transformar em int no futuro
    //  Rever como (e principalmente em que lugar do código) ver colisão com qualquer coisa da tela nos slides do Lucas.
    if(CheckCollision())
    {
        //TODO COLLISION 14 - fazer Lógica de diminuir nota da fase

        //Depois da lógica, destruir o projétil.
        DestroyProjectile();
    }
}

bool BossProjectile::InsideProjectileLimit() const {

    auto currPos = GetPosition();
    auto width = mScene->GetGame()->GetWindowWidth();
    auto height = mScene->GetGame()->GetWindowHeight();
    auto spriteWidth = mDrawComponent->GetSpriteWidth();
    auto spriteHeight = mDrawComponent->GetSpriteHeight();

    if(!(currPos.x <= (float)width + (float)spriteWidth && currPos.x >= (float)-spriteWidth - (float)width / 12
        && currPos.y <= (float)height + (float)spriteHeight && currPos.y >= (float)-spriteHeight - (float)height) / 12)
        return false;
    return true;

}

bool BossProjectile::CheckCollision() {
    // TODO COLLISION 9 - Lógica de colisão com o jogador (todos os projéteis de bosses colidem com o jogador, por isso ta aqui)
    // TODO COLLISION 5 - Receber Player aqui com função parecida com GetPlayerPosition (talvez usar essa criada em GetPPosition tbm)
    //  fazer GPPosition em Scene com virtual retornando nullptr, mas sobreescrever em BattleScene, e usar
    //  aqui e usar isso pra chamar o Intersect do player.
    //  Pensando... será que é melhor fazer essa checagem no player? pq serão muuuitos intersects na pilha se forem os projéteis que
    //  veem essa colisão. Deve ser melhor uma função na pilha vendo cada CircleCollider, e é bom que já trato a colisão com o Boss.
    return false;
}

void BossProjectile::insertComponents(DrawAnimatedComponent *pComponent = nullptr,
                                      CircleColliderComponent *pComponent1 = nullptr) {
    mDrawComponent = new DrawAnimatedComponent(*pComponent);
    mColliderComponent = new CircleColliderComponent(*pComponent1);

}

const Vector2& BossProjectile::GetPlayerDirection() {
    auto projPos = GetPosition();
    auto playerPos = GetPlayerPosition();

    auto direction = new Vector2(playerPos - projPos);
    direction->Normalize();

    return *direction;

}

const Vector2 &BossProjectile::GetPlayerPosition() {
    return mScene->GetPlayerPosition();
}

void BossProjectile::updateBehaviors(float deltaTime) {
    for(auto& behavior : mBehaviors) {
        behavior->update(this, deltaTime);
    }

    mBehaviors.erase(std::remove_if(mBehaviors.begin(), mBehaviors.end(),
                                    [](const std::unique_ptr<ProjectileBehavior>& behavior) {
        return behavior->isFinished();
    }), mBehaviors.end());

}



