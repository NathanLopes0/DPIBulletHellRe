//
// Created by nslop on 24/07/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECT_H
#define DPIBULLETHELLRE_STAGESELECT_H

#include "Scene.h"
#include <vector>

class StageSelect : public Scene {
public:
    StageSelect(class Game* game);
    void Load() override;
    void ProcessInput(const Uint8* keyState) override;

private:
    class Font* mStageSelectFont;
    std::vector<class StageSelectButton*> button;
    int mButtonSelected, numMat, mTimer;
};


#endif //DPIBULLETHELLRE_STAGESELECT_H
