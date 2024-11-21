//
// Created by nslop on 21/08/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECT_H
#define DPIBULLETHELLRE_STAGESELECT_H

#include "../Game.h"
#include "../Scenes/Scene.h"

class StageSelect : public Scene {

public:
    explicit StageSelect(Game* game);

    void Load() override;
    void ProcessInput(const Uint8* keyState) override;
    void Update(float deltaTime) override;
    int GetSelected() const { return mSelectedButton; }

private:
    class Font* mStageSelectFont;
    int mNumMaterias, mSelectedButton;
    float mTimer;
    std::vector<class StageSelectButton*> mButtons;

    void CreateButtons();
    void DrawUI();
    void InputButtonSelect(const Uint8 *keyState);
    [[nodiscard]] bool isInBorder() const;
};


#endif //DPIBULLETHELLRE_STAGESELECT_H
