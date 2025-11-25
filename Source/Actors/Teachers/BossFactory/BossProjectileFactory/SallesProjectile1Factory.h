//
// Created by gensh on 18/10/2025.
//

#pragma once

#include <string>
#include "../../../ProjectileFactory.h"


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
    std::unique_ptr<Projectile> createProjectile(Scene* scene, Actor* owner) override;

private:
    std::string mDataPath = "../Assets/Teachers/Projectiles/Capivara.json";
};
