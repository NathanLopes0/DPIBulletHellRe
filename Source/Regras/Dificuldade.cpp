//
// O modo de dificuldade: Normal ou Dificil.
//

#include "Dificuldade.h"

namespace Dificuldade {

    const char* Nome(const Nivel n) {
        switch (n) {
            case Nivel::Normal:  return "NORMAL";
            case Nivel::Dificil: return "DIFÍCIL";
        }
        return "";
    }

    const char* Descricao(const Nivel n) {
        switch (n) {
            case Nivel::Normal:  return "As fases como foram pensadas.";
            case Nivel::Dificil: return "Mais tiros e mais rápidos, e um acerto mal protege do próximo. "
                                        "A nota vale o mesmo.";
        }
        return "";
    }

    const char* Vantagem(const Nivel n) {
        switch (n) {
            case Nivel::Normal:  return "";
            case Nivel::Dificil: return "Passar no Difícil conta para liberar as optativas.";
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

    Nivel Mover(const Nivel n, const int passo) {
        int i = 0;
        while (i < kQuantos && kNiveis[i] != n) ++i;
        // O resto do C++ pode ser negativo: soma kQuantos ate ficar positivo.
        int j = (i + passo) % kQuantos;
        if (j < 0) j += kQuantos;
        return kNiveis[j];
    }
}
