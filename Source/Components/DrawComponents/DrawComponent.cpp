//
// Created by nslop on 05/04/2024.
//

#include "DrawComponent.h"
#include "../../Actors/Actor.h"
#include "../../Scenes/Scene.h"
#include "../../Game.h"

DrawComponent::DrawComponent(class Actor *owner, int drawOrder)
        : Component(owner),
        mDrawOrder(drawOrder),
        mIsVisible(true)
{
    mOwner->GetScene()->AddDrawable(this);
}

DrawComponent::~DrawComponent()
{
    mOwner->GetScene()->RemoveDrawable(this);
}

void DrawComponent::Draw(SDL_Renderer *renderer)
{

}
