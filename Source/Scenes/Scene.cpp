//
// Created by nslop on 04/04/2024.
//

#include "Scene.h"

Scene::Scene(Game* game)
{
    mGame = game;
}

void Scene::Load()
{

}

const Vector2& Scene::GetCameraPos()
{
    return Vector2::Zero;
}

void Scene::ProcessInput(const Uint8 *keyState) {

}