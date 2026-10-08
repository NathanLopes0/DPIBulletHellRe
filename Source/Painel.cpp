//
// Created by Claude on 08/10/2026.
//

#include "Painel.h"

#include <utility>

namespace {

/// TUDO O QUE SE SABE DE UM BOTAO, numa linha so.
///
/// Eram tres switches sobre o mesmo enum - a tecla, o nome do botao e o nome da
/// tecla. Tres lugares para acertar ao mudar um botao de tecla, e nada avisando
/// se um ficasse para tras; e justamente o erro que este modulo existe para
/// impedir. Ha teste exigindo que as tres colunas sejam distintas entre botoes.
struct Descricao {
    SDL_Scancode tecla;
    const char* nome;
    const char* nomeDaTecla;
};

const Descricao& Descreve(const Painel::Botao botao) {
    static const Descricao kUm{SDL_SCANCODE_SPACE, "BOTAO 1", "ESPACO"};
    static const Descricao kDois{SDL_SCANCODE_B, "BOTAO 2", "B"};
    static const Descricao kTres{SDL_SCANCODE_N, "BOTAO 3", "N"};
    static const Descricao kNenhum{SDL_SCANCODE_UNKNOWN, "", ""};

    switch (botao) {
        case Painel::Botao::Um:   return kUm;
        case Painel::Botao::Dois: return kDois;
        case Painel::Botao::Tres: return kTres;
    }
    return kNenhum;
}

} // namespace

namespace Painel {

SDL_Scancode Tecla(const Botao botao) { return Descreve(botao).tecla; }

const char* Nome(const Botao botao) { return Descreve(botao).nome; }

const char* NomeDaTecla(const Botao botao) { return Descreve(botao).nomeDaTecla; }

bool Apertado(const Uint8* teclado, const Botao botao) {
    if (!teclado) return false;
    return teclado[Tecla(botao)] != 0;
}

Acao::Acao(const Botao botao, std::string oQueFaz)
    : rotulo(Nome(botao)), tecla(NomeDaTecla(botao)), oQueFaz(std::move(oQueFaz)) {}

Acao::Acao(std::string rotulo, std::string tecla, std::string oQueFaz)
    : rotulo(std::move(rotulo)), tecla(std::move(tecla)), oQueFaz(std::move(oQueFaz)) {}

Acao Manche(std::string oQueFaz) {
    return Acao{std::string("MANCHE"), std::string("SETAS"), std::move(oQueFaz)};
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
