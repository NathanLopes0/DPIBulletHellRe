//
// O modo daltonismo: as cores do jogo corrigidas.
//

#include "Daltonismo.h"

#include <algorithm>
#include <string>

namespace {

    Daltonismo::Modo gModo = Daltonismo::Modo::Desligado;

    /// A simulacao de Machado, Oliveira e Fernandes (2009), severidade total, linha a linha.
    struct Matriz {
        float m[3][3];
    };
    const Matriz& Da(const Daltonismo::Modo m) {
        static const Matriz kProtanopia{{{0.152286f, 1.052583f, -0.204868f},
                                         {0.114503f, 0.786281f, 0.099216f},
                                         {-0.003882f, -0.048116f, 1.051998f}}};
        static const Matriz kDeuteranopia{{{0.367322f, 0.860646f, -0.227968f},
                                           {0.280085f, 0.672501f, 0.047413f},
                                           {-0.011820f, 0.042940f, 0.968881f}}};
        static const Matriz kTritanopia{{{1.255528f, -0.076749f, -0.178779f},
                                         {-0.078411f, 0.930809f, 0.147602f},
                                         {0.004733f, 0.691367f, 0.303900f}}};
        static const Matriz kIdentidade{{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
        switch (m) {
            case Daltonismo::Modo::Protanopia:   return kProtanopia;
            case Daltonismo::Modo::Deuteranopia: return kDeuteranopia;
            case Daltonismo::Modo::Tritanopia:   return kTritanopia;
            case Daltonismo::Modo::Desligado:    break;
        }
        return kIdentidade;
    }

    void Simulado(const Daltonismo::Rgb c, const Daltonismo::Modo m, float& r, float& g, float& b) {
        const Matriz& x = Da(m);
        const float e[3] = {static_cast<float>(c.r), static_cast<float>(c.g), static_cast<float>(c.b)};
        r = x.m[0][0] * e[0] + x.m[0][1] * e[1] + x.m[0][2] * e[2];
        g = x.m[1][0] * e[0] + x.m[1][1] * e[1] + x.m[1][2] * e[2];
        b = x.m[2][0] * e[0] + x.m[2][1] * e[1] + x.m[2][2] * e[2];
    }

    unsigned char Canal(const float v) {
        return static_cast<unsigned char>(std::clamp(v + 0.5f, 0.0f, 255.0f));
    }
}

namespace Daltonismo {

    Rgb Simular(const Rgb c, const Modo m) {
        if (m == Modo::Desligado) return c;
        float r = 0.0f, g = 0.0f, b = 0.0f;
        Simulado(c, m, r, g, b);
        return {Canal(r), Canal(g), Canal(b)};
    }

    Rgb Corrigir(const Rgb c, const Modo m) {
        if (m == Modo::Desligado) return c;
        float r = 0.0f, g = 0.0f, b = 0.0f;
        Simulado(c, m, r, g, b);
        // O erro (o que o olho perde) vai para os canais que ele ve.
        const float er = static_cast<float>(c.r) - r;
        const float eg = static_cast<float>(c.g) - g;
        const float eb = static_cast<float>(c.b) - b;
        if (m == Modo::Tritanopia) {
            // Sem o azul, um pouco do erro dele sai do verde e volta ao azul: leve, porque o olho
            // tritanope ja separa bem o vermelho do verde, e o par da dupla (azul e laranja) nao
            // pode se aproximar.
            return {c.r, Canal(c.g - 0.2f * eb), Canal(c.b + 0.1f * eb)};
        }
        // Sem o vermelho ou o verde, o erro do vermelho vai para o verde e o azul (Fidaner).
        return {c.r, Canal(c.g + 0.7f * er + eg), Canal(c.b + 0.7f * er + eb)};
    }

    const char* Nome(const Modo m) {
        switch (m) {
            case Modo::Desligado:    return "desligado";
            case Modo::Deuteranopia: return "deuteranopia";
            case Modo::Protanopia:   return "protanopia";
            case Modo::Tritanopia:   return "tritanopia";
        }
        return "desligado";
    }

    const char* NomeNoArquivo(const Modo m) {
        return m == Modo::Desligado ? "nao" : Nome(m);
    }

    bool DoArquivo(const char* nome, Modo& saida) {
        const std::string s = nome ? nome : "";
        if (s == "nao") { saida = Modo::Desligado; return true; }
        for (int k = 1; k < kModos; ++k) {
            const Modo m = static_cast<Modo>(k);
            if (s == Nome(m)) { saida = m; return true; }
        }
        return false;
    }

    Modo Mover(const Modo m, const int passo) {
        const int k = ((static_cast<int>(m) + passo) % kModos + kModos) % kModos;
        return static_cast<Modo>(k);
    }

    Modo Atual() { return gModo; }
    void Definir(const Modo m) { gModo = m; }
}
