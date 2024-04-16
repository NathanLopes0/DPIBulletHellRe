//
// Created by nslop on 04/04/2024.
//

#include "Actor.h"
#include "../Game.h"
#include "../Scenes/Scene.h"
#include "../Components/Component.h"

#include <algorithm>

Actor::Actor(Scene* scene) :
    mState(ActorState::Active),
    mPosition(Vector2::Zero),
    mScale(1.0f),
    mRotation(0.0f),
    mScene(scene)
{
    mScene->GetGame()->AddActor(this);
}

Actor::~Actor()
{
    mScene->GetGame()->RemoveActor(this);

    for(auto component : mComponents)
    {
        delete component;
    }
    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        for (auto comp : mComponents)
        {
            if(comp->IsEnabled())
            {
                comp->Update(deltaTime);
            }
        }

        OnUpdate(deltaTime);
    }
}

void Actor::OnUpdate(float deltaTime)
{

}

void Actor::OnCollision()
{

}

void Actor::Kill()
{

}

void Actor::ProcessInput(const Uint8 *keyState)
{
    if(mState == ActorState::Active)
    {
        for(auto comp : mComponents)
        {
            comp->ProcessInput(keyState);
        }

        OnProcessInput(keyState);
    }
}

void Actor::OnProcessInput(const Uint8 *keyState)
{

}

void Actor::AddComponent(Component *c)
{
    mComponents.emplace_back(c);
    std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
        return a->GetUpdateOrder() < b->GetUpdateOrder();
    });
}