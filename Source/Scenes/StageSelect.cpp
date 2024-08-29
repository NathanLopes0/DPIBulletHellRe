//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "../Actors/Buttons/StageSelectButton.h"
#include "../Font.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect),
    mNumMaterias(10)
{
    mStageSelectFont = new Font();
    mStageSelectFont->Load("../Assets/Fonts/Zelda.ttf");

    mButtons.reserve(mNumMaterias);
    StageSelect::Load();

}

void StageSelect::Load() {

    SDL_Log("Loading Stage Select");
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
/*    for (int j = 0; j < 4; j++)
    {
        switch(j)
        {
            case 0:
            {
                subjectCode = "INF 213";
                auto stage = new StageSelectButton(this, subjectCode);
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
                            break;
                        case 1:
                            subjectCode = "INF 220";
                            break;
                        case 2:
                            subjectCode = "INF 330";
                            break;
                        case 3:
                            subjectCode = "INF 332";
                            break;
                        default:
                            subjectCode = "ERROR";
                            break;
                    }
                    float area2y = heightBorder + buttonHeight/2 + i * (espacamentoY + buttonHeight);
                    auto stage = new StageSelectButton(this, subjectCode);
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
                            break;
                        }
                        case 1: {
                            subjectCode = "BIO INF";
                            break;
                        }
                        case 2: {
                            subjectCode = "INF 394";
                            break;
                        }
                        case 3: {
                            subjectCode = "VIS CCP";
                            break;
                        }
                        default:
                            subjectCode = "ERROR";
                            break;
                    }
                    float area2y = heightBorder + buttonHeight/2 + i * (espacamentoY + buttonHeight);
                    auto stage = new StageSelectButton(this, subjectCode);
                    stage->SetPosition(Vector2(area2x, area2y));
                    mButtons.push_back(stage);
                }
                break;
            }
            case 3:
            {
                subjectCode = "TCC";
                auto stage = new StageSelectButton(this, subjectCode);
                stage->SetPosition(Vector2(mGame->GetWindowWidth() - widthBorder - buttonWidth/2, mGame->GetWindowHeight()/2));
                mButtons.push_back(stage);
                break;
            }
            default:
                break;
        }
    }
    mButtons[0]->changeSelected();
 */
    subjectCode = "INF 213";
    auto stage = new StageSelectButton(this, subjectCode);
    mButtons.push_back(stage);
    stage->SetPosition(Vector2((float)mGame->GetWindowWidth()/2, (float)mGame->GetWindowHeight()/2));

}

void StageSelect::DrawUI() {

}

void StageSelect::ProcessInput(const Uint8 *keyState) {

}
