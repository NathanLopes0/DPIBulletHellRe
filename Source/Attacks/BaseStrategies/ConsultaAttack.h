//
// A varredura de uma linha ou coluna da tabela.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class ConsultaAttack
 * @brief Varre UMA faixa da tabela - uma linha ou uma coluna - de ponta a ponta.
 *
 * A IDEIA. O campo de jogo e uma tabela. Uma consulta percorre uma faixa dela, e
 * quem estiver naquela faixa e atingido. O jogador escapa saindo da faixa
 * anunciada, e nao desviando de projetil por projetil.
 *
 * COMO ELA SE ANUNCIA. Os projeteis nascem VISIVEIS e PARADOS na borda de
 * entrada da faixa e so partem depois de `aviso` segundos. Esse intervalo e o
 * ataque inteiro do ponto de vista do jogador: e nele que ele le qual linha foi
 * escolhida e decide para onde correr.
 *
 * POR QUE NAO USA DeactivateBehavior PARA ESPERAR. Seria o jeito obvio - e o que
 * a WaveAttack faz - mas DeactivateBehavior apaga o sprite SEM desligar o
 * colisor. Durante a espera o projetil viraria dano invisivel, exatamente o
 * oposto de um telegrafo. Aqui os projeteis apenas nascem com velocidade zero.
 *
 * ONDE A FAIXA FICA e conta de Tabela, a mesma que o chefe usa para descobrir em
 * que linha o jogador esta. Uma conta so, para o anuncio e a varredura nunca
 * discordarem.
 */
class ConsultaAttack : public IAttackStrategy {
public:
    ConsultaAttack(ProjectileFactory* spawner, Actor* owner);
    ~ConsultaAttack() override = default;

    /**
     * @param params Precisa ser um ConsultaAttackParams. Le dele: forma, eixo,
     *        indice, aviso, invertido, numProjectiles e projectileSpeed.
     *        firePosition e centralAngle sao IGNORADOS - uma consulta nasce da
     *        borda da tabela, nao do chefe.
     */
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
