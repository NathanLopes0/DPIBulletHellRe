//
// Created by nslop on 04/04/2024.
//

#pragma once

#include <SDL_stdinc.h>
#include <memory>
#include <SDL_rect.h>
#include <vector>

class Actor;
class Game;
class DrawComponent;
class ColliderComponent;

class Scene {
public:
    enum class SceneType
    {
        MainMenu,
        StageSelect,
        Battle
    };

    explicit Scene(Game* game, SceneType sceneType);
    virtual ~Scene() = default;


    void ProcessInput(const Uint8* keyState);
    void Update(float deltaTime);

    virtual void Load() = 0;

    [[nodiscard]] Game* GetGame() const { return mGame; }
    [[nodiscard]] SceneType GetType() const { return mType; }
    [[nodiscard]] const std::vector<DrawComponent*>& GetDrawables() const { return mDrawables; }


    void AddCollider(ColliderComponent* collider);
    void RemoveCollider(ColliderComponent* collider);
    std::vector<ColliderComponent*>& GetColliders() { return mColliders; }

    void AddDrawable(DrawComponent* drawable);
    void RemoveDrawable(DrawComponent* drawable);

    [[nodiscard]] virtual SDL_FRect GetPlayfieldBounds() const;

protected:
    // As classes filhas vão dar override nestas para adicionar
    // sua lógica específica. Elas têm um corpo vazio por padrão.
    virtual void OnProcessInput(const Uint8* keyState);
    virtual void OnUpdate(float deltaTime);

    Actor* AddActor(std::unique_ptr<Actor> actor);
    void RemoveDeadActors();

    Game* mGame;
    SceneType mType;

    // A cena é DONA de todos os atores que vivem nela
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<std::unique_ptr<Actor>> mPendingActors;

    bool mIsUpdatingActors{};

    // Lista de observadores para desenho.
    std::vector<DrawComponent*> mDrawables;

    //Lista de observadores para colisores
    std::vector<ColliderComponent*> mColliders;
};
