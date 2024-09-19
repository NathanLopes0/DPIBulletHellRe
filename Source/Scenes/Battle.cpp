//
// Created by nslop on 16/09/2024.
//

#include "Battle.h"
#include "../Scenes/StageSelect.h"

Battle::Battle(Game *game) : Scene(game, SceneType::Battle) {

    auto stageSelect = mGame->GetStageSelect();
    int bossNum;
    if(stageSelect)
        bossNum = stageSelect->GetSelected();

    //terminar de inicializar mBossFactory
    //para isso, criar todas as BossFactory em Game e inicializa-las



}
