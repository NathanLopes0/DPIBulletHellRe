//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECTBUTTON_H
#define DPIBULLETHELLRE_STAGESELECTBUTTON_H


#include <string>
#include "Button.h"
#include "../../Game.h"

class Font;
class StageSelectButton : public Button {

public:
    explicit StageSelectButton(Scene* scene, const std::string& buttonText,
                               Game::GameSubject subject, const std::string& fontPath);
    ~StageSelectButton() override = default;

    void OnUpdate(float deltaTime) override;
    void SetText(const std::string& newText);

    [[nodiscard]] Game::GameSubject GetSubject() const { return mSubject; }

private:

    Game::GameSubject mSubject{};
    std::unique_ptr<Font> mFont;
};


#endif //DPIBULLETHELLRE_STAGESELECTBUTTON_H
