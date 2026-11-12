//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "Actor.h"
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include "../Motor/Math.h"

// Incluido, e nao apenas declarado: os static_assert de insertMotion e insertModifier usam
// std::is_base_of, que exige tipo completo.
#include "../Attacks/Behaviors.h"

class ProjectileFactory;

/// Classe base abstrata para todos os projéteis do jogo. Define o contrato, o gerenciamento de
/// behaviors e a lógica de colisão.
class Projectile : public Actor {

public:

    explicit Projectile(Scene* scene, Actor* owner);
    ~Projectile() override;

    // 4. Métodos de ciclo de vida que as classes filhas podem implementar.
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor* other) override;

    // 5. Getter para o dono (essencial para o sistema de colisão e behaviors).
    [[nodiscard]] class Actor* GetOwner() const { return mOwner; }
    // Reatribui o dono.
    void SetOwner(Actor* owner) { mOwner = owner; }

    // ---- Object Pooling ----
    [[nodiscard]] ProjectileFactory* GetOriginFactory() const { return mOriginFactory; }
    void SetOriginFactory(ProjectileFactory* factory) { mOriginFactory = factory; }

    // Ponto de entrada publico para matar um projetil de fora da classe (ProjectileManager,
    // Battle).
    void Kill() override;

    // Coloca o objeto no "estado de repouso" exigido de tudo que esta guardado no pool: invisivel,
    // sem colisor, sem velocidade, sem behaviors, Inactive.
    virtual void OnEnterPool();

    // Restaura o projétil para um estado "recém-nascido" antes de ser reutilizado pelo pool.
    virtual void Reset();

    /// Joga fora so a Motion: o projetil segue reto, no rumo e no modulo em que estava, e os
    /// Modifiers continuam.
    void DescartarMotion() { mMotion.reset(); }

    /// Registra a aparencia com que a fabrica construiu este projetil, para que Reset() possa
    /// devolve-la.
    virtual void MarcarPadraoDeFabrica();

    /// Define a Motion do projetil: quem escreve a velocidade.
    void RenovarTempoDeVida() { mTempoDeVida = 0.0f; }

    /// Ha quanto tempo o projetil saiu do pool.
    [[nodiscard]] float TempoDeVida() const { return mTempoDeVida; }

    template <typename M, typename... Args>
    void insertMotion(Args&&... args) {
        static_assert(std::is_base_of_v<ProjectileMotion, M>,
            "insertMotion so aceita ProjectileMotion (Tracking, Wobble, Path). "
            "Para Accelerate/SlowDown/Activate/Deactivate use insertModifier.");
        mMotion = std::make_unique<M>(std::forward<Args>(args)...);
    }

    /// Acrescenta um Modifier: quem modifica a velocidade existente ou agenda quando ela passa a
    /// valer. Podem coexistir varios.
    template <typename M, typename... Args>
    void insertModifier(Args&&... args) {
        static_assert(std::is_base_of_v<ProjectileModifier, M>,
            "insertModifier so aceita ProjectileModifier (Accelerate, SlowDown, "
            "Activate, Deactivate). Para Tracking/Wobble/Path use insertMotion.");
        mModifiers.emplace_back(std::make_unique<M>(std::forward<Args>(args)...));
    }

protected:

    Actor* mOwner;

    // Behavior structures and methods A Motion e unica; os Modifiers se acumulam.
    static constexpr float kTempoDeVidaMaximo = 20.0f;

    float mTempoDeVida = 0.0f;

    /// Aparencia de fabrica, capturada por MarcarPadraoDeFabrica e devolvida por Reset().
    float mEscalaPadrao = 1.0f;
    std::string mAnimacaoPadrao;
    bool mPadraoMarcado = false;

    std::unique_ptr<ProjectileBehavior> mMotion;
    std::vector<std::unique_ptr<ProjectileBehavior>> mModifiers;

    // Sub-funções de OnUpdate.
    [[nodiscard]] virtual bool IsOffScreen() const = 0;

    // Ponto único de decisão de "este projétil morreu" (saiu de tela, colidiu).
    void MarkDead();

private:

    ProjectileFactory* mOriginFactory{};

};

#endif // DPIBULLETHELLRE_PROJECTILE_H.