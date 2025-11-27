//
// Created by gensh on 26/11/2025.
//

#pragma once

#include <string>
#include "../../../ProjectileFactory.h"

/**
 * @class RicardoProjectile1Factory
 * @brief Uma fábrica concreta que sabe como construir
 * o projétil "Arduíno" do Ricardo.
 */

class RicardoProjectile1Factory : public ProjectileFactory {
public:
    /**
     * @brief Cria uma instancia do projétil do Ricardo
     * @param scene A cena onde o projétil será criado
     * @param owner O ator que disparou
     * @return Um ponteiro único para o Projétil recém-criado.
     */

    std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) override;

private:
    std::string mDataPath = "../Assets/Teachers/Projectiles/DPIBHRicardoProjectile.json";
};
