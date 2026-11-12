//
// O alfabeto do Vladimir, e o que o intercambio faz com ele.
//

#pragma once

#include <string>

/// O alfabeto e {a, b}, e cada letra e uma animacao do projetil "Simbolo" do Vladimir: o "a" vai
/// reto, o "b" persegue (regras.json).
namespace Alfabeto {

    /// As animacoes das duas letras, em projeteis.json.
    inline constexpr const char* kA = "A";
    inline constexpr const char* kB = "B";

    /// O posto das casas da fita de Turing: o intercambio nao as troca.
    inline constexpr const char* kPostoDaFita = "fita";

    /// O que o placar diz durante o aviso do intercambio, no Vladimir: "Intercambio: a <-> b", com
    /// o acento e a seta de verdade.
    inline constexpr const char* kAvisoDaTroca = "Interc\xC3\xA2mbio: a \xE2\x86\x94 b";

    /// Se a animacao e uma das duas letras.
    bool EhLetra(const std::string& animacao);

    /// A outra letra. Quem nao e letra fica como esta.
    std::string Trocar(const std::string& animacao);

    /// A palavra depois da troca: cada 'a' vira 'b' e cada 'b' vira 'a'. O que nao e letra do
    /// alfabeto fica.
    std::string TrocarPalavra(const std::string& palavra);

    /// Se o intercambio troca este projetil: uma letra, em voo - sem posto, e nao enfeite.
    bool Troca(const std::string& animacao, const std::string& posto, bool enfeite);
}
