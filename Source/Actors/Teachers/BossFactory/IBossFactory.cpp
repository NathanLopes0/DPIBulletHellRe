//
// Created by gensh on 19/10/2025.
//

#include "IBossFactory.h"

#include "../../Teachers/Boss.h"

IBossFactory::IBossFactory(Game* game) : mGame(game) {

}

std::unique_ptr<Boss> IBossFactory::CreateBoss(Scene *scene) {


    auto boss = InstantiateBoss(scene);

    ConfigureComponents(boss.get());

    ConfigureAttacksAndFSM(boss.get());

    return boss;

}
