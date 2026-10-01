//
// Created by gensh on 26/11/2025.
//

#pragma once

#include <string>
#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/RicardoBossProjectile.h"

#include "../../../../CaminhosArquivo.h"

/**
 * @class RicardoProjectile1Factory
 * @brief Uma fábrica concreta que sabe como construir
 * o projétil "Arduíno" do Ricardo.
 */

class RicardoProjectile1Factory : public ProjectileFactory {
public:

    RicardoProjectile1Factory()
        : ProjectileFactory(Caminhos::Asset("Teachers/Projectiles/DPIBHRicardoProjectile.png"),
                            Caminhos::Asset("Teachers/Projectiles/DPIBHRicardoProjectile.json")) {}
    /**
     * @brief Cria uma instancia do projétil do Ricardo
     * @param scene A cena onde o projétil será criado
     * @param owner O ator que disparou
     * @return Um ponteiro único para o Projétil recém-criado.
     */

    std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:
    // mDataPath privado REMOVIDO: sombreava o membro da base (ver comentario
    // em AndreProjectile1Factory).
    ProjectilePool<RicardoBossProjectile> mPool;
};
