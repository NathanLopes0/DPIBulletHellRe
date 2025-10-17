//
// Created by nslop on 04/04/2024.
//


#include <utility>
#include <algorithm>
#include "Scene.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawComponent.h"

Scene::Scene(Game* game, const SceneType sceneType) :
    mGame(game),
    mType(sceneType)
{

}

Actor* Scene::AddActor(std::unique_ptr<Actor> actor) {
    // A lógica de qual vetor usar continua a mesma.
    // A mudança está em como capturamos o ponteiro de retorno.

    if (mIsUpdatingActors) {
        // 1. Adicionamos o ator e guardamos a referência para o unique_ptr recém-criado DENTRO do vetor.
        auto& newActorPtr = mPendingActors.emplace_back(std::move(actor));
        return newActorPtr.get();
    } else {
        auto& newActorPtr = mActors.emplace_back(std::move(actor));
        return newActorPtr.get();
    }
}

void Scene::ProcessInput(const Uint8* keyState) {
    // Delega o input para todos os atores da cena
    for (const auto& actor : mActors) {
        if (actor->GetState() == ActorState::Active) {
            actor->ProcessInput(keyState);
        }
    }

    OnProcessInput(keyState);
}

void Scene::Update(float deltaTime) {

    // Atualiza todos os atores da cena
    mIsUpdatingActors = true;
    for (const auto& actor : mActors) {
        actor->Update(deltaTime);
    }
    mIsUpdatingActors = false;


    for(auto& pending : mPendingActors) {
        mActors.emplace_back(std::move(pending));
    }
    mPendingActors.clear();


    // Remove os atores mortos
    RemoveDeadActors();

    OnUpdate(deltaTime);
}

void Scene::RemoveDeadActors() {
    auto it = std::remove_if(mActors.begin(), mActors.end(),
                             [](const std::unique_ptr<Actor>& actor) {
                                 return actor->GetState() == ActorState::Destroy;
                             });
    mActors.erase(it, mActors.end());
}

void Scene::AddCollider(ColliderComponent* collider) {
    mColliders.emplace_back(collider);
}

void Scene::RemoveCollider(ColliderComponent* collider) {
    auto iter = std::find(mColliders.begin(), mColliders.end(), collider);
    if (iter != mColliders.end()) {
        mColliders.erase(iter);
    }
}

void Scene::AddDrawable(DrawComponent* drawable)
{
    mDrawables.emplace_back(drawable);
    // Opcional, mas recomendado: manter a lista ordenada por draw order
    std::sort(mDrawables.begin(), mDrawables.end(), [](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Scene::RemoveDrawable(DrawComponent* drawable)
{
    if (auto iter = std::find(mDrawables.begin(),
                                            mDrawables.end(), drawable);
                                            iter != mDrawables.end()) {
        mDrawables.erase(iter);
    }
}


void Scene::OnProcessInput(const Uint8* keyState) {}
void Scene::OnUpdate(float deltaTime) {}

