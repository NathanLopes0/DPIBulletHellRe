//
// Created by nslop on 10/12/2024.
//

#pragma once

#include <memory>
#include <string>
#include <utility>

class Projectile;
class Scene;
class Actor;

/// Define o contrato para qualquer "fábrica" que saiba como construir um tipo de projétil.
class ProjectileFactory {
public:
    ProjectileFactory() = default;

    // Construtor que toda fabrica concreta deve usar para declarar seus assets.
    ProjectileFactory(std::string spritePath, std::string dataPath)
        : mSpritePath(std::move(spritePath)), mDataPath(std::move(dataPath)) {}

    virtual ~ProjectileFactory() = default;

    /// O método principal da fábrica.
    virtual std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) = 0;

    /// Pede um Projétil pronto para uso, sem parâmetros de posição/cena/owner explícitos por
    /// chamada.
    virtual std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) = 0;

    /// Devolve um Projétil ao pool desta Factory para reuso futuro.
    virtual void Release(std::unique_ptr<Projectile> projectile) = 0;

    virtual void Prewarm(Scene* scene, Actor* owner, int count) = 0;

protected:
    std::string mSpritePath;
    std::string mDataPath;
};