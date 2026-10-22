//
// A geracao de tokens: um por vez, sorteados com temperatura.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class TokenAttack
 * @brief Uma FRASE de 'numProjectiles' tokens, um saindo depois do outro, cada
 * um num angulo SORTEADO entre os candidatos com a temperatura do ataque
 * (Temperatura.h), em volta de centralAngle.
 *
 * A IDEIA. A fase 1 do Julio e um modelo de linguagem gerando texto: cada
 * disparo e uma frase, e cada tiro dela um token, na ordem da frase ("o mo de
 * lo te pre diz"). Fria, a geracao sai sempre no mais provavel - o tiro
 * mirado -, e a frase chega inteira, em fila; quente, ela se espalha, e a
 * frase vira alucinacao.
 *
 * O token i acorda aos i * creationSpeed segundos, como na WaveAttack: a frase
 * nasce inteira em cima do professor e sai escrita, um token por vez. A mira e
 * a do disparo - a frase inteira vai para onde o aluno estava.
 *
 * Le de TokenAttackParams: temperatura, candidatos, abertura, frase. De
 * AttackParams: numProjectiles, projectileSpeed, creationSpeed, centralAngle (a
 * mira, que o professor poe) e firePosition. angle e IGNORADO.
 */
class TokenAttack : public IAttackStrategy {
public:
    TokenAttack(ProjectileFactory* spawner, Actor* owner);
    ~TokenAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
