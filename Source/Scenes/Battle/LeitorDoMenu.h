//
// As batidas dos menus da batalha: a pausa e o resultado.
//

#pragma once

#include <SDL.h>

#include "../../Entrada/Borda.h"

/// Cima, baixo e confirmar, de qualquer um dos dois paineis (e o Enter, de quem testa no teclado).
/// So o que acabou de ser apertado neste quadro conta.
class LeitorDoMenu {
public:
    struct Batidas {
        bool cima = false;
        bool baixo = false;
        bool confirmar = false;
    };

    Batidas Ler(const Uint8* keyState);

    /// O que ja esta apertado agora nao conta: quem vinha segurando o manche ou o tiro quando o menu
    /// abriu nao anda nele por isso.
    void Segurar() { mCima.Segurar(); mBaixo.Segurar(); mConfirmar.Segurar(); }

private:
    Borda mCima{true};
    Borda mBaixo{true};
    Borda mConfirmar{true};
};
