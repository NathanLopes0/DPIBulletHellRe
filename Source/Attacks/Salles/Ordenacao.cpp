//
// O bubble sort da repescagem do Salles.
//

#include "Ordenacao.h"

#include <algorithm>
#include <utility>

namespace Ordenacao {

    float X(const float k, const int n, const float largura) {
        const float meio = static_cast<float>(n > 0 ? n - 1 : 0) / 2.0f;
        return largura / 2.0f + (k - meio) * kEntre;
    }

    float VelocidadeDaQueda(const int valor) {
        return kQuedaMinima + kQuedaPorValor * static_cast<float>(std::clamp(valor, kMenorValor, kMaiorValor));
    }

    std::vector<int> Sortear(std::uint32_t sorteio, const int n) {
        // UM GERADOR PROPRIO, a partir do sorteio: os valores tem de ser
        // diferentes, e a escolha de cada um depende das anteriores. Os bits
        // altos de um congruencial linear sao os bons.
        std::vector<int> v;
        const int quantos = std::clamp(n, 0, kMaiorValor - kMenorValor + 1);
        auto proximo = [&sorteio]() {
            sorteio = sorteio * 1664525u + 1013904223u;
            return sorteio >> 8;
        };
        while (static_cast<int>(v.size()) < quantos) {
            const int k = kMenorValor + static_cast<int>(proximo() % static_cast<std::uint32_t>(kMaiorValor - kMenorValor + 1));
            if (std::find(v.begin(), v.end(), k) == v.end()) v.push_back(k);
        }
        // FORA DE ORDEM: um vetor que nasceu ordenado troca os dois primeiros.
        if (v.size() >= 2 && std::is_sorted(v.begin(), v.end())) std::swap(v[0], v[1]);
        return v;
    }

    Bolha Comecar(std::vector<int> v) {
        Bolha b;
        b.v = std::move(v);
        b.fim = static_cast<int>(b.v.size()) - 1;
        if (b.fim < 0) b.fim = 0;
        return b;
    }

    Passo Avancar(Bolha& b) {
        Passo p;
        const int n = static_cast<int>(b.v.size());
        if (b.ordenado || n <= 1) {
            b.ordenado = true;
            p.tipo = Tipo::Ordenado;
            return p;
        }

        // UMA COMPARACAO: o par v[i], v[i + 1].
        if (b.i < b.fim) {
            p.i = b.i;
            p.a = b.v[static_cast<size_t>(b.i)];
            p.b = b.v[static_cast<size_t>(b.i + 1)];
            if (p.a > p.b) {
                std::swap(b.v[static_cast<size_t>(b.i)], b.v[static_cast<size_t>(b.i + 1)]);
                b.trocou = true;
                p.tipo = Tipo::Troca;
            } else {
                p.tipo = Tipo::Fica;
            }
            ++b.i;
            return p;
        }

        // O FIM DA PASSADA. Sem troca nenhuma, o vetor ja estava em ordem.
        if (!b.trocou) {
            b.ordenado = true;
            p.tipo = Tipo::Ordenado;
            p.semTroca = true;
            return p;
        }

        // COM TROCA, A BOLHA CHEGOU: v[fim] e o maior do que faltava, e assenta.
        p.tipo = Tipo::NoLugar;
        p.i = b.fim;
        p.a = b.v[static_cast<size_t>(b.fim)];
        --b.fim;
        b.i = 0;
        b.trocou = false;
        if (b.fim <= 0) b.ordenado = true;   // sobrou um so, e ele ja esta no lugar
        return p;
    }

    void Remover(Bolha& b, const int k) {
        const int n = static_cast<int>(b.v.size());
        if (k < 0 || k >= n) return;
        b.v.erase(b.v.begin() + k);
        if (b.ordenado) return;

        if (k <= b.fim) {
            // NA PARTE QUE FALTA ORDENAR, a passada recomeca: o maior que a
            // bolha vinha carregando pode ter sido justamente o tirado.
            --b.fim;
            b.i = 0;
            b.trocou = false;
        }
        // NA PARTE JA ORDENADA, quem sobrou continua em ordem e maior que o resto.
        if (b.fim < 0) b.fim = 0;
        if (static_cast<int>(b.v.size()) <= 1) b.ordenado = true;
    }

    std::string PlacarDoComeco(const int n) {
        return "bubble sort, n = " + std::to_string(n);
    }

    std::string Placar(const Passo& p) {
        switch (p.tipo) {
            case Tipo::Troca:   return std::to_string(p.a) + " > " + std::to_string(p.b) + ": troca";
            case Tipo::Fica:    return std::to_string(p.a) + " < " + std::to_string(p.b) + ": fica";
            case Tipo::NoLugar: return std::to_string(p.a) + " no lugar";
            case Tipo::Ordenado: return p.semTroca ? "nenhuma troca: ordenado!" : "ordenado!";
        }
        return "";
    }

    std::string CaraDoElemento(const int valor) {
        return "K" + std::to_string(std::clamp(valor, 0, kMaiorValor));
    }

    std::vector<int> Sortear(const std::uint32_t sorteio, const int n, const Entrada e) {
        std::vector<int> v = Sortear(sorteio, n);
        if (e == Entrada::Aleatoria || v.size() < 2) return v;
        std::sort(v.begin(), v.end());
        if (e == Entrada::Invertida) {
            std::reverse(v.begin(), v.end());
            return v;
        }
        // QUASE ORDENADO: dois pares vizinhos trocados, longe um do outro.
        const std::size_t m = v.size();
        const std::size_t a = static_cast<std::size_t>((sorteio >> 3) % static_cast<std::uint32_t>(m / 2));
        std::swap(v[a], v[a + 1]);
        const std::size_t b = std::min(m - 2, a + 2 + static_cast<std::size_t>((sorteio >> 9) % 3u));
        if (b > a + 1) std::swap(v[b], v[b + 1]);
        return v;
    }

    std::string PlacarDoComeco(const int n, const Entrada e) {
        switch (e) {
            case Entrada::QuaseOrdenada: return PlacarDoComeco(n) + " \xc2\xb7 quase ordenado";
            case Entrada::Invertida:     return PlacarDoComeco(n) + " \xc2\xb7 invertido: o pior caso";
            default:                     return PlacarDoComeco(n);
        }
    }

    Plano PlanoDeSempre() {
        Plano p;
        p.entradas = {Entrada::Aleatoria, Entrada::Aleatoria, Entrada::Aleatoria};
        return p;
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        std::vector<Entrada> e = {Entrada::Aleatoria, Entrada::QuaseOrdenada, Entrada::Invertida};
        s.Embaralhar(e);
        for (std::size_t k = 0; k < e.size(); ++k) p.entradas[k] = e[k];
        p.tamanho = s.Inteiro(7, 8);
        return p;
    }

    Entrada EntradaDaVez(const Plano& p, const int k) {
        const int n = static_cast<int>(p.entradas.size());
        return p.entradas[static_cast<std::size_t>(((k % n) + n) % n)];
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.passo = Dificuldade::Escolher(n, kPasso, 0.27f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.25f);
        d.anelDoLugar = Dificuldade::Escolher(n, 8, 10);
        return d;
    }
}
