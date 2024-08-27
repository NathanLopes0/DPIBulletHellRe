//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECTBUTTON_H
#define DPIBULLETHELLRE_STAGESELECTBUTTON_H


#include <string>
#include "Button.h"

class StageSelectButton : public Button {

public:
    StageSelectButton(Scene* scene, const std::string& spritePath, const std::string& buttonText);
    void SetText(const std::string& newText);

private:
    class Font* mFont;
    class DrawSpriteComponent* mDrawComponent;
    class DrawTextComponent* mDrawTextComponent;
};


#endif //DPIBULLETHELLRE_STAGESELECTBUTTON_H
