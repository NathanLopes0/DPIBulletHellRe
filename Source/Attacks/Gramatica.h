//
// A gramatica livre de contexto da fase 2 do Vladimir.
//

#pragma once

#include <cstdint>
#include <string>

/**
 * CAMADA PURA.
 *
 * A FASE 2 DO VLADIMIR E UMA GRAMATICA LIVRE DE CONTEXTO, a classica de aⁿbⁿ:
 *
 *     S -> aSb | e
 *
 * O professor atira o S - a VARIAVEL, a letra que ainda nao e letra - e o S
 * DERIVA EM VOO: depois de kDerivar segundos ele vira tres, um "a" de um lado,
 * um "b" do outro e um S novo no meio, que segue e deriva de novo. Na ultima
 * vez o S vira a palavra vazia e some. Um S que deriva n vezes deixa n letras
 * "a" de um lado e n "b" do outro: e aⁿbⁿ escrito no campo, letra por letra, na
 * ordem em que a derivacao as poe.
 *
 * O ALUNO PODE INTERROMPER A DERIVACAO: o S e derrubavel, e derrubado nao
 * deriva mais. As letras que ele ja soltou ficam.
 *
 * O PLACAR E O ULTIMO PASSO DA DERIVACAO: "aSb -> aaSbb". Quem atira, deriva e
 * escreve no placar e o Vladimir; aqui ficam quantas vezes cada S deriva, a
 * forma sentencial e o texto. Coberto por test_gramatica.cpp.
 */
namespace Gramatica {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaGramatica = "StateTwo";
    constexpr const char* kPostoDaDerivacao = "derivacao";
    constexpr const char* kGatilhoDoPar = "derivar";
    constexpr const char* kGatilhoDaVariavel = "variavel";
    constexpr const char* kCaraDaVariavel = "S";
    constexpr const char* kCaraDaVazia = "Vazia";

    /// Quantos segundos o S voa antes de derivar.
    constexpr float kDerivar = 0.55f;

    /// Quanto a palavra vazia fica na tela antes de sumir: o suficiente para
    /// ser vista, e ela nao machuca ninguem.
    constexpr float kVaziaNaTela = 0.4f;

    /// Quantas vezes um S atirado pelo professor deriva aSb antes do e.
    constexpr int kMenosDerivacoes = 2;
    constexpr int kMaisDerivacoes = 4;

    /// @brief Quantas vezes o S deriva aSb, de kMenosDerivacoes a
    /// kMaisDerivacoes, tirado do 'sorteio'.
    int SortearDerivacoes(std::uint32_t sorteio);

    /// @brief A forma sentencial depois de S -> aSb: o S vira aSb. Sem S, fica.
    std::string Derivar(const std::string& forma);

    /// @brief A forma sentencial depois de S -> e: o S some.
    std::string Esvaziar(const std::string& forma);

    /// @brief O placar: um passo da derivacao, "aSb → aaSbb". A palavra vazia
    /// e escrita epsilon.
    std::string Passo(const std::string& antes, const std::string& depois);

    /// @brief A forma que um S que deriva 'n' vezes deixa: aⁿbⁿ.
    std::string Palavra(int n);
}
