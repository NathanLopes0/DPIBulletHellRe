//
// A fase 3 do Michel: as redes neurais e as residuais.
//

#include "RedeNeural.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace RedeNeural {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    std::vector<int> CamadasDa(const int etapa) {
        if (etapa >= 2) return {4, 4, 4, 4, 3};
        return {4, 5, 3};
    }

    float Rede::Peso(const int k, const int i, const int j) const {
        return pesos[static_cast<size_t>(k)][static_cast<size_t>(i * camadas[static_cast<size_t>(k + 1)] + j)];
    }

    float& Rede::Peso(const int k, const int i, const int j) {
        return pesos[static_cast<size_t>(k)][static_cast<size_t>(i * camadas[static_cast<size_t>(k + 1)] + j)];
    }

    Rede Montar(const std::vector<int>& camadas, Variacao::Sorteio& s, const float menor, const float maior) {
        Rede r;
        r.camadas = camadas;
        for (size_t k = 0; k + 1 < camadas.size(); ++k) {
            std::vector<float> w(static_cast<size_t>(camadas[k] * camadas[k + 1]));
            for (float& p : w) p = s.Real(menor, maior);
            r.pesos.push_back(std::move(w));
        }
        return r;
    }

    std::vector<int> Ligacoes(const Rede& r, const int k, const int i, const float limiar) {
        std::vector<int> js;
        if (k < 0 || k + 1 >= static_cast<int>(r.camadas.size())) return js;
        if (i < 0 || i >= r.camadas[static_cast<size_t>(k)]) return js;
        for (int j = 0; j < r.camadas[static_cast<size_t>(k + 1)]; ++j) {
            if (r.Peso(k, i, j) >= limiar) js.push_back(j);
        }
        return js;
    }

    int Alinhado(const int i, const int n1, const int n2) {
        if (n2 <= 1) return 0;
        if (n1 <= 1) return (n2 - 1) / 2;
        const float u = static_cast<float>(i) / static_cast<float>(n1 - 1);
        return std::clamp(static_cast<int>(std::lround(u * static_cast<float>(n2 - 1))), 0, n2 - 1);
    }

    std::vector<Atalho> Atalhos(const std::vector<int>& camadas, const int pulo) {
        std::vector<Atalho> as;
        if (pulo < 2) return as;
        const int n = static_cast<int>(camadas.size());
        for (int k = 0; k + pulo < n; k += pulo) {
            for (int i = 0; i < camadas[static_cast<size_t>(k)]; ++i) {
                as.push_back(Atalho{k, i, Alinhado(i, camadas[static_cast<size_t>(k)], camadas[static_cast<size_t>(k + pulo)]),
                                    pulo});
            }
        }
        return as;
    }

    bool Somar(float& soma, const float entrada, const float limiar) {
        soma += entrada;
        if (soma < limiar) return false;
        soma -= limiar;
        return true;
    }

    void Retropropagar(Rede& r, const int alvo, const float taxa, const float menor, const float maior) {
        const int camadas = static_cast<int>(r.camadas.size());
        if (camadas < 2) return;
        // O erro da saida: +1 na que devia disparar, -1 nas outras.
        std::vector<float> delta(static_cast<size_t>(r.camadas.back()), -1.0f);
        if (alvo >= 0 && alvo < r.camadas.back()) delta[static_cast<size_t>(alvo)] = 1.0f;
        for (int k = camadas - 2; k >= 0; --k) {
            const int n1 = r.camadas[static_cast<size_t>(k)], n2 = r.camadas[static_cast<size_t>(k + 1)];
            std::vector<float> antes(static_cast<size_t>(n1), 0.0f);
            for (int i = 0; i < n1; ++i) {
                for (int j = 0; j < n2; ++j) antes[static_cast<size_t>(i)] += r.Peso(k, i, j) * delta[static_cast<size_t>(j)];
            }
            for (int i = 0; i < n1; ++i) {
                for (int j = 0; j < n2; ++j) {
                    float& w = r.Peso(k, i, j);
                    w = std::clamp(w + taxa * delta[static_cast<size_t>(j)], menor, maior);
                }
            }
            // O erro da camada de tras, no mesmo tamanho (-1 a 1), para o passo nao crescer.
            float maiorAbs = 0.0f;
            for (const float d : antes) maiorAbs = std::max(maiorAbs, std::fabs(d));
            if (maiorAbs > 1e-6f) {
                for (float& d : antes) d /= maiorAbs;
            }
            delta = std::move(antes);
        }
    }

    int SaidaDoAluno(const std::vector<float>& xDasSaidas, const float xDoAluno) {
        int melhor = -1;
        float menor = std::numeric_limits<float>::max();
        for (size_t k = 0; k < xDasSaidas.size(); ++k) {
            const float d = std::fabs(xDasSaidas[k] - xDoAluno);
            if (d < menor) {
                menor = d;
                melhor = static_cast<int>(k);
            }
        }
        return melhor;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreEntradas = 0.34f;
        d.velocidadeDoSinal = 170.0f;
        d.ligacaoForte = 0.5f;
        d.resistencia = 4;
        d.dropout = 2.6f;
        d.entreTreinos = 0.7f;
        d.taxa = 0.22f;
        d.tirosDaSaida = 4;
        d.velocidadeDaSaida = 220.0f;
        return d;
    }
}
