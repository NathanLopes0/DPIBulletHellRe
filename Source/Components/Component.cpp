//
// Created by nslop on 05/04/2024.
//

#include "Component.h"
#include "../Actors/Actor.h"
#include "../Scenes/Scene.h"

Component::Component(Actor* owner, int updateOrder) :
    mOwner(owner),
    mUpdateOrder(updateOrder),
    mIsEnabled(true)
{
    mOwner->AddComponent(this);
}

Component::~Component()
{
}

void Component::Update(float deltaTime)
{
}

void Component::ProcessInput(const Uint8 *keyState)
{
}

Scene* Component::GetScene() const
{
    return mOwner->GetScene();
}