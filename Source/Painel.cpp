//
// Created by Claude on 08/10/2026.
//

#include "Painel.h"

#include <utility>

namespace Painel {

SDL_Scancode Tecla(const Botao botao) {
    // AS TRES PRECISAM SER DIFERENTES. Duas acoes na mesma tecla disparariam
    // juntas, e o rodape anunciaria duas coisas para o mesmo aperto. Ha teste.
    switch (botao) {
        case Botao::Um:   return SDL_SCANCODE_SPACE;
        case Botao::Dois: return SDL_SCANCODE_B;
        case Botao::Tres: return SDL_SCANCODE_N;
    }
    return SDL_SCANCODE_UNKNOWN;
}

const char* Nome(const Botao botao) {
    switch (botao) {
        case Botao::Um:   return "BOTAO 1";
        case Botao::Dois: return "BOTAO 2";
        case Botao::Tres: return "BOTAO 3";
    }
    return "";
}

bool Apertado(const Uint8* teclado, const Botao botao) {
    if (!teclado) return false;
    return teclado[Tecla(botao)] != 0;
}

Acao::Acao(const Botao botao, std::string oQueFaz)
    : rotulo(Nome(botao)), oQueFaz(std::move(oQueFaz)) {}

Acao::Acao(std::string rotulo, std::string oQueFaz)
    : rotulo(std::move(rotulo)), oQueFaz(std::move(oQueFaz)) {}

Acao Manche(std::string oQueFaz) {
    return Acao{std::string("MANCHE"), std::move(oQueFaz)};
}

std::string Rodape(const std::vector<Acao>& acoes) {
    std::string linha;
    for (const Acao& acao : acoes) {
        if (!linha.empty()) linha += kEntreAcoes;
        linha += acao.rotulo;
        linha += kEntreNomeEAcao;
        linha += acao.oQueFaz;
    }
    return linha;
}

} // namespace Painel
