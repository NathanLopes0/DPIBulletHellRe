//
// As batidas dos menus da batalha.
//

#include "LeitorDoMenu.h"

#include "../../Entrada/Painel.h"

LeitorDoMenu::Batidas LeitorDoMenu::Ler(const Uint8* keyState) {
    // Qualquer um dos dois mexe no menu.
    bool cima = false, baixo = false, confirmar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        cima = cima || Painel::Apertado(keyState, quem, Painel::Direcao::Cima);
        baixo = baixo || Painel::Apertado(keyState, quem, Painel::Direcao::Baixo);
        confirmar = confirmar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    confirmar = confirmar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    Batidas b;
    b.cima = mCima.Bateu(cima);
    b.baixo = mBaixo.Bateu(baixo);
    b.confirmar = mConfirmar.Bateu(confirmar);
    return b;
}
