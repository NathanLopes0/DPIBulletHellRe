//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"

#include "Battle.h"
#include "../Actors/Player/Player.h"
#include "../Scenes/StageSelect.h"
#include "../Actors/Teacher/Boss.h"

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

    mBoss = mBossFactory->CreateBoss(this); //Load no boss, o Bossfactory se encarrega do resto.
    LoadPlayer();                           //Será que devo fazer um Playerfactory? acho que sim... pode ser confuso inicialmente, mas
    LoadHUD();                                      //pode ajudar se eu quiser salvar powerups do player e inicializa-lo com eles etc.

    //Se houver mais coisa pra dar Load, dar antes do BattleStart().

    BattleStart();
}

void Battle::BattleStart() {
    mBoss->Start();
}

void Battle::LoadHUD() {

}

void Battle::LoadPlayer() {

    mPlayer = new Player(this); //inicializando o player

    //Posição inicial do jogador:
    auto midWidth = mGame->GetWindowWidth() / 2; //posição X, no meio da tela
    auto startingHeight = mGame->GetWindowHeight() * 7/8; //Boss começa em 1/8 do Height da tela, Player começa em 7/8 (espelhado)
    auto startingPosition = Vector2((float)midWidth,(float)startingHeight);

    mPlayer->SetPosition(startingPosition);
}
