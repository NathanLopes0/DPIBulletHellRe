//
// Created by nslop on 21/11/2024.
//

#include "BossProjectile.h"
#include "../../Boss.h"

BossProjectile::BossProjectile(Scene *scene, Boss *owner)
:Projectile(scene),
mOwner(owner)
{

}

void BossProjectile::OnUpdate(float deltaTime) {

    //O OnUpdate do Projectile já vê se ele ta dentro dos limites da tela, e se tiver, transforma o State em Destroy.
    Projectile::OnUpdate(deltaTime);


    // TODO 8.0 - CheckCollision de Boss retorna 1 se colidir com jogador
    // TODO 8.0.1 - Por enquanto ta como BOOL, pode transformar em int no futuro
    // TODO 8.0.2 - Rever como (e principalmente em que lugar do código) ver colisão com qualquer coisa da tela nos slides do Lucas.
    if(CheckCollision())
    {
        //TODO 9.1 - fazer Lógica de diminuir nota da fase

        //Depois da lógica, desativar o projétil. Nesse caso, ele é destruído.
        DeactivateProjectile();
    }
}

void BossProjectile::DeactivateProjectile() {
    SetState(ActorState::Destroy);
}

bool BossProjectile::InsideProjectileLimit() const {

    auto currPos = GetPosition();
    auto width = mScene->GetGame()->GetWindowWidth();
    auto height = mScene->GetGame()->GetWindowHeight();
    auto spriteWidth = mDrawComponent->GetSpriteWidth();
    auto spriteHeight = mDrawComponent->GetSpriteHeight();

    if(!(currPos.x <= (float)width + (float)spriteWidth && currPos.x >= (float)-spriteWidth
        && currPos.y <= (float)height + (float)spriteHeight && currPos.y >= (float)-spriteHeight))
        return false;
    return true;

}

bool BossProjectile::CheckCollision() {
    // TODO 4.1 - Lógica de colisão com o jogador (todos os projéteis de bosses colidem com o jogador, por isso ta aqui)
}
