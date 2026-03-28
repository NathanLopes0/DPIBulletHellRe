//
// Created by gensh on 24/03/2026.
//

#ifndef DPIBULLETHELLRE_ANDREBALOONPROJECTILE_H
#define DPIBULLETHELLRE_ANDREBALOONPROJECTILE_H

#include <string>

#include "BossProjectile.h"


class AndreBaloonProjectile : public BossProjectile {

public:

    explicit AndreBaloonProjectile(Scene* scene, Boss* owner);
    void OnUpdate(float deltaTime) override;

private:

    [[nodiscard]] bool IsOffScreen() const override;

};


#endif //DPIBULLETHELLRE_ANDREBALOONPROJECTILE_H