//
// O modo de dificuldade: Normal ou Dificil.
//

#include "Dificuldade.h"

namespace Dificuldade {

    const char* Nome(const Nivel n) {
        // \xc3\x8d e o I agudo: o MSVC nao le acento em literal.
        switch (n) {
            case Nivel::Normal:  return "NORMAL";
            case Nivel::Dificil: return "DIF\xc3\x8d" "CIL";
        }
        return "";
    }

    const char* Descricao(const Nivel n) {
        switch (n) {
            case Nivel::Normal:  return "As fases como foram pensadas.";
            case Nivel::Dificil: return "Mais tiros, mais r\xc3\xa1pidos e com menos tempo entre eles. "
                                        "A nota vale o mesmo.";
        }
        return "";
    }

    const char* Chave(const Nivel n) {
        switch (n) {
            case Nivel::Normal:  return "normal";
            case Nivel::Dificil: return "dificil";
        }
        return "";
    }

    bool DeChave(const std::string& chave, Nivel& saida) {
        for (const Nivel n : kNiveis) {
            if (chave == Chave(n)) {
                saida = n;
                return true;
            }
        }
        return false;
    }

    Nivel Mover(const Nivel n, const int passo) {
        int i = 0;
        while (i < kQuantos && kNiveis[i] != n) ++i;
        // O resto do C++ pode ser negativo: soma kQuantos ate ficar positivo.
        int j = (i + passo) % kQuantos;
        if (j < 0) j += kQuantos;
        return kNiveis[j];
    }
}
