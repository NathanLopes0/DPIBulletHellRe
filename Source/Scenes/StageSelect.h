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

private:
    class Font* mStageSelectFont;
    int mNumMaterias;
    std::vector<class StageSelectButton*> mButtons;

    void CreateButtons();

    void DrawUI();
};


#endif //DPIBULLETHELLRE_STAGESELECT_H
