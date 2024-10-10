//
// Created by nslop on 25/09/2024.
//

#include <string>
#include "../Boss.h"
#include "../../../Game.h"
#include "Boss1Factory.h"
#include "../Salles.h"

Boss1Factory::Boss1Factory(Game *game) : BossFactory(game) {

}

Boss* Boss1Factory::CreateBoss(Scene* scene) {

    //debug
    SDL_Log("CreateBoss() - Bossfactory1");

    std::string spritePath = "../Assets/Teachers/DPIBHSalles.png";
    std::string dataPath = "../Assets/Teachers/DPIBHSalles.json";


    mBoss = new Salles(scene, spritePath, dataPath);
    return mBoss;
}
