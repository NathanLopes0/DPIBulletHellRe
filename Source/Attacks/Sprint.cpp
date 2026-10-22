//
// A sprint dos metodos ageis: o cursor arrasta as historias para o quadro.
//

#include "Sprint.h"

#include <algorithm>
#include <cmath>

namespace Sprint {

    float TempoDeFitts(const float distancia, const float largura, const Fitts& f) {
        const float l = std::max(largura, 1.0f);
        const float d = std::max(distancia, 0.0f);
        return f.a + f.b * std::log2(d / l + 1.0f);
    }

    float SemSolavanco(const float u) {
        const float x = std::clamp(u, 0.0f, 1.0f);
        return x * x * x * (10.0f + x * (-15.0f + 6.0f * x));
    }

    Vector2 Vaga(const int i, const int n, const Quadro& q) {
        const float meio = 0.5f * static_cast<float>(n - 1);
        return Vector2(q.centroX + (static_cast<float>(i) - meio) * q.espaco, q.altura);
    }

    Vector2 NaPilha(const int i) {
        const float k = static_cast<float>(i) * 3.0f;
        return Vector2(-13.0f - k, -12.0f - k);
    }

    Vector2 CentroDoCursor(const Vector2 ponta, const float escala) {
        return ponta + Vector2(10.0f, 12.0f) * escala;
    }

    Plano Planejar(const Vector2 origem, const int n, const float revisao, const Quadro& q, const Fitts& f) {
        Plano p;
        p.origem = origem;
        p.soltura = Vector2(q.centroX, q.altura);
        for (int i = 0; i < n; ++i) p.vagas.push_back(Vaga(i, n, q));

        // O ALVO DO ARRASTE E O QUADRO INTEIRO, da primeira vaga a ultima: a
        // sprint cheia tem alvo largo, e Fitts diz que alvo largo se acerta
        // mais depressa.
        const float largura = static_cast<float>(std::max(n - 1, 0)) * q.espaco + q.vaga;
        p.arraste = TempoDeFitts((p.soltura - origem).Length(), largura, f);
        p.espalha = kEspalha;
        p.revisao = std::max(revisao, p.arraste + p.espalha);
        return p;
    }

    Vector2 PontaDoCursor(const Plano& p, const float t) {
        if (t >= p.arraste) return p.soltura;
        const float u = (p.arraste > 0.0f) ? t / p.arraste : 1.0f;
        return Vector2::Lerp(p.origem, p.soltura, SemSolavanco(u));
    }

    Vector2 PosicaoDaHistoria(const Plano& p, const int i, const float t) {
        if (i < 0 || i >= static_cast<int>(p.vagas.size())) return p.soltura;
        if (t < p.arraste) return PontaDoCursor(p, t) + NaPilha(i);

        const float s = t - p.arraste;
        if (s < p.espalha) {
            return Vector2::Lerp(p.soltura + NaPilha(i), p.vagas[static_cast<size_t>(i)],
                                 SemSolavanco(s / p.espalha));
        }
        return p.vagas[static_cast<size_t>(i)];
    }

    Gesto GestoDoCursor(const Plano& p, const float t) {
        if (t >= p.revisao) return Gesto::Acabou;
        if (t < p.arraste) return Gesto::Arrastando;
        if (t < p.arraste + p.espalha) return Gesto::Soltando;
        if (t >= p.revisao - kAvisoDaRevisao) return Gesto::Clicando;
        return Gesto::Esperando;
    }
}
