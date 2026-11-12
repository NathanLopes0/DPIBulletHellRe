//
// Created by Claude on 08/10/2026.
//

#include "Painel.h"

#include <iterator>
#include <utility>

namespace {

/// A tecla de um botao e como ela se chama na tela.
struct TeclaDoBotao {
    SDL_Scancode codigo;
    const char* nome;
};

/// Um posto do painel: um manche e tres botoes, que e tudo o que uma posicao tem. As duas posicoes
/// do gabinete sao duas instancias disto.
struct Posto {
    const char* nomeDoManche;
    SDL_Scancode manche[4];     ///< na ordem de Painel::Direcao
    TeclaDoBotao botoes[3];     ///< na ordem de Painel::Botao
};

/// As duas posicoes, e o unico lugar do projeto onde teclas aparecem. As teclas sao as de um
/// teclado de computador porque e nele que o jogo e testado.
const Posto& Posicao(const Painel::Jogador quem) {
    static const Posto kUm{
        "SETAS",
        {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT},
        {{SDL_SCANCODE_SPACE, "ESPACO"}, {SDL_SCANCODE_B, "B"}, {SDL_SCANCODE_N, "N"}}};

    static const Posto kDois{
        "WASD",
        {SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D},
        {{SDL_SCANCODE_F, "F"}, {SDL_SCANCODE_E, "E"}, {SDL_SCANCODE_H, "H"}}};

    switch (quem) {
        case Painel::Jogador::Um:   return kUm;
        case Painel::Jogador::Dois: return kDois;
    }
    return kUm;
}

/// As listas de Painel.h e as colunas daqui contam a mesma coisa, e e preciso que contem. Esta e a
/// unica amarra dura entre o enum e a tabela.
static_assert(sizeof(Posto::botoes) / sizeof(TeclaDoBotao) == std::size(Painel::kBotoes),
              "a tabela tem uma coluna por botao: ver Painel::kBotoes");
static_assert(sizeof(Posto::manche) / sizeof(SDL_Scancode) == std::size(Painel::kDirecoes),
              "a tabela tem uma coluna por direcao do manche: ver Painel::kDirecoes");

/// A coluna do botao na tabela.
int Coluna(const Painel::Botao botao) {
    switch (botao) {
        case Painel::Botao::Um:   return 0;
        case Painel::Botao::Dois: return 1;
        case Painel::Botao::Tres: return 2;
    }
    return 0;
}

/// A coluna da direcao na tabela, pelo mesmo motivo.
int Coluna(const Painel::Direcao direcao) {
    switch (direcao) {
        case Painel::Direcao::Cima:     return 0;
        case Painel::Direcao::Baixo:    return 1;
        case Painel::Direcao::Esquerda: return 2;
        case Painel::Direcao::Direita:  return 3;
    }
    return 0;
}

} // namespace

namespace Painel {

SDL_Scancode Tecla(const Jogador quem, const Botao botao) {
    return Posicao(quem).botoes[Coluna(botao)].codigo;
}

SDL_Scancode Tecla(const Jogador quem, const Direcao direcao) {
    return Posicao(quem).manche[Coluna(direcao)];
}

const char* Nome(const Botao botao) {
    switch (botao) {
        case Botao::Um:   return "BOTAO 1";
        case Botao::Dois: return "BOTAO 2";
        case Botao::Tres: return "BOTAO 3";
    }
    return "";
}

const char* NomeDaTecla(const Jogador quem, const Botao botao) {
    return Posicao(quem).botoes[Coluna(botao)].nome;
}

const char* NomeDoManche(const Jogador quem) {
    return Posicao(quem).nomeDoManche;
}

bool Apertado(const Uint8* teclado, const Jogador quem, const Botao botao) {
    if (!teclado) return false;
    return teclado[Tecla(quem, botao)] != 0;
}

bool Apertado(const Uint8* teclado, const Jogador quem, const Direcao direcao) {
    if (!teclado) return false;
    return teclado[Tecla(quem, direcao)] != 0;
}

Acao::Acao(const Jogador quem, const Botao botao, std::string oQueFaz)
    : rotulo(Nome(botao)), tecla(NomeDaTecla(quem, botao)), oQueFaz(std::move(oQueFaz)) {}

Acao::Acao(std::string rotulo, std::string tecla, std::string oQueFaz)
    : rotulo(std::move(rotulo)), tecla(std::move(tecla)), oQueFaz(std::move(oQueFaz)) {}

Acao Manche(const Jogador quem, std::string oQueFaz) {
    return Acao{std::string("MANCHE"), std::string(NomeDoManche(quem)), std::move(oQueFaz)};
}

std::string Rodape(const std::vector<Acao>& acoes, const Jeito jeito) {
    std::string linha;
    for (const Acao& acao : acoes) {
        if (!linha.empty()) linha += kEntreAcoes;

        linha += acao.rotulo;
        if (jeito == Jeito::ComATecla && !acao.tecla.empty()) {
            linha += " (" + acao.tecla + ")";
        }
        linha += kEntreNomeEAcao;
        linha += acao.oQueFaz;
    }
    return linha;
}

} // namespace Painel
