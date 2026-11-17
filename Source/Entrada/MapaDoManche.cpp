//
// Created by Claude on 17/11/2026.
//

#include "MapaDoManche.h"

#include <algorithm>
#include <cstddef>
#include <exception>

#include "../Motor/JsonDeDados.h"

namespace {

// Os bits do direcional, como o SDL_joystick.h os define (SDL_HAT_UP...): repetidos aqui para a
// camada pura nao depender do SDL.
constexpr Uint8 kHatCima = 0x01;
constexpr Uint8 kHatDireita = 0x02;
constexpr Uint8 kHatBaixo = 0x04;
constexpr Uint8 kHatEsquerda = 0x08;

size_t Coluna(const Painel::Direcao d) {
    switch (d) {
        case Painel::Direcao::Cima:     return 0;
        case Painel::Direcao::Baixo:    return 1;
        case Painel::Direcao::Esquerda: return 2;
        case Painel::Direcao::Direita:  return 3;
    }
    return 0;
}

size_t Coluna(const Painel::Botao b) {
    switch (b) {
        case Painel::Botao::Um:   return 0;
        case Painel::Botao::Dois: return 1;
        case Painel::Botao::Tres: return 2;
    }
    return 0;
}

bool BotaoApertado(const MapaDoManche::Leitura& l, const int indice) {
    return indice >= 0 && static_cast<size_t>(indice) < l.botoes.size() && l.botoes[static_cast<size_t>(indice)] != 0;
}

/// O eixo de -1 a 1; 0 se ele nao existe.
float Eixo(const MapaDoManche::Leitura& l, const int indice, const bool inverter) {
    if (indice < 0 || static_cast<size_t>(indice) >= l.eixos.size()) return 0.0f;
    const float v = static_cast<float>(l.eixos[static_cast<size_t>(indice)]) / 32767.0f;
    return inverter ? -v : v;
}

Uint8 Hat(const MapaDoManche::Leitura& l, const int indice) {
    if (indice < 0 || static_cast<size_t>(indice) >= l.hats.size()) return 0;
    return l.hats[static_cast<size_t>(indice)];
}

/// Le um inteiro, se o campo existe; senao fica o que estava.
void Inteiro(const nlohmann::json& j, const char* campo, int& destino, const std::string& onde,
             std::vector<std::string>* problemas) {
    if (!j.contains(campo)) return;
    if (j[campo].is_number_integer()) destino = j[campo].get<int>();
    else if (problemas) problemas->push_back(onde + "." + campo + " nao e um numero inteiro");
}

void Logico(const nlohmann::json& j, const char* campo, bool& destino, const std::string& onde,
            std::vector<std::string>* problemas) {
    if (!j.contains(campo)) return;
    if (j[campo].is_boolean()) destino = j[campo].get<bool>();
    else if (problemas) problemas->push_back(onde + "." + campo + " nao e true nem false");
}

template <size_t N>
void Lista(const nlohmann::json& j, const char* campo, std::array<int, N>& destino, const std::string& onde,
           std::vector<std::string>* problemas) {
    if (!j.contains(campo)) return;
    const nlohmann::json& v = j[campo];
    if (!v.is_array() || v.size() != N ||
        !std::all_of(v.begin(), v.end(), [](const nlohmann::json& x) { return x.is_number_integer(); })) {
        if (problemas) problemas->push_back(onde + "." + campo + " precisa de " + std::to_string(N) + " numeros inteiros");
        return;
    }
    for (size_t i = 0; i < N; ++i) destino[i] = v[i].get<int>();
}

} // namespace

namespace MapaDoManche {

Configuracao Padrao() {
    Configuracao c;
    c.posicoes[1].dispositivo = 1;
    return c;
}

Configuracao Ler(const std::string& texto, std::vector<std::string>* problemas) {
    Configuracao c = Padrao();
    nlohmann::json j;
    try {
        j = LerJsonDeDados(texto);
    } catch (const std::exception& e) {
        if (problemas) problemas->push_back(std::string("o arquivo nao e um JSON valido: ") + e.what());
        return c;
    }
    if (!j.is_object()) {
        if (problemas) problemas->push_back("o arquivo precisa ser um objeto { }");
        return c;
    }
    if (j.contains("zonaMorta")) {
        if (j["zonaMorta"].is_number() && j["zonaMorta"].get<float>() > 0.0f && j["zonaMorta"].get<float>() < 1.0f) {
            c.zonaMorta = j["zonaMorta"].get<float>();
        } else if (problemas) {
            problemas->push_back("zonaMorta precisa ser um numero entre 0 e 1");
        }
    }
    if (!j.contains("jogadores")) return c;
    const nlohmann::json& jogadores = j["jogadores"];
    if (!jogadores.is_array()) {
        if (problemas) problemas->push_back("jogadores precisa ser uma lista");
        return c;
    }
    if (jogadores.size() > c.posicoes.size() && problemas) problemas->push_back("o gabinete tem duas posicoes; o resto da lista foi ignorado");
    for (size_t i = 0; i < jogadores.size() && i < c.posicoes.size(); ++i) {
        const nlohmann::json& p = jogadores[i];
        const std::string onde = "jogadores[" + std::to_string(i) + "]";
        if (!p.is_object()) {
            if (problemas) problemas->push_back(onde + " precisa ser um objeto { }");
            continue;
        }
        Posicao& posicao = c.posicoes[i];
        Inteiro(p, "dispositivo", posicao.dispositivo, onde, problemas);
        Inteiro(p, "eixoX", posicao.eixoX, onde, problemas);
        Inteiro(p, "eixoY", posicao.eixoY, onde, problemas);
        Logico(p, "inverterX", posicao.inverterX, onde, problemas);
        Logico(p, "inverterY", posicao.inverterY, onde, problemas);
        Inteiro(p, "hat", posicao.hat, onde, problemas);
        Lista(p, "botoes", posicao.botoes, onde, problemas);
        Lista(p, "direcoes", posicao.direcoes, onde, problemas);
    }
    return c;
}

bool Empurrado(const Leitura& leitura, const Posicao& posicao, const float zonaMorta, const Painel::Direcao direcao) {
    if (BotaoApertado(leitura, posicao.direcoes[Coluna(direcao)])) return true;
    const Uint8 hat = Hat(leitura, posicao.hat);
    const float x = Eixo(leitura, posicao.eixoX, posicao.inverterX);
    const float y = Eixo(leitura, posicao.eixoY, posicao.inverterY);
    switch (direcao) {
        case Painel::Direcao::Cima:     return y <= -zonaMorta || (hat & kHatCima) != 0;
        case Painel::Direcao::Baixo:    return y >= zonaMorta || (hat & kHatBaixo) != 0;
        case Painel::Direcao::Esquerda: return x <= -zonaMorta || (hat & kHatEsquerda) != 0;
        case Painel::Direcao::Direita:  return x >= zonaMorta || (hat & kHatDireita) != 0;
    }
    return false;
}

bool Apertado(const Leitura& leitura, const Posicao& posicao, const Painel::Botao botao) {
    return BotaoApertado(leitura, posicao.botoes[Coluna(botao)]);
}

void Apertar(const Leitura& leitura, const Posicao& posicao, const float zonaMorta, const Painel::Jogador quem,
             Uint8* teclado, const int quantasTeclas) {
    if (!teclado) return;
    const auto ligar = [&](const SDL_Scancode codigo) {
        if (codigo > SDL_SCANCODE_UNKNOWN && codigo < quantasTeclas) teclado[codigo] = 1;
    };
    for (const Painel::Direcao d : Painel::kDirecoes) {
        if (Empurrado(leitura, posicao, zonaMorta, d)) ligar(Painel::Tecla(quem, d));
    }
    for (const Painel::Botao b : Painel::kBotoes) {
        if (Apertado(leitura, posicao, b)) ligar(Painel::Tecla(quem, b));
    }
}

} // namespace MapaDoManche
