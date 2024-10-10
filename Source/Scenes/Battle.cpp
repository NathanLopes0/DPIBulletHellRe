//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"

#include "Battle.h"
#include "../Scenes/StageSelect.h"

Battle::Battle(Game *game) : Scene(game, SceneType::Battle) {

    auto stageSelect = mGame->GetStageSelect();
    int bossNum = 0;
    if(stageSelect)
        bossNum = stageSelect->GetSelected();

    mBossFactory = mGame->GetFactory(bossNum);
    mGrade = mGame->GetGrade(bossNum);

    //pensar quais outras variáveis Battle teria:
        //os objetos da HUD podem ser uma das variaveis

    //inicializar as outras variaveis de Battle, depois chamar o Load()
    //implementar o Load();

    Battle::Load();
}

void Battle::Load() {

    mBoss = mBossFactory->CreateBoss(this);
    LoadHUD();
    //Maybe call BattleStart();
}

void Battle::LoadHUD() {

}
