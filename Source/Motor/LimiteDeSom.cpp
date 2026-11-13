//
// O limite dos efeitos sonoros.
//

#include "LimiteDeSom.h"

namespace LimiteDeSom {
    bool Controle::PodeTocar(const std::string& grupo, const std::uint32_t agoraMs, const int tocando,
                             const Regra& regra) {
        if (tocando >= regra.simultaneos) return false;
        if (const auto it = mUltimoComeco.find(grupo); it != mUltimoComeco.end()) {
            // Sem conta negativa: um relogio que volta (outra batalha, outro jogo) nao trava o grupo.
            if (agoraMs >= it->second && agoraMs - it->second < regra.intervaloMs) return false;
        }
        mUltimoComeco[grupo] = agoraMs;
        return true;
    }
}
