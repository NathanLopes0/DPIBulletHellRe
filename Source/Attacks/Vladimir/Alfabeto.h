//
// O alfabeto do Vladimir, e o que o intercambio faz com ele.
//

#pragma once

#include <string>

/**
 * CAMADA PURA.
 *
 * O ALFABETO E {a, b}, e cada letra e uma animacao do projetil "Simbolo" do
 * Vladimir: o "a" vai reto, o "b" persegue (regras.json). Em toda fase dele as
 * duas letras fazem isso, e e o que o aluno aprende primeiro.
 *
 * O PROGRAMA DE INTERCAMBIO TROCA AS LETRAS TAMBEM. Quando os alunos trocam de
 * lugar (Intercambio.h), toda letra em voo troca de letra - e de comportamento:
 * o "a" que vinha reto passa a perseguir, e o "b" que perseguia segue reto. Na
 * disciplina isso tem nome: e o homomorfismo h(a) = b, h(b) = a, aplicado a
 * tudo o que esta escrito no campo.
 *
 * SO TROCA O QUE ESTA EM VOO: a letra sem posto e que nao e enfeite. As casas
 * da fita de Turing estao no posto da fita e ficam como estao - quem as
 * reescreve e a cabeca da maquina, e nao o intercambio.
 *
 * Quem aplica a troca nos projeteis e o Vladimir (Vladimir::OnIntercambio).
 * Coberto por test_alfabeto.cpp.
 */
namespace Alfabeto {

    /// As animacoes das duas letras, em projeteis.json.
    inline constexpr const char* kA = "A";
    inline constexpr const char* kB = "B";

    /// O posto das casas da fita de Turing: o intercambio nao as troca.
    inline constexpr const char* kPostoDaFita = "fita";

    /// O que o placar diz durante o aviso do intercambio, no Vladimir:
    /// "Intercambio: a <-> b", com o acento e a seta de verdade - escritos em
    /// UTF-8 por escape, como o ponto do pipeline do Ricardo, para nao depender
    /// da codificacao com que o compilador le este arquivo.
    inline constexpr const char* kAvisoDaTroca = "Interc\xC3\xA2mbio: a \xE2\x86\x94 b";

    /// @brief Se a animacao e uma das duas letras.
    bool EhLetra(const std::string& animacao);

    /// @brief A outra letra. Quem nao e letra fica como esta.
    std::string Trocar(const std::string& animacao);

    /// @brief A palavra depois da troca: cada 'a' vira 'b' e cada 'b' vira 'a'.
    /// O que nao e letra do alfabeto fica.
    std::string TrocarPalavra(const std::string& palavra);

    /// @brief Se o intercambio troca este projetil: uma letra, em voo - sem
    /// posto, e nao enfeite.
    bool Troca(const std::string& animacao, const std::string& posto, bool enfeite);
}
