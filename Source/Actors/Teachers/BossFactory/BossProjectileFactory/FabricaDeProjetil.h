//
// A fabrica de projetil de chefe, unica, dirigida por dados.
//

#pragma once

#include <memory>

#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/ProjetilDeChefe.h"
#include "../../../../Attacks/ProjeteisDeChefe.h"

/// Monta projeteis de chefe conforme uma DescricaoDeProjetil.
class FabricaDeProjetil : public ProjectileFactory {

public:

    FabricaDeProjetil(DescricaoDeProjetil descricao, std::string nome);

    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

    [[nodiscard]] const DescricaoDeProjetil& Descricao() const { return mDescricao; }

private:

    DescricaoDeProjetil mDescricao;
    std::string mNome;

    ProjectilePool<ProjetilDeChefe> mPool;
};
