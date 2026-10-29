//
// A fase 1 do Hugo: a proposta, e o cronograma dela.
//

#include "Cronograma.h"

#include <algorithm>
#include <cmath>

namespace Cronograma {

    namespace {
        const std::vector<std::string> kNomes = {
            "Revisão bibliográfica", "Proposta", "Metodologia", "Implementação",
            "Experimentos", "Análise dos resultados", "Escrita", "Revisão do orientador"};

        bool Cabe(const std::vector<Tarefa>& feitas, const Tarefa& nova) {
            for (const Tarefa& t : feitas) {
                if (t.linha == nova.linha && nova.de < t.ate + kFolgaNaLinha && t.de < nova.ate + kFolgaNaLinha) {
                    return false;
                }
            }
            return true;
        }
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.tarefas = 5;
        d.varredura = Dificuldade::Escolher(n, 4.2f, 3.9f);
        d.entrePaginas = Dificuldade::Escolher(n, 40.0f, 38.0f);
        d.queda = Dificuldade::Escolher(n, 250.0f, 270.0f);
        return d;
    }

    std::vector<Tarefa> Gerar(Variacao::Sorteio& s, const float largura, const Degrau& d) {
        const float inicio = kMargem, fim = largura - kMargem;
        std::vector<std::string> nomes = kNomes;
        s.Embaralhar(nomes);
        const int n = d.tarefas;
        for (int tentativa = 0; tentativa < 64; ++tentativa) {
            // OS PRAZOS PRIMEIRO, com kSeparacao entre eles e a folga sorteada.
            const float folga = (fim - (inicio + kMenor)) - static_cast<float>(n - 1) * kSeparacao;
            std::vector<float> sobras(static_cast<size_t>(n));
            for (float& f : sobras) f = s.Real(0.0f, std::max(0.0f, folga));
            std::sort(sobras.begin(), sobras.end());
            std::vector<Tarefa> feitas;
            bool deuCerto = true;
            for (int i = 0; i < n && deuCerto; ++i) {
                Tarefa t;
                t.ate = inicio + kMenor + sobras[static_cast<size_t>(i)] + static_cast<float>(i) * kSeparacao;
                t.nome = nomes[static_cast<size_t>(i) % nomes.size()];
                // DEPOIS A BARRA: o comprimento sorteado, numa linha livre.
                std::vector<int> linhas;
                for (int l = 0; l < kLinhas; ++l) linhas.push_back(l);
                s.Embaralhar(linhas);
                const float comprimento = s.Real(kMenor, std::min(kMaior, t.ate - inicio));
                deuCerto = false;
                for (const int l : linhas) {
                    t.linha = l;
                    t.de = t.ate - comprimento;
                    if (Cabe(feitas, t)) { deuCerto = true; break; }
                }
                if (deuCerto) feitas.push_back(t);
            }
            if (deuCerto) return feitas;
        }
        // NAO COUBE (nao acontece com os numeros de hoje): as barras mais
        // curtas, uma por linha, em escada.
        std::vector<Tarefa> escada;
        for (int i = 0; i < n; ++i) {
            Tarefa t;
            t.linha = i % kLinhas;
            t.ate = inicio + kMenor + static_cast<float>(i) * kSeparacao;
            t.de = t.ate - kMenor;
            t.nome = nomes[static_cast<size_t>(i) % nomes.size()];
            escada.push_back(t);
        }
        return escada;
    }

    float YDaLinha(const int linha) {
        return kYDaPrimeiraLinha + kEntreLinhas * static_cast<float>(linha);
    }

    float XDoHoje(const float t, const float largura, const Degrau& d) {
        const float inicio = kMargem, fim = largura - kMargem;
        return inicio + (fim - inicio) * std::clamp(t / d.varredura, 0.0f, 1.0f);
    }

    std::vector<float> Paginas(const Tarefa& t, const Degrau& d) {
        std::vector<float> xs;
        const int n = std::max(2, static_cast<int>(std::ceil((t.ate - t.de) / d.entrePaginas)) + 1);
        for (int i = 0; i < n; ++i) {
            xs.push_back(t.de + (t.ate - t.de) * static_cast<float>(i) / static_cast<float>(n - 1));
        }
        return xs;
    }

    int Semana(const float x, const float largura) {
        const float inicio = kMargem, fim = largura - kMargem;
        const float u = std::clamp((x - inicio) / (fim - inicio), 0.0f, 1.0f);
        return std::clamp(static_cast<int>(std::ceil(u * kSemanas)), 1, kSemanas);
    }

    std::string Placar(const Tarefa& t, const float largura) {
        return t.nome + "\nvenceu na semana " + std::to_string(Semana(t.ate, largura));
    }
}
