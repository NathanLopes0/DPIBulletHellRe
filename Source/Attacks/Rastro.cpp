//
// A serpente: sequi, "seguir".
//

#include "Rastro.h"

#include <cmath>

namespace Rastro {

void Caminho::Marcar(const Vector2 ponto, const bool salto) {
    if (mPontos.empty()) { mPontos.push_back({ponto, 0.0f, false}); return; }
    const Ponto& ultimo = mPontos.back();
    const float s = salto ? ultimo.s : ultimo.s + (ponto - ultimo.p).Length();
    mPontos.push_back({ponto, s, salto});
}

float Caminho::Comprimento() const {
    return mPontos.empty() ? 0.0f : mPontos.back().s;
}

size_t Caminho::Trecho(const float s) const {
    // O trecho i vai do ponto i-1 ao ponto i: e o primeiro cujo comeco ja nao
    // passa de 's'. Procura de tras para frente, porque os gomos estao sempre
    // perto do fim do caminho. A desigualdade ESTRITA decide o empate do
    // salto: o ponto de onde a cabeca saltou e o ponto onde ela caiu tem o
    // mesmo comprimento, e o gomo que chega ali ja esta do lado de la.
    size_t i = mPontos.size() - 1;
    while (i > 0 && mPontos[i - 1].s > s) --i;
    return i;
}

Vector2 Caminho::Atras(const float distancia) const {
    if (mPontos.empty()) return Vector2::Zero;
    const float s = Comprimento() - distancia;
    if (s <= 0.0f) return mPontos.front().p;

    const size_t i = Trecho(s);
    const Ponto& b = mPontos[i];
    if (i == 0) return b.p;
    const Ponto& a = mPontos[i - 1];
    // O SALTO NAO TEM MEIO: o trecho de um salto tem comprimento zero, e quem
    // esta nele ja esta do outro lado.
    if (b.salto || b.s <= a.s) return b.p;
    const float t = (s - a.s) / (b.s - a.s);
    return a.p + (b.p - a.p) * t;
}

Vector2 Caminho::DirecaoAtras(const float distancia) const {
    if (mPontos.size() < 2) return Vector2(0.0f, 1.0f);
    const float s = std::fmax(Comprimento() - distancia, 0.0f);
    size_t i = Trecho(s);
    if (i == 0) i = 1;
    // Um trecho de salto nao tem direcao: vale a do trecho seguinte, que e
    // para onde o gomo vai depois de saltar.
    while (i + 1 < mPontos.size() && (mPontos[i].salto || mPontos[i].s <= mPontos[i - 1].s)) ++i;
    Vector2 d = mPontos[i].p - mPontos[i - 1].p;
    const float n = d.Length();
    return (n > 0.0f) ? d * (1.0f / n) : Vector2(0.0f, 1.0f);
}

Vector2 Girar(const Vector2 direcao, const Vector2 paraOAlvo, const float giroMaximo) {
    const float atual = std::atan2(direcao.y, direcao.x);
    const float alvo = std::atan2(paraOAlvo.y, paraOAlvo.x);
    constexpr float kPi = 3.14159265f;
    float diferenca = alvo - atual;
    while (diferenca > kPi) diferenca -= 2.0f * kPi;
    while (diferenca <= -kPi) diferenca += 2.0f * kPi;
    const float passo = std::fmax(-giroMaximo, std::fmin(giroMaximo, diferenca));
    const float novo = atual + passo;
    return Vector2(std::cos(novo), std::sin(novo));
}

std::string Romano(int n) {
    if (n < 1 || n > 3999) return "";
    static const struct { int valor; const char* letras; } kTabela[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"},
        {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
    };
    std::string r;
    for (const auto& t : kTabela) {
        while (n >= t.valor) { r += t.letras; n -= t.valor; }
    }
    return r;
}

}
