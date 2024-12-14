//
// Created by nslop on 17/09/2024.
//

#include "BossFactory.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"

BossFactory::BossFactory(Game* game) {

        mGame = game;
        mBoss = nullptr;
}

void BossFactory::configureBaseDrawComponent(DrawAnimatedComponent *drawAnimatedComponent) {

    drawAnimatedComponent->AddAnimation("Idle", {0});
    drawAnimatedComponent->SetAnimation("Idle");

}
