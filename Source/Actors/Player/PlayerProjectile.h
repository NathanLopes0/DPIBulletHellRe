//
// Created by nslop on 19/11/2024.
//

#ifndef DPIBULLETHELLRE_PLAYERPROJECTILE_H
#define DPIBULLETHELLRE_PLAYERPROJECTILE_H

#include <cstdint>

#include "../Projectile.h"

class PlayerProjectile : public Projectile {

    friend class Player;
    friend class Orientador;

public:
    /// O dono e quem atira: o aluno, ou o orientador que luta do lado dele no
    /// TCC. O tiro nasce onde o dono esta, e paga a nota do mesmo jeito.
    PlayerProjectile(class Scene* scene, class Actor* owner);
    void OnUpdate(float deltaTime) override;
    [[nodiscard]] bool IsOffScreen() const override;
    void OnCollision(Actor* other) override;

    /// Um numero que nenhum outro tiro da partida tem. O manager apaga o tiro
    /// que morre, entao quem acompanha um tiro (o golpe do orientador) guarda
    /// isto, e nao o ponteiro, e o procura a cada quadro.
    [[nodiscard]] std::uint64_t Id() const { return mId; }


private:

    Actor* mOwner;
    std::uint64_t mId;

    void ActivateProjectile();
    void DeactivateProjectile();

};


#endif //DPIBULLETHELLRE_PLAYERPROJECTILE_H
