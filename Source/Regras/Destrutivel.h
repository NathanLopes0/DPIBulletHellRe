//
// O contorno de quem o tiro do aluno destroi.
//

#pragma once

#include <cstdint>

/// Todo projetil que o tiro do aluno destroi ganha um contorno vermelho.
namespace Destrutivel {

    /// O tiro do aluno destroi este projetil?
    bool Pode(bool derrubavel, bool blindado, bool abrigo, bool enfeite);

    /// A espessura do contorno, em pixels de tela, e a cor: um vermelho que se
    /// le no fundo escuro sem gritar.
    constexpr int kEspessura = 2;
    constexpr std::uint8_t kVermelho = 235;
    constexpr std::uint8_t kVerde = 75;
    constexpr std::uint8_t kAzul = 75;
    constexpr std::uint8_t kOpacidade = 210;

    /// O alfa da silhueta de um pixel com alfa 'a': opaco ou nada. A borda meio transparente do
    /// sprite viraria um contorno borrado.
    constexpr std::uint8_t kLimiarDaSilhueta = 64;
    std::uint8_t AlfaDaSilhueta(std::uint8_t a);
}
