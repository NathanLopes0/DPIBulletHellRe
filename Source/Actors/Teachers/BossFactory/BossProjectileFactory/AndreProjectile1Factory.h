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

    AndreProjectile1Factory()
        : ProjectileFactory("../Assets/Teachers/Projectiles/DPIBHAndreGraph.png",
                            "../Assets/Teachers/Projectiles/DPIBHAndreGraph.json") {}

    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:
    // mSpritePath/mDataPath privados foram REMOVIDOS daqui: eles sombreavam os
    // membros protegidos da base, entao SetSpritePath() gravava num campo e
    // createProjectile lia outro.
    ProjectilePool<AndreBossProjectile> mPool;
};
