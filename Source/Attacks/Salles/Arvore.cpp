//
// A arvore binaria de busca da fase 3 do Salles.
//

#include "Arvore.h"

#include <algorithm>

namespace Arvore {

    int Profundidade(const int indice) {
        int d = 0;
        for (int i = indice; i > 1; i /= 2) ++d;
        return d;
    }

    bool Cabe(const int indice) {
        return indice >= 1 && Profundidade(indice) <= kProfundidadeMaxima;
    }

    float X(const int indice) {
        // NO NIVEL d HA 2^d LUGARES, cada um com a largura dividida por 2^d; o
        // de indice i e o (i - 2^d)-esimo da esquerda.
        const int d = Profundidade(indice);
        const int lugares = 1 << d;
        const int p = indice - lugares;
        const float passo = kLarguraDaArvore / static_cast<float>(lugares);
        return kXDaRaiz - kLarguraDaArvore / 2.0f + (static_cast<float>(p) + 0.5f) * passo;
    }

    float Y(const int indice) {
        return kYDaRaiz + static_cast<float>(Profundidade(indice)) * kEntreNiveis;
    }

    std::vector<int> Caminho(const Nos& nos, const int chave) {
        std::vector<int> caminho;
        int i = 1;
        while (true) {
            caminho.push_back(i);
            const auto it = nos.find(i);
            if (it == nos.end() || it->second == chave) break;   // vazio, ou achou
            i = chave < it->second ? 2 * i : 2 * i + 1;
            if (!Cabe(i)) { caminho.push_back(i); break; }      // passou do fundo
        }
        return caminho;
    }

    std::vector<int> Subarvore(const Nos& nos, const int indice) {
        std::vector<int> sub;
        std::vector<int> pilha = {indice};
        while (!pilha.empty()) {
            const int i = pilha.back();
            pilha.pop_back();
            if (!nos.count(i)) continue;
            sub.push_back(i);
            pilha.push_back(2 * i);
            pilha.push_back(2 * i + 1);
        }
        std::sort(sub.begin(), sub.end());
        return sub;
    }

    Operacao Proxima(const Nos& nos, const std::uint32_t sorteio) {
        return Proxima(nos, sorteio, Plano{});
    }

    Operacao Proxima(const Nos& nos, const std::uint32_t sorteio, const Plano& p) {
        Operacao op;
        const float tipo = static_cast<float>(sorteio & 1023u) / 1024.0f;
        const float achar = static_cast<float>((sorteio >> 10) & 1023u) / 1024.0f;
        const auto escolha = sorteio >> 20;
        op.tipo = (nos.empty() || tipo < p.chanceDeInserir) ? Tipo::Inserir : Tipo::Buscar;

        const auto esta = [&](const int k) {
            return std::any_of(nos.begin(), nos.end(), [k](const auto& n) { return n.second == k; });
        };
        if (op.tipo == Tipo::Buscar && achar < kChanceDeAchar) {
            auto it = nos.begin();
            std::advance(it, static_cast<long>(escolha % nos.size()));
            op.chave = it->second;
            return op;
        }
        int k = 1 + static_cast<int>(escolha % static_cast<std::uint32_t>(kMaiorChave));
        for (int i = 0; i < kMaiorChave && esta(k); ++i) k = 1 + (k % kMaiorChave);
        op.chave = k;
        return op;
    }

    std::string PlacarDaOperacao(const Operacao& op) {
        return (op.tipo == Tipo::Inserir ? "inserir(" : "busca(") + std::to_string(op.chave) + ")";
    }

    std::string PlacarDaComparacao(const int chave, const int doNo) {
        const bool menor = chave < doNo;
        return std::to_string(chave) + (menor ? " < " : " > ") + std::to_string(doNo)
             + (menor ? ": esquerda" : ": direita");
    }

    std::string PlacarDoFim(const Operacao& op, const bool achou, const bool cabe) {
        const std::string k = std::to_string(op.chave);
        if (op.tipo == Tipo::Inserir) return cabe ? k + " entra" : k + ": altura demais!";
        return achou ? k + ": achou" : k + ": NULL";
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        p.chanceDeInserir = s.Real(0.55f, 0.75f);
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.emCadaNo = Dificuldade::Escolher(n, kEmCadaNo, 0.16f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.25f);
        d.anelDoAchado = Dificuldade::Escolher(n, 10, 12);
        return d;
    }
}
