//
// Os campos que so a FlexAttack usa.
//

#pragma once

#include "AttackParams.h"
#include "../Flex.h"

/**
 * @brief Uma fileira em flexbox: como ela nasce arrumada, como ela termina, e
 * quando a transicao acontece.
 *
 * Mesmo padrao do ConsultaAttackParams: a FlexAttack faz dynamic_cast, e o
 * bloco "flex" e OBRIGATORIO em fases.json - "de" e "para" nao tem um padrao
 * que faca sentido sozinho.
 */
struct FlexAttackParams : AttackParams {

    Flex::Justificar de   = Flex::Justificar::Centro;
    Flex::Justificar para = Flex::Justificar::EspacoEntre;

    /// Segundos de queda antes de a fileira comecar a se rearranjar.
    float aos = 0.6f;

    /// Quanto dura o rearranjo. Zero e um salto.
    float em = 0.5f;

    /// A largura de cada item, em pixels - a distancia entre dois vizinhos
    /// encostados.
    float item = 44.0f;

    /// A largura do conteiner. Ele e CENTRADO NO CHEFE: e a div dele, e e por
    /// isso que a varredura muda onde as fileiras caem.
    float largura = 760.0f;
};
