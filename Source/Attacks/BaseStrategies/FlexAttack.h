//
// O flexbox do CSS: uma fileira que cai arrumada de um jeito e, no meio da
// queda, se rearranja de outro.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class FlexAttack
 * @brief Uma fileira de projeteis que troca de justify-content no meio da queda.
 *
 * A IDEIA. A fase 2 do Lucas e o CSS, e o CSS nao cria nada: ele REARRANJA o que
 * ja esta na pagina. Cada disparo e uma fileira que nasce arrumada segundo "de"
 * - todos juntos no meio, por exemplo - e cai reta. Depois de "aos" segundos,
 * em "em" segundos, cada item escorrega ate onde "para" o poria: os juntos se
 * espalham ate as bordas, ou os espalhados se juntam.
 *
 * O LUGAR SEGURO TROCA NO MEIO DA QUEDA. Com "center" as laterais estao livres;
 * depois de "space-between" o livre passa a ser o vao entre dois itens. Quem
 * leu a regra no placar sabe para onde a fileira vai se abrir.
 *
 * O CONTEINER E CENTRADO NO CHEFE, com "largura" pixels: e a div dele. Itens que
 * caem fora da tela simplesmente saem.
 *
 * Le de FlexAttackParams: de, para, aos, em, item, largura. De AttackParams:
 * numProjectiles, projectileSpeed (a queda, sempre para baixo) e firePosition.
 * angle e centralAngle sao IGNORADOS.
 */
class FlexAttack : public IAttackStrategy {
public:
    FlexAttack(ProjectileFactory* spawner, Actor* owner);
    ~FlexAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
