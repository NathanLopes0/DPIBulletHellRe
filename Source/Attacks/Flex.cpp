//
// O flexbox do CSS - onde cada item de uma fileira fica, para cada valor de
// justify-content, e a curva da transicao entre dois arranjos.
//

#include "Flex.h"

namespace Flex {

bool PeloNome(const std::string& nome, Justificar& saida) {
    if (nome == "flex-start")    { saida = Justificar::Inicio;        return true; }
    if (nome == "flex-end")      { saida = Justificar::Fim;           return true; }
    if (nome == "center")        { saida = Justificar::Centro;        return true; }
    if (nome == "space-between") { saida = Justificar::EspacoEntre;   return true; }
    if (nome == "space-around")  { saida = Justificar::EspacoEmVolta; return true; }
    return false;
}

std::string Nome(const Justificar j) {
    switch (j) {
        case Justificar::Inicio:        return "flex-start";
        case Justificar::Fim:           return "flex-end";
        case Justificar::Centro:        return "center";
        case Justificar::EspacoEntre:   return "space-between";
        case Justificar::EspacoEmVolta: return "space-around";
    }
    return "flex-start";
}

std::vector<float> Posicoes(const Justificar j, const int n, const float item,
                            const float inicio, const float fim) {

    std::vector<float> x;
    if (n <= 0) return x;
    x.reserve(static_cast<size_t>(n));

    const float linha = fim - inicio;
    const float ocupado = item * static_cast<float>(n);
    const float sobra = linha - ocupado;

    // Onde fica a borda esquerda do primeiro item, e quanto de espaco vai entre
    // dois vizinhos. O centro de cada item sai disso.
    float borda = inicio;
    float vao = 0.0f;
    switch (j) {
        case Justificar::Inicio:
            break;
        case Justificar::Fim:
            borda = fim - ocupado;
            break;
        case Justificar::Centro:
            borda = inicio + sobra / 2.0f;
            break;
        case Justificar::EspacoEntre:
            vao = (n > 1) ? sobra / static_cast<float>(n - 1) : 0.0f;
            break;
        case Justificar::EspacoEmVolta:
            vao = sobra / static_cast<float>(n);
            borda = inicio + vao / 2.0f;
            break;
    }

    for (int i = 0; i < n; ++i) {
        x.push_back(borda + item / 2.0f + static_cast<float>(i) * (item + vao));
    }
    return x;
}

float Progresso(const float t, const float atraso, const float duracao) {
    if (t <= atraso) return 0.0f;
    if (duracao <= 0.0f || t >= atraso + duracao) return 1.0f;
    const float u = (t - atraso) / duracao;
    return u * u * (3.0f - 2.0f * u);
}

bool Junto(const Justificar j) {
    return j == Justificar::Inicio || j == Justificar::Fim || j == Justificar::Centro;
}

Justificar Proxima(Variacao::Sorteio& s, const Justificar de) {
    if (Junto(de)) return s.Chance(0.5f) ? Justificar::EspacoEntre : Justificar::EspacoEmVolta;
    const Justificar juntos[] = {Justificar::Inicio, Justificar::Fim, Justificar::Centro};
    return juntos[s.Inteiro(0, 2)];
}




}
