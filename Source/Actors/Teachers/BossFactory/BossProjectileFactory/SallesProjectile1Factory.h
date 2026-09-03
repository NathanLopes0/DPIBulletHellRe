//
// Created by gensh on 18/10/2025.
//

#pragma once

#include <string>
#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/SallesBossProjectile.h"


/**
 * @class SallesProjectile1Factory
 * @brief Uma fábrica concreta que sabe como construir
 * o projétil "Capivara" do Salles.
 */
class SallesProjectile1Factory : public ProjectileFactory {
public:
    /**
     * @brief Cria uma instância do projétil do Salles.
     * @param scene A cena onde o projétil será criado.
     * @param owner O ator que disparou (que esperamos ser o Boss).
     * @return Um ponteiro único para o Projétil recém-criado.
     */
    // O mDataPath aqui era "Capivara.json", arquivo que NAO EXISTE (o correto
    // e DPIBHSallesCapivara.json). Passou despercebido porque createProjectile
    // ignorava os dois membros e repetia os caminhos certos na mao - ou seja,
    // o campo errado nunca chegava a ser lido. Agora que a fabrica le daqui,
    // o caminho esta corrigido.
    SallesProjectile1Factory()
        : ProjectileFactory("../Assets/Teachers/Projectiles/DPIBHSallesCapivara.png",
                            "../Assets/Teachers/Projectiles/DPIBHSallesCapivara.json") {}


    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:
    ProjectilePool<SallesBossProjectile> mPool;
};
