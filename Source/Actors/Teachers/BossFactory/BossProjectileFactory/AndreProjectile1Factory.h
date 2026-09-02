//
// Created by gensh on 27/11/2025.
//

#pragma once
#include <memory>
#include <string>

#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/AndreBossProjectile.h"


class Projectile;
class Scene;
class Actor;
/**
 * @class AndreProjectile1Factory
 * @brief Uma fábrica concreta que sabe como construir
 * o projétil "Grafo" do Andre.
 */
class AndreProjectile1Factory : public ProjectileFactory {
public:

    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:
    std::string mSpritePath = "../Assets/Teachers/Projectiles/DPIBHAndreGraph.png";
    std::string mDataPath = "../Assets/Teachers/Projectiles/DPIBHAndreGraph.json";

    ProjectilePool<AndreBossProjectile> mPool;
};
