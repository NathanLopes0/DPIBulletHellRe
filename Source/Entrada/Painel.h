//
// Created by Claude on 08/10/2026.
//

#pragma once

#include <SDL_scancode.h>
#include <SDL_stdinc.h>

#include <string>
#include <vector>

/// O painel de controle do gabinete: duas posicoes, cada uma com um manche e tres botoes. Nao ha
/// mais nada la - nem teclado, nem mouse.
namespace Painel {

/// A posicao no painel. O gabinete tem duas, uma ao lado da outra.
enum class Jogador { Um, Dois };

/// Os botoes que existem de verdade no gabinete. Cada posicao tem os tres.
enum class Botao { Um, Dois, Tres };

/// Para onde o manche esta empurrado. Nao ha diagonal: quem precisa de uma
/// soma as duas, como o jogador faz.
enum class Direcao { Cima, Baixo, Esquerda, Direita };

/// Tudo o que existe, para quem precisa percorrer.
constexpr Jogador kJogadores[] = {Jogador::Um, Jogador::Dois};
constexpr Botao kBotoes[] = {Botao::Um, Botao::Dois, Botao::Tres};
constexpr Direcao kDirecoes[] = {Direcao::Cima, Direcao::Baixo,
                                 Direcao::Esquerda, Direcao::Direita};

/// A tecla que esta posicao do painel manda por este botao.
[[nodiscard]] SDL_Scancode Tecla(Jogador quem, Botao botao);

/// A tecla que esta posicao manda quando o manche vai para este lado.
[[nodiscard]] SDL_Scancode Tecla(Jogador quem, Direcao direcao);

/// Como o botao se chama na tela: "BOTAO 1". Nao depende da posicao - o gabinete tem os botoes 1, 2
/// e 3 dos dois lados.
[[nodiscard]] const char* Nome(Botao botao);

/// Como a tecla do botao se chama na tela: "ESPACO".
[[nodiscard]] const char* NomeDaTecla(Jogador quem, Botao botao);

/// Como as teclas do manche se chamam na tela, de uma vez: "SETAS", "WASD".
[[nodiscard]] const char* NomeDoManche(Jogador quem);

/// Se o botao esta apertado neste quadro. Nao e borda: quem precisa de borda guarda o estado
/// anterior, como as cenas ja fazem.
[[nodiscard]] bool Apertado(const Uint8* teclado, Jogador quem, Botao botao);

/// Se o manche esta empurrado para este lado neste quadro.
[[nodiscard]] bool Apertado(const Uint8* teclado, Jogador quem, Direcao direcao);

/// Como o rodape nomeia o que apertar. No gabinete nao ha teclado, entao dizer a tecla seria ruido.
enum class Jeito {
    SoOPainel,   ///< "BOTAO 1  jogar"
    ComATecla    ///< "BOTAO 1 (ESPACO)  jogar"
};

/// Um item do rodape: o que apertar e o que isso faz.
struct Acao {
    Acao(Jogador quem, Botao botao, std::string oQueFaz);

    /// Para o manche, que nao e botao.
    Acao(std::string rotulo, std::string tecla, std::string oQueFaz);

    std::string rotulo;   ///< "BOTAO 1", "MANCHE"
    std::string tecla;    ///< "ESPACO", "SETAS" - so aparece em Jeito::ComATecla.
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

/// Monta a linha do rodape.
[[nodiscard]] std::string Rodape(const std::vector<Acao>& acoes, Jeito jeito);

} // namespace Painel
