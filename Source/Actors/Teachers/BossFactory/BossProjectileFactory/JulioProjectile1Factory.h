//
// Fábrica de projéteis do Júlio (INF 420)
//

#pragma once

#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/JulioBossProjectile.h"

/**
 * @class JulioProjectile1Factory
 * @brief Cria e recicla os "pontos de dado" do Júlio.
 *
 * Os caminhos dos assets são declarados no construtor, via classe base — não
 * repita strings dentro de createProjectile.
 */
class JulioProjectile1Factory : public ProjectileFactory {
public:

    JulioProjectile1Factory()
        : ProjectileFactory("../Assets/Teachers/Projectiles/DPIBHJulioDado.png",
                            "../Assets/Teachers/Projectiles/DPIBHJulioDado.json") {}

    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;

    void Release(std::unique_ptr<Projectile> projectile) override;

    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:
    ProjectilePool<JulioBossProjectile> mPool;
};
