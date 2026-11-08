//
// A pausa da batalha, no computador.
//

#include "Pausa.h"

namespace Pausa {

    const char* Texto(const Opcao o) {
        // \xc3\xa7 e o c cedilha: o MSVC nao le acento em literal.
        switch (o) {
            case Opcao::Continuar: return "Continuar";
            case Opcao::Recomecar: return "Recome\xc3\xa7" "ar a fase";
            case Opcao::Sair:      return "Sair da fase";
        }
        return "";
    }

    bool PodePausar(const bool arcade, const bool emCombate) {
        return !arcade && emCombate;
    }

    void Abrir(Menu& m) {
        m.aberto = true;
        m.selecionada = 0;
    }

    void Fechar(Menu& m) {
        m.aberto = false;
    }

    void Mover(Menu& m, const int passo) {
        // O resto de um negativo e negativo em C++: soma kQuantas antes.
        const int r = (m.selecionada + passo) % kQuantas;
        m.selecionada = r < 0 ? r + kQuantas : r;
    }

    Opcao Escolhida(const Menu& m) {
        if (m.selecionada < 0 || m.selecionada >= kQuantas) return Opcao::Continuar;
        return kOpcoes[m.selecionada];
    }
}
