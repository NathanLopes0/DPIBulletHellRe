//
// Created by nslop on 10/12/2024.
//

#pragma once

#include <memory>
#include <string>

class Projectile;
class Scene;
class Actor;

/**
 * @interface ProjectileFactory
 * @brief Define o contrato para qualquer "fábrica"
 * que saiba como construir um tipo de projétil.
 */
class ProjectileFactory {
public:
    virtual ~ProjectileFactory() = default;

    /**
     * @brief O método principal da fábrica.
     * @param scene A cena onde o projétil será criado.
     * @param owner O ator que "disparou" o projétil (pode ser um Boss ou Player).
     * @return Um unique_ptr para o Projétil recém-criado.
     */
    virtual std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) = 0;
    void SetDataPath(std::string path);
    void SetSpritePath(std::string path);

protected:
    std::string mSpritePath;
    std::string mDataPath;
};