//
// Created by nslop on 27/08/2024.
//

#pragma once

#include "DrawSpriteComponent.h"
#include <unordered_map>

class DrawAnimatedComponent : public DrawSpriteComponent {

public:
    DrawAnimatedComponent(Actor* owner, const std::string& spriteSheetPath, const std::string& spriteSheetData, int drawOrder = 100);
    ~DrawAnimatedComponent() override;

    void Draw(SDL_Renderer* renderer) override;
    void Update(float deltaTime) override;

    // Change the FPS of the animation.
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    // Set the current active animation.
    void SetAnimation(const std::string& name);

    /// Nome da animacao em curso. Existe para que Projectile::Reset possa devolver a animacao que a
    /// fabrica escolheu.
    [[nodiscard]] const std::string& GetAnimation() const { return mAnimName; }

    // Use to pause/unpause the animation.
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    void AddAnimation(const std::string& name, const std::vector<int>& images);

    /// Troca a folha de sprites desta componente.
    void LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath);

    /// E o padrao de todo projetil que o tiro do aluno destroi (Destrutivel.h); quem liga e
    /// BossProjectile::OnUpdate.
    void DefinirContorno(bool ligado, SDL_Color cor);

private:
    void DesenharContorno(SDL_Renderer* renderer, const SDL_Rect& quadro, const SDL_Rect& destino,
                          double angulo, SDL_RendererFlip flip);

    /// A folha, para a silhueta ser carregada so quando o contorno liga.
    std::string mCaminhoDaFolha;
    bool mContorno = false;
    SDL_Color mCorDoContorno{255, 255, 255, 255};
    SDL_Texture* mSilhueta = nullptr;
    bool mSilhuetaTentada = false;

    // Vector of sprites.
    std::vector<SDL_Rect> mSpriteSheetData;

    // Map of animation names -> vector of textures corresponding to the animation.
    std::unordered_map<std::string,std::vector<int>> mAnimations;

    // Name of the current animation.
    std::string mAnimName;

    // True quando a sprite sheet nao pode ser lida e caimos no frame unico.
    bool mUsingFallbackSheet = false;

    // Current elapsed time in animation.
    float mAnimTimer = 0.0f;

    // The frames per second the animation should run at.
    float mAnimFPS = 10.0f;

    // Whether the animation is paused (defaults to false)
    bool mIsPaused = false;

};
