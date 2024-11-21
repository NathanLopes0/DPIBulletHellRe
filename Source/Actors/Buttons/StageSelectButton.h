//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECTBUTTON_H
#define DPIBULLETHELLRE_STAGESELECTBUTTON_H


#include <string>
#include "Button.h"
#include "../../Game.h"

class StageSelectButton : public Button {

public:
    explicit StageSelectButton(Scene* scene, const std::string& buttonText, Game::GameSubject subject);
    void OnUpdate(float deltaTime);
    void SetText(const std::string& newText);
    void StartBattle();

private:

    Game::GameSubject mSubject;
    class Font* mFont;
    class DrawAnimatedComponent* mDrawComponent;
    class DrawTextComponent* mDrawTextComponent;
};


#endif //DPIBULLETHELLRE_STAGESELECTBUTTON_H
