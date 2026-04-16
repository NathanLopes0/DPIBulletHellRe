//
// Created by gensh on 20/03/2026.
//

#ifndef DPIBULLETHELLRE_BALOONATTACK_H
#define DPIBULLETHELLRE_BALOONATTACK_H
#include "../IAttackStrategy.h"
#include "../AttackParameters/BaloonAttackParams.h"
#include "../../Scenes/Battle/Battle.h"


class BaloonAttack : public IAttackStrategy {

public:
    BaloonAttack(ProjectileFactory* projectileFactory, Actor* owner);
    ~BaloonAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
    ProjectileFactory* mSpawner;
    Actor* mOwner;

    static bool ValidateParams(const BaloonAttackParams& params) ;

    static Vector2 CalculateSpawnPosition(
        const BaloonAttackParams& params,
        const Battle* battle
    );

    static Vector2 GetDirectionFromSide( enum
        BaloonAttackParams::side side
    ) ;

    [[nodiscard]] float GetSpawnCoordinateForSide( enum
        BaloonAttackParams::side side,
        const Vector2& screenSize,
        float spriteSize
    ) const;

    static void SpawnBalloonsAtRandom(const BaloonAttackParams& params, Battle* battle, Projectile* projectile);
    static void SpawnBalloonAtPosition(const BaloonAttackParams& params, Projectile* projectile, const Vector2& position);

};


#endif //DPIBULLETHELLRE_BALOONATTACK_H