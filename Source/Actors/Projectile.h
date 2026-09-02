//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "Actor.h"
#include <vector>
#include <memory>
#include "../Math.h"

class ProjectileBehavior;
class ProjectileFactory;

/**
 * @class Projectile
 * @brief Classe base abstrata para todos os projéteis do jogo.
 * Define o contrato, o gerenciamento de behaviors e a lógica de colisão.
 */
class Projectile : public Actor {

public:

    explicit Projectile(Scene* scene, Actor* owner);
    ~Projectile() override;

    // 4. Métodos de ciclo de vida que as classes filhas podem implementar.
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor* other) override;

    // 5. Getter para o dono (essencial para o sistema de colisão e behaviors).
    [[nodiscard]] class Actor* GetOwner() const { return mOwner; }
    // Reatribui o dono. Necessário para Object Pooling: um projétil reciclado
    // pode ser reutilizado por um ataque disparado por outro owner (ex: mesmo Boss,
    // mas o parâmetro owner pode variar entre chamadas de Acquire()).
    void SetOwner(Actor* owner) { mOwner = owner; }

    // 6. Getters/Setters para propriedades de gameplay.
    void SetForwardSpeed(float newSpeed);
    [[nodiscard]] float GetForwardSpeed() const;

    // --- Object Pooling ---
    // A ProjectileFactory que originalmente construiu este objeto (via createProjectile).
    // Setado uma única vez, no Acquire(), e usado pelo ProjectileManager para saber
    // para qual Factory devolver o objeto quando ele for marcado como Inactive.
    // Fica nullptr para projéteis que não passam por pooling (ex: PlayerProjectile).
    [[nodiscard]] ProjectileFactory* GetOriginFactory() const { return mOriginFactory; }
    void SetOriginFactory(ProjectileFactory* factory) { mOriginFactory = factory; }

    // Restaura o projétil para um estado "recém-nascido" antes de ser reutilizado
    // pelo pool. Limpa o que é comum a todo Projectile (behaviors, velocidade,
    // estado). Classes filhas devem chamar Projectile::Reset() e então
    // complementar com o que for específico delas (ex: reiniciar animação).
    virtual void Reset();

    template <typename Behavior, typename... Args>
    void insertBehavior(Args &&... args) {
        mBehaviors.push_back(std::make_unique<Behavior>(std::forward<Args>(args)...));
    }

protected:

    Actor* mOwner;
    float mForwardSpeed;

    //Behavior structures and methods
    std::vector<std::unique_ptr<ProjectileBehavior>> mBehaviors;

    //Sub-funções de OnUpdate
    [[nodiscard]] virtual bool IsOffScreen() const = 0;

    // Ponto único de decisão de "este projétil morreu" (saiu de tela, colidiu).
    // Se este objeto tem uma factory de origem (foi obtido via
    // ProjectileFactory::Acquire, isto é, participa de Object Pooling),
    // marca Inactive e ESCONDE o sprite (SetIsVisible(false)) para que o
    // ProjectileManager o devolva ao pool sem deixar um "fantasma" visível
    // na última posição onde ele morreu. Caso contrário (ex: PlayerProjectile,
    // que nunca passa por uma ProjectileFactory), marca Destroy como sempre foi feito.
    void MarkDead();

private:

    ProjectileFactory* mOriginFactory{};

};


#endif //DPIBULLETHELLRE_PROJECTILE_H