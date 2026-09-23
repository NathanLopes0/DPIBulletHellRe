//
// Combinacoes de behaviors que so fazem sentido juntas.
//

#pragma once

#include "../Actors/Projectile.h"
#include "Behaviors.h"
#include "PathAim.h"

/**
 * @brief Monta a investida repetida num projetil.
 *
 * A investida precisa de duas pecas independentes - uma Motion que re-aponta e
 * um Modifier que pulsa o modulo - derivadas do MESMO ritmo. Separadas elas
 * continuam uteis sozinhas; juntas, e facil entregar ritmos diferentes a cada
 * uma por descuido. Esta funcao remove essa chance: um ritmo, uma chamada.
 *
 * @param ritmo quando comeca, quanto dura cada fase, quantas investidas
 * @param moduloInvestida velocidade durante o mergulho
 * @param moduloPausa velocidade durante a pausa. NAO use zero: a direcao mora
 *        dentro do vetor velocidade, e com modulo zero a mirada seguinte nao
 *        tem o que girar.
 */
inline void AplicarInvestidaRepetida(Projectile* projetil, const RitmoDeCiclos& ritmo,
                                     const float moduloInvestida, const float moduloPausa) {
    if (!projetil) return;
    projetil->insertMotion<MiraPeriodicaBehavior>(ritmo);
    projetil->insertModifier<PulsoDeVelocidadeBehavior>(ritmo, moduloInvestida, moduloPausa);
}
