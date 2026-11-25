//
// O que os arquivos do Ferreira (Ferreira*.cpp, um por fase) dividem: as contas pequenas e os
// textos.
//

#pragma once

#include <cmath>
#include <string>

#include "../../../Attacks/JogosDigitais.h"

namespace FerreiraComum {

    using Plataforma::Lado;

    inline float Mistura(const float a, const float b, const float u) {
        return a + (b - a) * u;
    }

    inline std::string Inteiro(const float x) {
        return std::to_string(static_cast<int>(std::lround(x)));
    }

    /// Se pisca aceso em 't', a cada 'periodo' segundos.
    inline bool Aceso(const float t, const float periodo) {
        return static_cast<int>(std::floor(t / periodo)) % 2 == 0;
    }

    inline std::string NomeDoLado(const Lado l) {
        switch (l) {
            case Lado::Baixo:    return "CHÃO";
            case Lado::Esquerda: return "ESQUERDA";
            case Lado::Cima:     return "TETO";
            case Lado::Direita:  return "DIREITA";
        }
        return "";
    }

    /// O que o placar diz num lado: para onde a gravidade puxa, e o que fazer nele.
    inline std::string AnuncioDoLado(const Lado l) {
        switch (Gravidade::DesafioDo(l)) {
            case Gravidade::Desafio::Pulos:   return "gravidade = CHÃO: PULE as caixas (segure para ir alto)";
            case Gravidade::Desafio::Lancas:  return "gravidade = ESQUERDA: lança de baixo, PULE; de cima, FIQUE";
            case Gravidade::Desafio::Chuva:   return "gravidade = TETO: fuja dos blocos e PULE as fagulhas";
            case Gravidade::Desafio::Esteira: return "gravidade = DIREITA: ande CONTRA a esteira e pule os espinhos";
        }
        return "";
    }
}
