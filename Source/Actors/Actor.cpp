//
// Created by nslop on 04/04/2024.
//

#include "Actor.h"
#include "../Game.h"
#include "../Scenes/Scene.h"
#include "../Components/Component.h"

Actor::Actor(Scene* scene) :
    mState(ActorState::Active),
    mPosition(Vector2::Zero),
    mScale(1.0f),
    mRotation(0.0f),
    mScene(scene)
{

}

void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        for (const auto& comp : mComponents)
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
        for(const auto& comp : mComponents)
        {
            comp->ProcessInput(keyState);
        }

        OnProcessInput(keyState);
    }
}

void Actor::OnProcessInput(const Uint8 *keyState)
{

}