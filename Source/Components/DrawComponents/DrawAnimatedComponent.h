//
// Created by nslop on 23/07/2024.
//

#ifndef DPIBULLETHELLRE_DRAWANIMATEDCOMPONENT_H
#define DPIBULLETHELLRE_DRAWANIMATEDCOMPONENT_H

#include "DrawSpriteComponent.h"
#include <unordered_map>

class DrawAnimatedComponent : public DrawSpriteComponent {
public:

    // Lower draw order corresponds with furter back
    DrawAnimatedComponent(class Actor *owner, const std::string& spriteSheetPath,
                        const std::string &spriteSheetData,
                        int drawOrder = 100);

    ~DrawAnimatedComponent() override;

    void Draw(SDL_Renderer* renderer) override;
    void Update(float deltaTime) override;


    //change the fps of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    void SetAnimation(const std::string &name);

    void SetIsPaused(bool pause) {mIsPaused = pause; }

    void AddAnimation(const std::string& name,
                      const std::vector<int>& images);

private:

    void LoadSpriteSheet(const std::string& texturePath,
                         const std::string& dataPath);

    // Vetor de Sprites
    std::vector<SDL_Rect*> mSpriteSheetData;

    // Map of [animation name] to [vector of textures corresponding to the
    // animation]
    std::unordered_map<std::string, std::vector<int>> mAnimations;

    // Name of current animation
    std::string mAnimName;

    // Tempo passado na animação
    float mAnimTimer = 0.0f;

    // frames por segundo que a animação deve correr
    float mAnimFPS = 10.0f;

    // Se a animação está ou não pausada
    bool mIsPaused = false;
};


#endif //DPIBULLETHELLRE_DRAWANIMATEDCOMPONENT_H
