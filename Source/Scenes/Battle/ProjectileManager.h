//
// Created by nslop on 15/10/2025.
//

#ifndef DPIBULLETHELLRE_PROJECTILEMANAGER_H
#define DPIBULLETHELLRE_PROJECTILEMANAGER_H

#include <vector>
#include <memory>
#include "../../Math.h"

class Scene;
class PlayerProjectile;
class BossProjectile;
class Player;

class ProjectileManager {

public:
    explicit ProjectileManager(Scene* owner);
    ~ProjectileManager();

    // --- Ciclo de Vida
    void Update(float deltaTime);

    // --- Receber Projéteis criados por Bosses e Players
    void AddPlayerProjectile(std::unique_ptr<PlayerProjectile> projectile);
    void AddBossProjectile(std::unique_ptr<BossProjectile> projectile);

    // --- Função de conveniência (receber vários projectiles de uma vez só)
    void AddBossProjectiles(std::vector<std::unique_ptr<BossProjectile>> projectiles);

    // Acesso para usar para colisão
    [[nodiscard]] const std::vector<std::unique_ptr<PlayerProjectile>>& GetPlayerProjectiles() { return mPlayerProjectiles; }
    [[nodiscard]] const std::vector<std::unique_ptr<BossProjectile>>& GetBossProjectiles() { return mBossProjectiles; }

private:
    void CleanupProjectiles(); // para remover projéteis mortos

    Scene* mOwnerScene{};

    // --- O Manager vira DONO de todos os projéteis na tela (os Actors passam propriedade pra ele)
    std::vector<std::unique_ptr<PlayerProjectile>> mPlayerProjectiles;
    std::vector<std::unique_ptr<BossProjectile>> mBossProjectiles;


};


#endif //DPIBULLETHELLRE_PROJECTILEMANAGER_H