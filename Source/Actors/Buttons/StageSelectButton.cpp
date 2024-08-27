//
// Created by nslop on 26/08/2024.
//

#include "StageSelectButton.h"
#include "../../Font.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/DrawSpriteComponent.h"

StageSelectButton::StageSelectButton(Scene *scene, const std::string& spritePath, const std::string& buttonText) : Button(scene) {

    mDrawComponent = new DrawSpriteComponent(this, spritePath);
    mFont = new Font();
    mFont->Load("../Assets/Fonts/Zelda.ttf");
    mDrawTextComponent = new DrawTextComponent(this,buttonText,mFont,mWidth,mHeight);

}

void StageSelectButton::SetText(const std::string& newText) {
    mDrawTextComponent->SetText(newText);
}
