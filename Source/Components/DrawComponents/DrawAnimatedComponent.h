//
// Created by nslop on 27/08/2024.
//

#ifndef DPIBULLETHELLRE_DRAWANIMATEDCOMPONENT_H
#define DPIBULLETHELLRE_DRAWANIMATEDCOMPONENT_H


#include "DrawSpriteComponent.h"
#include <unordered_map>

class DrawAnimatedComponent : public DrawSpriteComponent {

public:
    DrawAnimatedComponent(class Actor* owner, const std::string& spriteSheetPath, const std::string& spriteSheetData, int drawOrder = 100);
    ~DrawAnimatedComponent() override;

    void Draw(SDL_Renderer* renderer) override;
    void Update(float deltaTime) override;

    //Change the FPS of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    //Set the current active animation
    void SetAnimation(const std::string& name);

    //Use to pause/unpause the animation
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    void AddAnimation(const std::string& name, const std::vector<int>& images);


private:
    void LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath);

    //vector of sprites
    std::vector<SDL_Rect*> mSpriteSheetData;

    //Map of animation names -> vector of textures corresponding to the animation
    std::unordered_map<std::string,std::vector<int>> mAnimations;

    //Name of the CURRENT animation
    std::string mAnimName;

    //current elapsed time in animation
    float mAnimTimer = 0.0f;

    // The frames per second the animation should run at
    float mAnimFPS = 10.0f;

    // Whether the animation is paused (defaults to false)
    bool mIsPaused = false;

};


#endif //DPIBULLETHELLRE_DRAWANIMATEDCOMPONENT_H
