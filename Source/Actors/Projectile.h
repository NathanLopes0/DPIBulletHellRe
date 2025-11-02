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

    // 6. Getters/Setters para propriedades de gameplay.
    void SetForwardSpeed(float newSpeed);
    [[nodiscard]] float GetForwardSpeed() const;

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

};


#endif //DPIBULLETHELLRE_PROJECTILE_H