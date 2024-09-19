//
// Created by nslop on 04/04/2024.
//

#include "Scene.h"
#include "../Game.h"

Scene::Scene(Game* game, SceneType sceneType)
{
    mGame = game;
    mSceneType = sceneType;
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

void Scene::SetScene(Scene::SceneType sceneType) {
    mGame->SetScene(sceneType);
}

void Scene::Update(float deltaTime) {

}
