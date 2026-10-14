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

    //Change the FPS of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    //Set the current active animation
    void SetAnimation(const std::string& name);

    /**
     * @brief Nome da animacao em curso.
     *
     * Existe para que Projectile::Reset possa devolver a animacao que a fabrica
     * escolheu. Sem isto, um projetil reciclado sairia do pool com a animacao do
     * ataque anterior.
     */
    [[nodiscard]] const std::string& GetAnimation() const { return mAnimName; }

    //Use to pause/unpause the animation
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    void AddAnimation(const std::string& name, const std::vector<int>& images);

    /**
     * @brief Troca a folha de sprites desta componente.
     *
     * Era privada e so o construtor chamava. Virou publica para a tela de
     * criacao de personagem, que recompoe a sprite a cada seta apertada e
     * precisa troca-la sem recriar o ator.
     *
     * SUBSTITUI os quadros, nao acrescenta - ver o comentario na implementacao.
     * As animacoes registradas por AddAnimation CONTINUAM valendo, entao a folha
     * nova precisa ter o mesmo recorte da anterior.
     */
    void LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath);

private:

    //vector of sprites
    std::vector<SDL_Rect> mSpriteSheetData;

    //Map of animation names -> vector of textures corresponding to the animation
    std::unordered_map<std::string,std::vector<int>> mAnimations;

    //Name of the CURRENT animation
    std::string mAnimName;

    // true quando a sprite sheet nao pode ser lida e caimos no frame unico.
    bool mUsingFallbackSheet = false;

    //current elapsed time in animation
    float mAnimTimer = 0.0f;

    // The frames per second the animation should run at
    float mAnimFPS = 10.0f;

    // Whether the animation is paused (defaults to false)
    bool mIsPaused = false;

};
