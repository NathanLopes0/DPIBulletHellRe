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
    if(stageSelect)
        mBossNum = stageSelect->GetSelected();

    mBossFactory = mGame->GetFactory(mBossNum);
    mGrade = mGame->GetGrade(mBossNum);
    //pensar quais outras variáveis Battle teria:
        //os objetos da HUD podem ser uma das variaveis

    //inicializar as outras variaveis de Battle, depois chamar o Load()
    //implementar o Load();

    Battle::Load();
}

void Battle::Load() {

    mBossAllProjectiles.clear();
    mBoss.reset();
    mPlayer.reset();

    mBoss = std::unique_ptr<Boss>(mBossFactory->CreateBoss(this)); //Load no boss, o Bossfactory se encarrega do resto.
    LoadPlayer();                           //Será que devo fazer um Playerfactory? acho que sim... pode ser confuso inicialmente, mas
    LoadHUD();                                      //pode ajudar se eu quiser salvar powerups do player e inicializa-lo com eles etc.

    mBossAllProjectiles.reserve(500);
    //Se houver mais coisa pra dar Load, dar antes do BattleStart().

    BattleStart();
}

void Battle::BattleStart() {
    mBoss->Start();
}

void Battle::Update(float deltaTime) {
    //SDL_Log("%f", mGrade);
}

void Battle::LoadHUD() {

}

void Battle::LoadPlayer() {

    mPlayer = std::make_unique<Player>(this);

    //Posição inicial do jogador:
    auto midWidth = mGame->GetWindowWidth() / 2; //posição X, no meio da tela
    auto startingHeight = mGame->GetWindowHeight() * 5/6;
    auto startingPosition = Vector2((float)midWidth,(float)startingHeight);

    mPlayer->SetPosition(startingPosition);
}

const Vector2 &Battle::GetPlayerPosition() {
    return mPlayer->GetPosition();
}
