//
// Created by Claude on 08/10/2026.
//

#pragma once

#include <SDL_scancode.h>
#include <SDL_stdinc.h>

#include <string>
#include <vector>

/**
 * @brief O PAINEL DE CONTROLE do gabinete: DUAS POSICOES, cada uma com um
 * manche e tres botoes. Nao ha mais nada la - nem teclado, nem mouse.
 *
 * Este modulo existe por causa de um erro que ja apareceu quatro vezes neste
 * projeto: a mesma coisa descrita em dois lugares, e uma das copias
 * envelhecendo. Aqui as duas copias eram a TECLA que a cena escuta e o NOME que
 * a cena escreve no rodape. A selecao de fases escutava o T e escrevia "T
 * trocar usuario" - e o gabinete nao tem tecla T, entao a acao era impossivel e
 * o rodape ensinava a fazer uma coisa que ninguem conseguia.
 *
 * Por isso tudo o que se sabe de um controle - a tecla, o nome do botao e o
 * nome da tecla - sai de UMA tabela, e o rodape e montado a partir do botao,
 * nunca de um texto solto.
 *
 * O MANCHE entrou na tabela depois dos botoes, e pelo mesmo motivo. O rodape
 * anunciava "MANCHE (SETAS)" enquanto o jogador tambem andava com W A S D, e a
 * selecao de fases - sozinha entre as cinco telas de menu - nao aceitava W A S
 * D. Tres descricoes do mesmo manche, duas delas erradas.
 *
 * Atalhos de teclado que existem so para quem desenvolve (ENTER, ESC) NAO
 * entram aqui, e por isso nunca aparecem escritos na tela.
 */
namespace Painel {

/**
 * @brief A POSICAO no painel. O gabinete tem duas, uma ao lado da outra.
 *
 * E o motivo de ele ser um gabinete e nao um computador com um joystick. Toda
 * consulta de tecla diz de qual posicao ela e - nao ha tecla "do jogador", so
 * tecla do jogador 1 ou do jogador 2.
 *
 * Ver Documentacao/requisitos-coop.md.
 */
enum class Jogador { Um, Dois };

/// Os botoes que existem de verdade no gabinete. Cada posicao tem os tres.
enum class Botao { Um, Dois, Tres };

/// Para onde o manche esta empurrado. Nao ha diagonal: quem precisa de uma
/// soma as duas, como o jogador faz.
enum class Direcao { Cima, Baixo, Esquerda, Direita };

/// Tudo o que existe, para quem precisa percorrer.
///
/// Estao aqui, ao lado dos enums, para o teste nao manter a propria copia da
/// lista: foi assim que a lista de chefes do teste de dados ficou sem o Thiago
/// e 32 verificacoes passaram um mes sem rodar.
///
/// SAO AINDA ASSIM UMA SEGUNDA DESCRICAO DOS ENUMS, e tem duas protecoes.
/// Esquecer de acrescentar aqui: o teste conta as teclas do painel e acha uma
/// de menos. Acrescentar aqui e esquecer a tabela: o static_assert de
/// Painel.cpp nao compila. O switch tambem aponta o enumerador que ficou de
/// fora, mas como AVISO (-Wswitch), e este projeto nao compila com -Werror.
constexpr Jogador kJogadores[] = {Jogador::Um, Jogador::Dois};
constexpr Botao kBotoes[] = {Botao::Um, Botao::Dois, Botao::Tres};
constexpr Direcao kDirecoes[] = {Direcao::Cima, Direcao::Baixo,
                                 Direcao::Esquerda, Direcao::Direita};

/// A tecla que esta posicao do painel manda por este botao.
[[nodiscard]] SDL_Scancode Tecla(Jogador quem, Botao botao);

/// A tecla que esta posicao manda quando o manche vai para este lado.
[[nodiscard]] SDL_Scancode Tecla(Jogador quem, Direcao direcao);

/// Como o botao se chama NA TELA: "BOTAO 1". NAO depende da posicao - o
/// gabinete tem os botoes 1, 2 e 3 dos dois lados.
[[nodiscard]] const char* Nome(Botao botao);

/// Como a TECLA do botao se chama na tela: "ESPACO". Ver Jeito::ComATecla.
[[nodiscard]] const char* NomeDaTecla(Jogador quem, Botao botao);

/// Como as teclas do manche se chamam na tela, de uma vez: "SETAS", "WASD".
[[nodiscard]] const char* NomeDoManche(Jogador quem);

/// Se o botao esta apertado NESTE quadro. Nao e borda: quem precisa de borda
/// guarda o estado anterior, como as cenas ja fazem.
[[nodiscard]] bool Apertado(const Uint8* teclado, Jogador quem, Botao botao);

/// Se o manche esta empurrado para este lado NESTE quadro.
[[nodiscard]] bool Apertado(const Uint8* teclado, Jogador quem, Direcao direcao);

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
 * O construtor usual recebe a POSICAO e o BOTAO, e nao um texto: e isso que
 * impede o rodape de anunciar uma tecla que a cena nao escuta.
 */
struct Acao {
    Acao(Jogador quem, Botao botao, std::string oQueFaz);

    /// Para o manche, que nao e botao. Ver Painel::Manche.
    Acao(std::string rotulo, std::string tecla, std::string oQueFaz);

    std::string rotulo;   ///< "BOTAO 1", "MANCHE"
    std::string tecla;    ///< "ESPACO", "SETAS" - so aparece em Jeito::ComATecla
    std::string oQueFaz;
};

/// O manche como item de rodape: Manche(Jogador::Um, "escolher") ->
/// "MANCHE  escolher", e com a tecla "MANCHE (SETAS)  escolher".
[[nodiscard]] Acao Manche(Jogador quem, std::string oQueFaz);

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
