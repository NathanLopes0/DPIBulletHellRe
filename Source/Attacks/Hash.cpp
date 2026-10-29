//
// A tabela hash da fase 2 do Salles.
//

#include "Hash.h"

#include <algorithm>

namespace Hash {

    int Balde(const int chave, const int m) {
        if (m <= 0) return 0;
        return ((chave % m) + m) % m;
    }

    int ProximoTamanho(const int m) {
        for (size_t i = 0; i + 1 < kTamanhos.size(); ++i) {
            if (kTamanhos[i] == m) return kTamanhos[i + 1];
        }
        return m;
    }

    bool PrecisaDeRehash(const int chaves, const int m) {
        return chaves > m && ProximoTamanho(m) != m;
    }

    float CentroDoBalde(const int i, const int m, const float largura) {
        if (m <= 0) return largura / 2.0f;
        return (static_cast<float>(i) + 0.5f) * largura / static_cast<float>(m);
    }

    float AlturaNaLista(const int p) {
        return kYDosBaldes + kPrimeiraChave + static_cast<float>(p < 0 ? 0 : p) * kEntreChaves;
    }

    Operacao Proxima(const std::vector<int>& naTabela, const std::uint32_t sorteio, const Plano& p) {
        Operacao op;
        const float tipo = static_cast<float>(sorteio & 1023u) / 1024.0f;
        const float achar = static_cast<float>((sorteio >> 10) & 1023u) / 1024.0f;
        const auto escolha = sorteio >> 20;
        op.tipo = (naTabela.empty() || tipo < p.chanceDeInserir) ? Tipo::Inserir : Tipo::Buscar;

        const auto esta = [&](const int k) { return std::find(naTabela.begin(), naTabela.end(), k) != naTabela.end(); };
        if (op.tipo == Tipo::Buscar && achar < kChanceDeAchar) {
            op.chave = naTabela[escolha % naTabela.size()];
            return op;
        }
        // UMA CHAVE QUE NAO ESTA: a sorteada, ou a primeira livre depois dela.
        int k = static_cast<int>(escolha % static_cast<std::uint32_t>(kMaiorChave + 1));
        for (int i = 0; i <= kMaiorChave && esta(k); ++i) k = (k + 1) % (kMaiorChave + 1);
        op.chave = k;
        return op;
    }

    std::string PlacarDaInsercao(const int chave, const int m) {
        return "h(" + std::to_string(chave) + ") = " + std::to_string(chave) + " mod " + std::to_string(m)
             + " = " + std::to_string(Balde(chave, m));
    }

    std::string PlacarDaBusca(const int chave, const bool achou) {
        return "busca(" + std::to_string(chave) + "): " + (achou ? "achou" : "NULL");
    }

    std::string PlacarDoRehash(const int de, const int para) {
        return "rehash: m = " + std::to_string(de) + " \xE2\x86\x92 " + std::to_string(para);
    }

    std::string CaraDaChave(const int chave) {
        return "K" + std::to_string(std::clamp(chave, 0, kMaiorChave));
    }

    std::string CaraDoBalde(const int balde) {
        return "B" + std::to_string(balde < 0 ? 0 : balde);
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        p.tamanhoInicial = kTamanhos[static_cast<size_t>(s.Inteiro(0, 1))];
        p.chanceDeInserir = s.Real(0.45f, 0.65f);
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.passo = Dificuldade::Escolher(n, kPasso, 0.55f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.25f);
        return d;
    }
}
