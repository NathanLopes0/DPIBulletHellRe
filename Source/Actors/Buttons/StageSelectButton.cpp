//
// Created by nslop on 26/08/2024.
//

#include "StageSelectButton.h"
#include "../../Font.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawSpriteComponent.h"

StageSelectButton::StageSelectButton(Scene *scene, const std::string& buttonText) : Button(scene)
{

    mWidth = 128;
    mHeight = 64;

    //-----DRAW ANIMATION COMPONENT-----//
    mDrawComponent = new DrawAnimatedComponent(this, "../Assets/Icons/DPIBHSelectSubjectButton.png",
                                               "../Assets/Icons/DPIBHSelectSubjectButton.json");
    mDrawComponent->AddAnimation("Button",{0});
    mDrawComponent->SetAnimation("Button");
    mDrawComponent->SetAnimFPS(10.0f);

    //-----DRAW TEXT COMPONENT-----//
    mFont = new Font();
    mFont->Load("../Assets/Fonts/Zelda.ttf");
    mDrawTextComponent = new DrawTextComponent(this,buttonText,mFont, mWidth / 2, mHeight / 4);


}


void StageSelectButton::OnUpdate(float deltaTime) {
    //if(isSelected)  mDrawComponent->SetIsPaused(false);
    //else    mDrawComponent->SetIsPaused(true);
}

void StageSelectButton::SetText(const std::string& newText) {
    mDrawTextComponent->SetText(newText);
}
