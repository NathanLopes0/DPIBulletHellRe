//
// Created by nslop on 04/04/2024.
//

#include <utility>
#include <algorithm>
#include "Scene.h"
#include "../Motor/Game.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawComponent.h"

Scene::Scene(Game* game, const SceneType sceneType) :
    mGame(game),
    mType(sceneType)
{

}

Actor* Scene::AddActor(std::unique_ptr<Actor> actor) {
    // A lógica de qual vetor usar continua a mesma.

    if (mIsUpdatingActors) {
        // 1.
        auto& newActorPtr = mPendingActors.emplace_back(std::move(actor));
        return newActorPtr.get();
    } else {
        auto& newActorPtr = mActors.emplace_back(std::move(actor));
        return newActorPtr.get();
    }
}

void Scene::ProcessInput(const Uint8* keyState) {
    // Delega o input para todos os atores da cena.
    if (!mAtoresCongelados) {
        for (const auto& actor : mActors) {
            if (actor->GetState() == ActorState::Active) {
                actor->ProcessInput(keyState);
            }
        }
    }

    OnProcessInput(keyState);
}

void Scene::Update(float deltaTime) {

    // Atualiza todos os atores da cena - menos com ela congelada (Scene.h).
    mIsUpdatingActors = true;
    if (!mAtoresCongelados) {
        for (const auto& actor : mActors) {
            actor->Update(deltaTime);
        }
    }
    mIsUpdatingActors = false;

    for(auto& pending : mPendingActors) {
        mActors.emplace_back(std::move(pending));
    }
    mPendingActors.clear();

    // A logica especifica da cena roda antes da remocao, com todos os atores ainda vivos.
    OnUpdate(deltaTime);

    // Remove os atores mortos - ultimo passo do frame.
    RemoveDeadActors();
}

void Scene::RemoveDeadActors() {
    auto it = std::remove_if(mActors.begin(), mActors.end(),
                             [](const std::unique_ptr<Actor>& actor) {
                                 return actor->GetState() == ActorState::Destroy;
                             });
    mActors.erase(it, mActors.end());
}

void Scene::AddDrawable(DrawComponent* drawable)
{
    mDrawables.emplace_back(drawable);

    // Estavel, e nao std::sort.
    std::stable_sort(mDrawables.begin(), mDrawables.end(), [](DrawComponent* a, DrawComponent* b) {
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

SDL_FRect Scene::GetPlayfieldBounds() const
{
    // Por padrão, a área é a tela inteira.
    return {
        0.0f,
        0.0f,
        static_cast<float>(mGame->GetWindowWidth()),
        static_cast<float>(mGame->GetWindowHeight())
    };
}

