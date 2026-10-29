//
// O Ponto Extra do co-op: o feixe ate o parceiro e o escudo em volta dele.
//

#include "PontoExtra.h"

#include <algorithm>

namespace PontoExtra {

    bool NoEscudo(const Vector2 centro, const Vector2 projetil, const float raio) {
        const float alcance = kRaioDoEscudo + std::max(raio, 0.0f);
        return (projetil - centro).LengthSq() <= alcance * alcance;
    }

    std::vector<std::pair<Vector2, Vector2>> Anel(const Vector2 centro, const float raio, const int lados) {
        std::vector<std::pair<Vector2, Vector2>> segmentos;
        const int n = std::max(lados, 3);
        segmentos.reserve(static_cast<size_t>(n));
        auto ponto = [&](const int k) {
            const float a = Math::TwoPi * static_cast<float>(k) / static_cast<float>(n);
            return centro + Vector2(Math::Cos(a), Math::Sin(a)) * raio;
        };
        for (int k = 0; k < n; ++k) segmentos.emplace_back(ponto(k), ponto(k + 1));
        return segmentos;
    }

    Feixe FeixeEm(const Vector2 de, const Vector2 ate, const float t) {
        Feixe f;
        if (t < 0.0f || !FeixeAceso(t)) return f;
        if (t < kViagemDoFeixe) {
            const float u = t / kViagemDoFeixe;
            f.cabeca = true;
            f.cabecaDe = Vector2::Lerp(de, ate, std::max(0.0f, u - kTamanhoDoFeixe));
            f.cabecaAte = Vector2::Lerp(de, ate, u);
            f.rastro = 1.0f;
        } else {
            f.rastro = 1.0f - (t - kViagemDoFeixe) / kRastroDoFeixe;
        }
        return f;
    }

    bool FeixeAceso(const float t) {
        return t < kViagemDoFeixe + kRastroDoFeixe;
    }

    int TirosNaSobrecarga(const float sobrecarga, const float entreTiros) {
        if (entreTiros <= 0.0f || sobrecarga <= 0.0f) return 0;
        // O EPSILON: 1,5 / 0,12 e 12,5 na conta exata, mas 0,12 nao e exato em
        // float, e um 11,999 nao pode virar 11.
        return static_cast<int>(sobrecarga / entreTiros + 1e-4f);
    }

    float NotaDepois(float nota, const int acertosSofridos, const float porAcerto, const int tiros) {
        for (int i = 0; i < tiros; ++i) nota = Nota::Somar(nota, porAcerto, acertosSofridos);
        return nota;
    }
}
