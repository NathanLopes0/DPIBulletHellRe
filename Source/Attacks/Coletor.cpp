//
// O coletor de lixo: memento mori, "lembra-te de que morreras".
//

#include "Coletor.h"

#include <cmath>

namespace Coletor {

Grafo Referencias(const std::vector<Vector2>& blocos, const Vector2 raiz, const float alcance) {
    Grafo g;
    const int n = static_cast<int>(blocos.size());
    g.vivo.assign(blocos.size(), false);
    const float alcance2 = alcance * alcance;

    auto perto = [&](const Vector2& a, const Vector2& b) {
        return (a - b).LengthSq() <= alcance2;
    };

    // As arestas: da raiz, e de bloco com bloco.
    std::vector<int> fila;
    for (int i = 0; i < n; ++i) {
        if (perto(raiz, blocos[static_cast<size_t>(i)])) {
            g.arestas.emplace_back(kRaiz, i);
            g.vivo[static_cast<size_t>(i)] = true;
            fila.push_back(i);
        }
    }
    std::vector<std::vector<int>> vizinhos(blocos.size());
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (!perto(blocos[static_cast<size_t>(i)], blocos[static_cast<size_t>(j)])) continue;
            g.arestas.emplace_back(i, j);
            vizinhos[static_cast<size_t>(i)].push_back(j);
            vizinhos[static_cast<size_t>(j)].push_back(i);
        }
    }

    // A MARCACAO: quem a raiz alcanca, por qualquer caminho.
    for (size_t k = 0; k < fila.size(); ++k) {
        for (const int v : vizinhos[static_cast<size_t>(fila[k])]) {
            if (g.vivo[static_cast<size_t>(v)]) continue;
            g.vivo[static_cast<size_t>(v)] = true;
            fila.push_back(v);
        }
    }
    return g;
}

bool Valido(const Relogio& r) {
    return r.aCada > 0.0f && r.pausa >= 0.0f && r.pausa < r.aCada;
}

Mundo Estado(const Relogio& r) {
    return (r.t >= r.aCada - r.pausa) ? Mundo::Parado : Mundo::Rodando;
}

Evento Avancar(Relogio& r, const float dt) {
    if (!Valido(r)) return Evento::Nenhum;

    const Mundo antes = Estado(r);
    r.t += dt;

    if (antes == Mundo::Rodando) {
        if (Estado(r) == Mundo::Parado) {
            // O mundo parou. Se o passo passou do fim do ciclo, a pausa fica
            // inteira para os quadros seguintes: varrer sem ter parado seria
            // um coletor que ninguem viu trabalhar.
            if (r.t >= r.aCada) r.t = r.aCada - r.pausa;
            return Evento::Para;
        }
        return Evento::Nenhum;
    }

    if (r.t >= r.aCada) {
        r.t = std::fmod(r.t - r.aCada, r.aCada);
        if (Estado(r) == Mundo::Parado) r.t = 0.0f;   // o proximo ciclo comeca rodando
        return Evento::Varre;
    }
    return Evento::Nenhum;
}

}
