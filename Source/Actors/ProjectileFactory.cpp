//
// Created by nslop on 10/12/2024.
//

#include "ProjectileFactory.h"

void ProjectileFactory::configureBaseDrawComponent(DrawAnimatedComponent* drawAnimatedComponent) {
    drawAnimatedComponent->AddAnimation("Idle", {0});
    drawAnimatedComponent->SetAnimation("Idle");
}
