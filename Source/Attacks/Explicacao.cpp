//
// A repescagem do Daniel: a explicabilidade dos modelos.
//

#include "Explicacao.h"

#include <algorithm>
#include <cmath>

namespace Explicacao {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    float Antecedencia(const int perguntas, const Dificuldade::Nivel n) {
        const float base = Dificuldade::Escolher(n, 0.15f, 0.08f);
        const float porPergunta = Dificuldade::Escolher(n, 0.09f, 0.07f);
        return std::min(kAntecedenciaMaxima, base + porPergunta * static_cast<float>(std::max(0, perguntas)));
    }

    std::vector<float> Rumos(const Palpite p, const float mira, const float giro, const int tiros) {
        std::vector<float> r;
        const int n = std::max(1, tiros);
        switch (p) {
            case Palpite::Anel:
                for (int k = 0; k < n; ++k) r.push_back(giro + 360.0f * static_cast<float>(k) / static_cast<float>(n));
                break;
            case Palpite::Leque: {
                const float abertura = 7.0f;
                for (int k = 0; k < n; ++k) r.push_back(mira + abertura * (static_cast<float>(k) - 0.5f * static_cast<float>(n - 1)));
                break;
            }
            case Palpite::Espiral: {
                // Dois bracos opostos, cada tiro um pouco mais girado: uma espiral quando sai.
                const int porBraco = std::max(1, n / 2);
                for (int braco = 0; braco < 2; ++braco) {
                    for (int k = 0; k < porBraco; ++k) {
                        r.push_back(giro + 180.0f * static_cast<float>(braco) + 11.0f * static_cast<float>(k));
                    }
                }
                break;
            }
        }
        return r;
    }

    std::vector<Barra> Cascata(const float base, const std::vector<float>& contribuicoes) {
        std::vector<Barra> barras;
        float x = base;
        for (size_t k = 0; k < contribuicoes.size(); ++k) {
            barras.push_back(Barra{x, x + contribuicoes[k], static_cast<int>(k % kAtributos.size())});
            x += contribuicoes[k];
        }
        return barras;
    }

    std::vector<float> Contribuicoes(Variacao::Sorteio& s, const int quantas, const float minimo, const float maximo,
                                     const float limite) {
        std::vector<float> c;
        float soma = 0.0f;
        for (int k = 0; k < quantas; ++k) {
            const float modulo = s.Real(minimo, maximo);
            float sinal = s.Inteiro(0, 1) == 0 ? -1.0f : 1.0f;
            // Se o sinal sorteado leva a soma para fora, vai o outro.
            if (std::fabs(soma + sinal * modulo) > limite) sinal = -sinal;
            float v = sinal * modulo;
            if (std::fabs(soma + v) > limite) v = (soma + v > 0.0f ? limite : -limite) - soma;
            c.push_back(v);
            soma += v;
        }
        return c;
    }

    std::string Legenda(const std::vector<float>& contribuicoes, const float escala) {
        std::string s;
        for (size_t k = 0; k < contribuicoes.size(); ++k) {
            const int pontos = static_cast<int>(std::lround(contribuicoes[k] / std::max(escala, 1e-3f)));
            if (!s.empty()) s += "  \xc2\xb7  ";   // o ponto do meio
            s += kAtributos[k % kAtributos.size()];
            s += pontos >= 0 ? " +" : " ";
            s += std::to_string(pontos);
        }
        return s;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entrePalpites = 0.6f;
        d.tirosDoAnel = 30;
        d.tirosDoLeque = 11;
        d.velocidade = 240.0f;
        d.entreBarras = 0.3f;
        d.avisoDaBarra = 0.4f;
        d.tirosDaPrevisao = 16;
        return d;
    }
}
