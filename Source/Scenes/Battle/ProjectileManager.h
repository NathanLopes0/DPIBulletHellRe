//
// Created by nslop on 15/10/2025.
//

#ifndef DPIBULLETHELLRE_PROJECTILEMANAGER_H
#define DPIBULLETHELLRE_PROJECTILEMANAGER_H

#include <vector>
#include <memory>
#include "../../Motor/Math.h"

class Actor;
class Scene;
class PlayerProjectile;
class BossProjectile;
class Player;

class ProjectileManager {

public:
    explicit ProjectileManager(Scene* owner);
    ~ProjectileManager();

    // ---- Ciclo de Vida ----
    void Update(float deltaTime);

    // ---- Receber Projéteis criados por Bosses e Players ----
    void AddPlayerProjectile(std::unique_ptr<PlayerProjectile> projectile);

    // ---- Função de conveniência (receber vários projectiles de uma vez só) ----
    void AddBossProjectiles(std::vector<std::unique_ptr<BossProjectile>> projectiles);

    // Acesso para usar para colisão.
    [[nodiscard]] const std::vector<std::unique_ptr<PlayerProjectile>>& GetPlayerProjectiles() { return mPlayerProjectiles; }
    [[nodiscard]] const std::vector<std::unique_ptr<BossProjectile>>& GetBossProjectiles() { return mBossProjectiles; }

    void ClearBossProjectiles() const;

    /// Tira de cena os projeteis de um corpo que vai ser destruido. Existe por causa de um crash, e
    /// o crash vale contado.
    void DescartarProjeteisDe(const Actor* dono) const;

    // Executa uma passada de limpeza fora do Update().
    void Cleanup();

private:
    void CleanupProjectiles(); // para remover projéteis mortos

    Scene* mOwnerScene{};

    // ---- O Manager vira dono de todos os projéteis na tela (os Actors passam propriedade pra ele) ----
    std::vector<std::unique_ptr<PlayerProjectile>> mPlayerProjectiles;
    std::vector<std::unique_ptr<BossProjectile>> mBossProjectiles;

};

#endif // DPIBULLETHELLRE_PROJECTILEMANAGER_H.