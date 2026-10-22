#pragma once

#include <cstdint>
#include <string>

#include "AttackParams.h"

/**
 * @brief Uma palavra de bits atravessando a tela: o valor, a altura da fileira
 * e a cara de cada bit.
 *
 * Mesmo padrao da TokenAttackParams: o bloco "palavra" e OPCIONAL em
 * fases.json, e a ponte constroi esta struct para todo ataque PalavraAttack.
 * O VALOR costuma vir do chefe, na hora do disparo (o Ricardo sorteia uma
 * palavra justa para a porta que ela vai atravessar); o arquivo pode fixar um.
 */
struct PalavraAttackParams : AttackParams {

    /// Os bits, a coluna 0 no mais alto (Logica::Bit). Quantos sao e o
    /// numProjectiles do ataque.
    std::uint32_t palavra = 0;

    /// O y da fileira. Negativo = a altura do professor no disparo.
    float altura = -1.0f;

    /// As animacoes do zero e do um.
    std::string zero = "Zero";
    std::string um = "Um";

    /// Quem se atravessa: o zero, que e nivel baixo, e - na fileira de portas,
    /// que nao machuca ninguem - o um tambem.
    bool zeroAtravessa = true;
    bool umAtravessa = false;
};
