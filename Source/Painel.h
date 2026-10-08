//
// Created by Claude on 08/10/2026.
//

#pragma once

#include <SDL_scancode.h>
#include <SDL_stdinc.h>

#include <string>
#include <vector>

/**
 * @brief O PAINEL DE CONTROLE do gabinete: um manche e tres botoes. Nao ha
 * mais nada la - nem teclado, nem mouse.
 *
 * Este modulo existe por causa de um erro que ja apareceu quatro vezes neste
 * projeto: a mesma coisa descrita em dois lugares, e uma das copias
 * envelhecendo. Aqui as duas copias eram a TECLA que a cena escuta e o NOME que
 * a cena escreve no rodape. A selecao de fases escutava o T e escrevia "T
 * trocar usuario" - e o gabinete nao tem tecla T, entao a acao era impossivel e
 * o rodape ensinava a fazer uma coisa que ninguem conseguia.
 *
 * Por isso tudo o que se sabe de um botao - a tecla, o nome do botao e o nome
 * da tecla - sai de UMA tabela, e o rodape e montado a partir do botao, nunca
 * de um texto solto.
 *
 * Atalhos de teclado que existem so para quem desenvolve (ENTER, ESC) NAO
 * entram aqui, e por isso nunca aparecem escritos na tela.
 */
namespace Painel {

/// Os botoes que existem de verdade no gabinete.
enum class Botao { Um, Dois, Tres };

/// A tecla que o painel manda por este botao.
[[nodiscard]] SDL_Scancode Tecla(Botao botao);

/// Como o botao se chama NA TELA: "BOTAO 1".
[[nodiscard]] const char* Nome(Botao botao);

/// Como a TECLA dele se chama na tela: "ESPACO". Ver Jeito::ComATecla.
[[nodiscard]] const char* NomeDaTecla(Botao botao);

/// Se o botao esta apertado NESTE quadro. Nao e borda: quem precisa de borda
/// guarda o estado anterior, como as cenas ja fazem.
[[nodiscard]] bool Apertado(const Uint8* teclado, Botao botao);

/**
 * @brief Como o rodape nomeia o que apertar.
 *
 * No gabinete nao ha teclado, entao dizer a tecla seria ruido. No computador e
 * o contrario: "BOTAO 2" nao diz nada a quem esta com as maos no teclado, e
 * decorar qual botao e qual tecla so atrapalha quem esta testando.
 *
 * Quem escolhe NAO e a cena: e o Game, que sabe se abriu com --arcade. Ver
 * Game::JeitoDoRodape - uma decisao so, em vez de uma por tela.
 */
enum class Jeito {
    SoOPainel,   ///< "BOTAO 1  jogar"
    ComATecla    ///< "BOTAO 1 (ESPACO)  jogar"
};

/**
 * @brief Um item do rodape: o que apertar e o que isso faz.
 *
 * O construtor usual recebe o BOTAO, e nao um texto: e isso que impede o
 * rodape de anunciar uma tecla que a cena nao escuta.
 */
struct Acao {
    Acao(Botao botao, std::string oQueFaz);

    /// Para o manche, que nao e botao. Ver Painel::Manche.
    Acao(std::string rotulo, std::string tecla, std::string oQueFaz);

    std::string rotulo;   ///< "BOTAO 1", "MANCHE"
    std::string tecla;    ///< "ESPACO", "SETAS" - so aparece em Jeito::ComATecla
    std::string oQueFaz;
};

/// O manche como item de rodape: Manche("escolher") -> "MANCHE  escolher".
[[nodiscard]] Acao Manche(std::string oQueFaz);

/// O espaco entre o nome e o que ele faz.
constexpr const char* kEntreNomeEAcao = "  ";

/// O espaco entre um item e o seguinte. Um so lugar, para os rodapes de todas
/// as telas ficarem iguais.
constexpr const char* kEntreAcoes = "      ";

/**
 * @brief Monta a linha do rodape.
 *
 * ATENCAO AO COMPRIMENTO: o componente de texto quebra em 500 pixels se
 * ninguem disser outra coisa, entao quem desenha este texto precisa chamar
 * SetLarguraDeQuebra com a largura real da caixa. Com Jeito::ComATecla a linha
 * cresce cerca de um terco - a caixa tem de caber nos dois jeitos.
 */
[[nodiscard]] std::string Rodape(const std::vector<Acao>& acoes, Jeito jeito);

} // namespace Painel
