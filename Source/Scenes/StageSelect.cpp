//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "../Actors/Buttons/StageSelectButton.h"
#include "../Font.h"

#define TIME_INTERVAL 0.14

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect),
    mNumMaterias(10),
    mTimer(10)
{
    mStageSelectFont = new Font();
    mStageSelectFont->Load("../Assets/Fonts/Zelda.ttf");

    mButtons.reserve(mNumMaterias);
    StageSelect::Load();

}

void StageSelect::Load() {

    CreateButtons();
    DrawUI();

}

void StageSelect::CreateButtons() {

    float espacamentoX = 162; //espaçamento horizontal entre botões
    float espacamentoY = 140; //espaçamento vertical entre botões

    float buttonWidth = 128;
    float buttonHeight = 64;

    float widthBorder = (float)mGame->GetWindowWidth() / 12;    //margem horizontal
    float heightBorder = (float)mGame->GetWindowHeight() / 12;  //margem vertical

    //Variavel para guardar texto que será escrito dentro do botão
    std::string subjectCode;
    Game::GameSubject subject;
    for (int j = 0; j < 4; j++)
    {
        switch(j)
        {
            case 0:
            {
                subjectCode = "INF 213";
                subject = Game::GameSubject::INF213;
                auto stage = new StageSelectButton(this, subjectCode, subject);
                stage->SetPosition(Vector2(widthBorder + buttonWidth/2, (float)mGame->GetWindowHeight()/2));
                stage->SetText(subjectCode);
                mButtons.push_back(stage);
                break;
            }
            case 1:
            {
                float area2x = widthBorder + 1.20f * (espacamentoX + buttonWidth);
                for (int i = 0; i < 4; i++)
                {
                    switch (i) {
                        case 0:
                            subjectCode = "INF 250";
                            subject = Game::GameSubject::INF250;
                            break;
                        case 1:
                            subjectCode = "INF 220";
                            subject = Game::GameSubject::INF220;
                            break;
                        case 2:
                            subjectCode = "INF 330";
                            subject = Game::GameSubject::INF330;
                            break;
                        case 3:
                            subjectCode = "INF 332";
                            subject = Game::GameSubject::INF332;
                            break;
                        default:
                            subjectCode = "ERROR";
                            break;
                    }
                    float area2y = heightBorder + buttonHeight/2 + i * (espacamentoY + buttonHeight);
                    auto stage = new StageSelectButton(this, subjectCode, subject);
                    stage->SetPosition(Vector2(area2x, area2y));
                    mButtons.push_back(stage);
                }
                break;
            }

            case 2:
            {
                float area2x = widthBorder + 2.25f * (espacamentoX + buttonWidth);
                for (int i = 0; i < 4; i++)
                {
                    switch (i) {
                        case 0: {
                            subjectCode = "INF 420";
                            subject = Game::GameSubject::INF420;
                            break;
                        }
                        case 1: {
                            subjectCode = "BIO INF";
                            subject = Game::GameSubject::BIOINF;
                            break;
                        }
                        case 2: {
                            subjectCode = "INF 394";
                            subject = Game::GameSubject::INF394;
                            break;
                        }
                        case 3: {
                            subjectCode = "VIS CCP";
                            subject = Game::GameSubject::VISCCP;
                            break;
                        }
                        default:
                            subjectCode = "ERROR";
                            break;
                    }
                    float area2y = heightBorder + buttonHeight/2 + i * (espacamentoY + buttonHeight);
                    auto stage = new StageSelectButton(this, subjectCode, subject);
                    stage->SetPosition(Vector2(area2x, area2y));
                    mButtons.push_back(stage);
                }
                break;
            }
            case 3:
            {
                subjectCode = "TCC";
                subject = Game::GameSubject::TCC;
                auto stage = new StageSelectButton(this, subjectCode, subject);
                stage->SetPosition(Vector2((float)mGame->GetWindowWidth() - widthBorder - buttonWidth/2, (float)mGame->GetWindowHeight()/2));
                mButtons.push_back(stage);
                break;
            }
            default:
                break;
        }
    }

    mButtons[0]->changeSelected();
    mSelectedButton = 0;
}

void StageSelect::DrawUI() {

}

void StageSelect::ProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);
    InputButtonSelect(keyState);

}

void StageSelect::InputButtonSelect(const Uint8 *keyState) {

    int currentSelected = mSelectedButton;
    if(mTimer > TIME_INTERVAL) {

        if(keyState[SDL_SCANCODE_UP])
        {
            if(mSelectedButton == 1)
                mSelectedButton = 4;
            else if(mSelectedButton == 5)
                mSelectedButton = 8;
            else if(!isInBorder())
                mSelectedButton--;
        }
        else if(keyState[SDL_SCANCODE_DOWN])
        {
            if(mSelectedButton == 4)
                mSelectedButton = 1;
            else if(mSelectedButton == 8)
                mSelectedButton = 5;
            else if(!isInBorder())
                mSelectedButton++;
        }
        else if(keyState[SDL_SCANCODE_LEFT])
        {
            if(!isInBorder()){
                mSelectedButton -= 4;
                if(mSelectedButton < 0) mSelectedButton = 0;
            }
            if(mSelectedButton == mNumMaterias - 1)
                mSelectedButton = 6;
        }
        else if(keyState[SDL_SCANCODE_RIGHT])
        {
            if(!isInBorder()) {
                mSelectedButton += 4;
                if(mSelectedButton > mNumMaterias - 1) mSelectedButton = mNumMaterias - 1;
            }
            if(mSelectedButton == 0)
                mSelectedButton = 3;
        }

        mButtons[currentSelected]->changeSelected();
        mButtons[mSelectedButton]->changeSelected();

        mTimer = 0;
    }

    //coloquei o Enter aqui ao invés de no próprio SSButton por questões de otimização.
    if(keyState[SDL_SCANCODE_RETURN])
    {
        mButtons[mSelectedButton]->StartBattle();
    }
}

bool StageSelect::isInBorder() const {
    if(mSelectedButton == 0 || mSelectedButton == mNumMaterias - 1)
        return true;
    return false;
}

void StageSelect::Update(float deltaTime) {

    if(mTimer < TIME_INTERVAL) mTimer += deltaTime;

}
