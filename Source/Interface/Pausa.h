//
// A pausa da batalha, no computador.
//

#pragma once

/// No computador, o ESC pausa a batalha e abre um menu: continuar, recomecar a fase ou sair dela.
namespace Pausa {

    /// As opcoes do menu, na ordem da tela. A primeira e a que abre em foco:
    /// quem apertou ESC sem querer volta com um botao so.
    enum class Opcao { Continuar, Recomecar, Sair };
    constexpr Opcao kOpcoes[] = {Opcao::Continuar, Opcao::Recomecar, Opcao::Sair};
    constexpr int kQuantas = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    /// Como a opcao se chama na tela.
    const char* Texto(Opcao o);

    /// Nunca no gabinete, e so com a luta valendo: carregando os projeteis nao ha o que pausar, e a
    /// tela de fim ja tem o menu dela (Resultado.h).
    bool PodePausar(bool arcade, bool emCombate);

    /// O menu: se esta aberto, e qual opcao esta em foco.
    struct Menu {
        bool aberto = false;
        int selecionada = 0;
    };

    /// Abre com o foco no Continuar, qualquer que tenha sido o da vez anterior.
    void Abrir(Menu& m);
    void Fechar(Menu& m);

    /// Anda 'passo' opcoes, dando a volta nas pontas.
    void Mover(Menu& m, int passo);

    Opcao Escolhida(const Menu& m);

    // ---- A volta ----

    /// Em bullet hell, voltar da pausa direto num campo cheio de tiros e levar um acerto que
    /// ninguem viu chegar.
    constexpr float kRetomada = 3.0f;

    /// O numero que a contagem mostra com 'restante' segundos: 3, 2, 1, e 0 quando acabou (a luta
    /// volta).
    int Contagem(float restante);

    /// Se a janela perder o foco (alguem foi para outra janela), a batalha pausa sozinha - nas
    /// mesmas condicoes do ESC (PodePausar).
    bool PausaAoPerderOFoco(bool arcade, bool emCombate, bool jaPausada);
}
