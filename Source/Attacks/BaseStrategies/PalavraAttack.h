//
// Uma palavra de bits, de lado a lado da tela.
//

#pragma once

#include "../Engine/IAttackStrategy.h"

/**
 * @class PalavraAttack
 * @brief Uma fileira de 'numProjectiles' bits, um por coluna, de uma borda da
 * tela a outra, descendo juntos a 'projectileSpeed'.
 *
 * A IDEIA. Na fase 1 do Ricardo o dado e uma palavra de 16 bits que desce como
 * uma parede: o UM e nivel alto, aceso, e machuca; o ZERO e nivel baixo, e se
 * atravessa - ele sai como enfeite (BossProjectile::DefinirEnfeite). A tela e
 * dividida em colunas iguais (Logica::CentroDaColuna), e a coluna 0 e o bit mais
 * alto, a esquerda: a palavra se le na tela como se escreve.
 *
 * A mesma estrategia monta a FILEIRA DE PORTAS, parada (velocidade zero) e toda
 * de enfeite: o "valor" dela e a entrada B, e quem troca a cara de cada porta
 * e o Ricardo.
 *
 * Le de PalavraAttackParams: palavra, altura, zero, um, zeroAtravessa,
 * umAtravessa. De AttackParams: numProjectiles (os bits), projectileSpeed e
 * firePosition (a altura, quando a do bloco e negativa). angle e centralAngle
 * sao IGNORADOS: a fileira sempre desce reta.
 */
class PalavraAttack : public IAttackStrategy {
public:
    PalavraAttack(ProjectileFactory* spawner, Actor* owner);
    ~PalavraAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
