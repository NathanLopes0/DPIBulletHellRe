//
// Created by nslop on 25/09/2024.
//

#include <string>
#include "../Boss.h"
#include "../../../Game.h"
#include "Boss1Factory.h"
#include "../Bosses/Salles.h"

Boss1Factory::Boss1Factory(Game *game) : BossFactory(game) {

}

Boss* Boss1Factory::CreateBoss(Scene* scene) {

    std::string spritePath = "../Assets/Teachers/DPIBHSalles.png";
    std::string dataPath = "../Assets/Teachers/DPIBHSalles.json";

    std::string SallesProjectileSpritePath = "../Assets/Teachers/Projectiles/Capivara.png";
    std::string SallesProjectileDataPath = "../Assets/Teachers/Projectiles/Capivara/json";

    mBoss = new Salles(scene, spritePath, dataPath, SallesProjectileSpritePath, SallesProjectileDataPath);

    //      ----------DEFINIÇÃO DE POSIÇÃO E VELOCIDADE INICIAL, E ATIVAÇÃO DO BOSS----------      //
    mBoss->SetState(ActorState::Active);
    mBoss->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);

    auto midWidth = mBoss->GetWindowsWidth() / 2;
    auto spriteHeight = mBoss->GetSpriteHeight();

    mBoss->SetPosition(Vector2((float)midWidth, -1.f * (float)spriteHeight));
    mBoss->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0, 50));
    //      ------------------- -- ------- - ---------- -------- - -------- -- ---------------      //

    return mBoss;
}
