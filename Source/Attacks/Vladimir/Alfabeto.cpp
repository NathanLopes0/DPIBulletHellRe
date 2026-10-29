//
// O alfabeto do Vladimir, e o que o intercambio faz com ele.
//

#include "Alfabeto.h"

namespace Alfabeto {

    bool EhLetra(const std::string& animacao) {
        return animacao == kA || animacao == kB;
    }

    std::string Trocar(const std::string& animacao) {
        if (animacao == kA) return kB;
        if (animacao == kB) return kA;
        return animacao;
    }

    std::string TrocarPalavra(const std::string& palavra) {
        std::string trocada = palavra;
        for (char& c : trocada) {
            if (c == 'a') c = 'b';
            else if (c == 'b') c = 'a';
        }
        return trocada;
    }

    bool Troca(const std::string& animacao, const std::string& posto, const bool enfeite) {
        return EhLetra(animacao) && posto.empty() && !enfeite;
    }
}
