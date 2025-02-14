//
// Created by nslop on 04/04/2024.
//


#include "../Game.h"
#include "Scene.h"
#include <utility>
#include "Battle.h"
#include "../Actors/Teacher/Boss.h"

Scene::Scene(Game* game, SceneType sceneType)
{
    mGame = game;
    mSceneType = sceneType;
    mBossAllProjectiles.clear();
    mBossAllProjectiles.reserve(500);
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
    mBossAllProjectiles.clear();
    mGame->SetScene(sceneType);
}

void Scene::Update(float deltaTime) {

}

Boss* Scene::GetBoss() const {
    return nullptr;
}

void Scene::SetBossProjectiles(std::vector<Projectile *> vector1) {
    mBossAllProjectiles = std::move(vector1);
}