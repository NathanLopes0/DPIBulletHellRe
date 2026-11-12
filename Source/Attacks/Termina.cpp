//
// A fase 3 do Luizinho: o programa Termina.
//

#include "Termina.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace Termina {

std::string Codigo(const Programa p) {
    switch (p) {
        case Programa::Contagem:     return "enquanto (n > 0) n = n - 1;";
        case Programa::DeDoisEmDois: return "enquanto (n != 0) n = n - 2;";
        case Programa::Metade:       return "enquanto (n > 1) n = n / 2;";
        case Programa::Crescente:    return "enquanto (n > 0) n = n + 1;";
        case Programa::DeTresEmTres: return "enquanto (n != 10) n = n + 3;";
    }
    return "";
}

std::optional<int> Passo(const Programa p, const int n) {
    switch (p) {
        case Programa::Contagem:     if (n > 0) return n - 1; break;
        case Programa::DeDoisEmDois: if (n != 0) return n - 2; break;
        case Programa::Metade:       if (n > 1) return n / 2; break;
        case Programa::Crescente:    if (n > 0) return n + 1; break;
        case Programa::DeTresEmTres: if (n != 10) return n + 3; break;
    }
    return std::nullopt;
}

bool Para(const Programa p, const int n) {
    switch (p) {
        case Programa::Contagem:     return true;
        case Programa::DeDoisEmDois: return n >= 0 && n % 2 == 0;
        case Programa::Metade:       return true;
        case Programa::Crescente:    return n <= 0;
        case Programa::DeTresEmTres: return n <= 10 && (10 - n) % 3 == 0;
    }
    return true;
}

std::vector<int> Valores(const Programa p, const int n, const int limite) {
    std::vector<int> v{n};
    while (static_cast<int>(v.size()) - 1 < limite) {
        const auto seguinte = Passo(p, v.back());
        if (!seguinte) break;
        v.push_back(*seguinte);
    }
    return v;
}

Rodada Proxima(const std::uint32_t sorteio, const int anterior) {
    std::mt19937 g(sorteio);
    Rodada r;
    int p = static_cast<int>(g() % static_cast<std::uint32_t>(kProgramas));
    if (p == anterior) p = (p + 1 + static_cast<int>(g() % static_cast<std::uint32_t>(kProgramas - 1))) % kProgramas;
    r.programa = static_cast<Programa>(p);
    const auto entre = [&g](const int de, const int ate) { return de + static_cast<int>(g() % static_cast<std::uint32_t>(ate - de + 1)); };
    switch (r.programa) {
        case Programa::Contagem:     r.n = entre(4, 7); break;
        case Programa::DeDoisEmDois: r.n = entre(5, 10); break;
        case Programa::Metade:       r.n = 4 * entre(3, 10); break;
        case Programa::Crescente:    r.n = entre(1, 4); break;
        case Programa::DeTresEmTres: r.n = entre(1, 9); break;
    }
    return r;
}

std::string Pergunta(const Rodada& r) {
    return "Termina(P, " + std::to_string(r.n) + ")";
}

std::string Resposta(const std::optional<bool> veredito) {
    if (!veredito) return "?";
    return *veredito ? "SIM" : "N\xC3\x83O";
}

std::string Placar(const Rodada& r, const std::optional<bool> veredito) {
    return Codigo(r.programa) + "\n" + Pergunta(r) + " = " + Resposta(veredito);
}

std::string PlacarDoParadoxo(const std::optional<bool> veredito, const bool contradicao) {
    if (contradicao) return "Termina n\xC3\xA3o pode existir.\ncontradi\xC3\xA7\xC3\xA3o!";
    const std::string programa = "Paradoxo: se Termina(Paradoxo)\n";
    if (!veredito) return programa + "repete; sen\xC3\xA3o, para.";
    return programa + (*veredito ? "SIM \xE2\x86\x92 o Paradoxo repete" : "N\xC3\x83O \xE2\x86\x92 o Paradoxo para");
}

float DuracaoDaVirada(const int k) {
    return std::max(0.3f, 0.95f * std::pow(0.8f, static_cast<float>(std::max(0, k))));
}

float DuracaoDoParadoxo() {
    float t = kPensando;
    for (int k = 0; k < kViradas; ++k) t += DuracaoDaVirada(k);
    return t;
}

bool VereditoDaVirada(const int k) {
    return k % 2 == 0;
}

Degrau DegrauDo(const Dificuldade::Nivel n) {
    Degrau d;
    if (n != Dificuldade::Nivel::Dificil) return d;
    d.bracos = 16;
    d.porVolta = 0.17f;
    d.velocidade = 205.0f;
    return d;
}

}
