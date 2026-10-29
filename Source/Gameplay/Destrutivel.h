//
// O contorno de quem o tiro do aluno destroi.
//

#pragma once

#include <cstdint>

/**
 * CAMADA PURA. Coberto por test_destrutivel.cpp.
 *
 * TODO PROJETIL QUE O TIRO DO ALUNO DESTROI GANHA UM CONTORNO VERMELHO - o
 * processo do Lucas, o agente e o guardrail do Julio, a bala de prata da Maria
 * Lucia, o bloco da pilha do Salles... Antes so o arquivo de dados sabia quem
 * era "derrubavel", e o aluno descobria atirando. Agora e um padrao visual so,
 * igual para todo professor.
 *
 * O contorno segue o desenho: e a SILHUETA do sprite (Game::LoadSilhueta),
 * tingida e desenhada kEspessura pixels deslocada em volta, por baixo dele
 * (DrawAnimatedComponent::DefinirContorno). Quem decide se ele aparece e
 * BossProjectile::OnUpdate, a cada quadro, com Pode - a MESMA regra com que a
 * Battle decide se o tiro derruba o projetil, para o contorno nunca mentir.
 */
namespace Destrutivel {

    /// @brief O tiro do aluno destroi este projetil? So o derrubavel, e nao
    /// quando esta blindado (a historia que o cursor da Maria Lucia arrasta: o
    /// tiro so some nela), nem quando e abrigo (a capivara come o tiro e
    /// fica), nem quando e enfeite (o tiro o atravessa).
    bool Pode(bool derrubavel, bool blindado, bool abrigo, bool enfeite);

    /// A espessura do contorno, em pixels de tela, e a cor: um vermelho que se
    /// le no fundo escuro sem gritar.
    constexpr int kEspessura = 2;
    constexpr std::uint8_t kVermelho = 235;
    constexpr std::uint8_t kVerde = 75;
    constexpr std::uint8_t kAzul = 75;
    constexpr std::uint8_t kOpacidade = 210;

    /// @brief O alfa da silhueta de um pixel com alfa 'a': opaco ou nada. A
    /// borda meio transparente do sprite viraria um contorno borrado.
    constexpr std::uint8_t kLimiarDaSilhueta = 64;
    std::uint8_t AlfaDaSilhueta(std::uint8_t a);
}
