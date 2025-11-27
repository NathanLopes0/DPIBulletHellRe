//
// Created by gensh on 19/10/2025.
//

#include "IBossFactory.h"

#include <SDL_log.h>

#include "../../Teachers/Boss.h"

IBossFactory::IBossFactory(Game* game) : mGame(game) {

}

std::unique_ptr<Boss> IBossFactory::CreateBoss(Scene *scene) {

    SDL_Log(" INTERFACE BOSS FACTORY - CRIANDO BOSS");
    auto boss = InstantiateBoss(scene);
    if (!boss) {
        SDL_Log(" INTERFACE BOSS FACTORY - BOSS NÃO INSTANCIADO");
    }

    SDL_Log(" INTERFACE BOSS FACTORY - CONFIGURANDO COMPONENTES");
    ConfigureComponents(boss.get());

    SDL_Log(" INTERFACE BOSS FACTORY - CONFIGURANDO ATAQUES E MAQUINA DE ESTADO");
    ConfigureAttacksAndFSM(boss.get());

    SDL_Log(" INTERFACE BOSS FACTORY - RETORNANDO BOSS MONTADO");
    return boss;

}
