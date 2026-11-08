//
// A pausa da batalha, no computador.
//

#pragma once

/**
 * CAMADA PURA.
 *
 * NO COMPUTADOR, O ESC PAUSA A BATALHA e abre um menu: continuar, recomecar a
 * fase ou sair dela. Quem joga em casa precisa de um jeito de parar no meio, e
 * quem testa uma fase precisa de um jeito de recomeca-la sem terminar a
 * batalha inteira.
 *
 * NO GABINETE NAO HA PAUSA. O painel nao tem ESC, e um arcade parado com a
 * batalha congelada no corredor seria uma tela morta: quem chega depois nao
 * sabe se o jogo travou. La o Ctrl+Esc segurado continua sendo a saida de quem
 * mantem a maquina (Game::AtualizarGabinete), e nada aqui mexe nele.
 *
 * Quem desenha o menu e congela a batalha e a Battle; aqui ficam a regra de
 * quando pode pausar, as opcoes e a navegacao. Coberto por test_pausa.cpp.
 */
namespace Pausa {

    /// As opcoes do menu, na ordem da tela. A primeira e a que abre em foco:
    /// quem apertou ESC sem querer volta com um botao so.
    enum class Opcao { Continuar, Recomecar, Sair };
    constexpr Opcao kOpcoes[] = {Opcao::Continuar, Opcao::Recomecar, Opcao::Sair};
    constexpr int kQuantas = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    /// Como a opcao se chama na tela.
    const char* Texto(Opcao o);

    /**
     * @brief Se o ESC pode abrir a pausa agora.
     *
     * Nunca no gabinete, e so com a luta valendo: carregando os projeteis nao
     * ha o que pausar, e na tela de fim a nota ja foi registrada - sair dali
     * pelo menu seria so um atalho para a tela que vem 4 segundos depois.
     */
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
}
