//
// A fase 1 da Sabrina: a dupla helice - o modelo 3D que gira.
//

#include "Helice.h"

#include <algorithm>
#include <cmath>

namespace Helice {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    Ponto3 GirarEmY(const Ponto3 p, const float angulo) {
        const float c = Math::Cos(angulo), s = Math::Sin(angulo);
        return Ponto3{p.x * c - p.z * s, p.y, p.x * s + p.z * c};
    }

    Ponto3 GirarEmX(const Ponto3 p, const float angulo) {
        const float c = Math::Cos(angulo), s = Math::Sin(angulo);
        return Ponto3{p.x, p.y * c - p.z * s, p.y * s + p.z * c};
    }

    NaTela Projetar(const Ponto3 p, const Vector2 centro, const float distancia) {
        // A camera fica em z = distancia, olhando para a tela: o que tem z maior esta mais perto.
        const float ate = std::max(distancia - p.z, 0.25f * distancia);
        const float e = distancia / ate;
        return NaTela{centro + Vector2(p.x * e, p.y * e), e};
    }

    bool NaFrente(const float z, const float raio, const float limiar) {
        return z > limiar * raio;
    }

    float Angulo(const int fita, const float s, const float giro, const float torcao, const float sulco) {
        return giro + s * torcao + (fita == 1 ? sulco : 0.0f);
    }

    Ponto3 DePe(const float s, const float raio, const float passo, const float angulo) {
        return Ponto3{raio * Math::Cos(angulo), s * passo, raio * Math::Sin(angulo)};
    }

    Ponto3 Deitada(const float s, const float raio, const float passo, const float angulo) {
        return Ponto3{s * passo, raio * Math::Cos(angulo), raio * Math::Sin(angulo)};
    }

    int Borda(const float antes, const float depois) {
        const int a = static_cast<int>(std::floor(antes / Math::Pi));
        const int d = static_cast<int>(std::floor(depois / Math::Pi));
        if (a == d) return 0;
        // O ultimo multiplo de pi cruzado: subindo, o de 'depois'; descendo, o de 'antes'.
        const int m = depois > antes ? d : a;
        return ((m % 2) + 2) % 2 == 0 ? 1 : -1;
    }

    float AndarDaRosca(const float giroPorSegundo, const float torcaoPorPx) {
        if (std::fabs(torcaoPorPx) < 1e-6f) return 0.0f;
        return giroPorSegundo / torcaoPorPx;
    }

    std::vector<Pareamento::Base> Sortear(Variacao::Sorteio& s, const int n) {
        using Pareamento::Base;
        std::vector<Base> v;
        v.reserve(static_cast<size_t>(std::max(0, n)));
        bool rico = s.Chance(0.5f);   // rico em G-C?
        while (static_cast<int>(v.size()) < n) {
            const int bloco = s.Inteiro(2, 4);
            for (int k = 0; k < bloco && static_cast<int>(v.size()) < n; ++k) {
                const bool primeira = s.Chance(0.5f);
                v.push_back(rico ? (primeira ? Base::G : Base::C) : (primeira ? Base::A : Base::T));
            }
            rico = !rico;
        }
        return v;
    }

    float Fusao(const std::vector<Pareamento::Base>& fita, const int k) {
        if (k < 0 || k >= static_cast<int>(fita.size())) return 0.0f;
        const float proprias = static_cast<float>(Pareamento::Pontes(fita[static_cast<size_t>(k)]));
        float vizinhas = 0.0f;
        int quantas = 0;
        for (const int j : {k - 1, k + 1}) {
            if (j < 0 || j >= static_cast<int>(fita.size())) continue;
            vizinhas += static_cast<float>(Pareamento::Pontes(fita[static_cast<size_t>(j)]));
            ++quantas;
        }
        const float media = quantas > 0 ? vizinhas / static_cast<float>(quantas) : proprias;
        return 50.0f + 8.0f * proprias + 6.0f * media;
    }

    float Temperatura(const float t, const float duracao, const float inicial, const float final) {
        const float u = duracao > 0.0f ? std::clamp(t / duracao, 0.0f, 1.0f) : 1.0f;
        return inicial + (final - inicial) * u;
    }

    float RaioDoPar(const float desde, const float raio, const float inchaco, const float tempo) {
        if (desde <= 0.0f) return raio;
        const float u = tempo > 0.0f ? std::clamp(desde / tempo, 0.0f, 1.0f) : 1.0f;
        return raio * (1.0f + inchaco * u * u * (3.0f - 2.0f * u));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.giro = 2.0f;
        d.limiar = 0.18f;
        d.varredura = 0.34f;
        d.periodo = 6.0f;
        d.velocidadeDaBase = 205.0f;
        d.duasFitasAtiram = true;
        d.torcaoInicial = 0.44f;
        d.torcaoFinal = 0.66f;
        d.giroDeitada = 2.7f;
        d.periodoDaDescida = 5.0f;
        d.entreLeques = 0.95f;
        d.tirosDoLeque = 4;
        d.velocidadeDoLeque = 220.0f;
        d.temperaturaInicial = 72.0f;
        d.temperaturaFinal = 104.0f;
        d.giroQuente = 3.3f;
        d.tirosDaFusao = 1;
        d.velocidadeDaFusao = 215.0f;
        return d;
    }
}
