//
// A fase 2 do Goulart: a memoria paginada.
//

#include "Paginacao.h"

#include <algorithm>

namespace Paginacao {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    Relogio::Relogio(const int molduras) : mMolduras(static_cast<size_t>(std::max(1, molduras))) {}

    Referencia Relogio::Referenciar(const int pagina) {
        Referencia r;
        const int n = static_cast<int>(mMolduras.size());
        if (const int m = MolduraDa(pagina); m >= 0) {
            mMolduras[static_cast<size_t>(m)].referenciada = true;
            r.acerto = true;
            r.moldura = m;
            return r;
        }
        // Falta. Uma moldura livre, se houver: a memoria ainda nao encheu.
        for (int m = 0; m < n; ++m) {
            if (mMolduras[static_cast<size_t>(m)].pagina >= 0) continue;
            mMolduras[static_cast<size_t>(m)] = Moldura{pagina, true};
            r.moldura = m;
            return r;
        }
        // Cheia: o ponteiro poupa quem tem R ligado (apagando o R) e despeja o primeiro sem.
        while (mMolduras[static_cast<size_t>(mPonteiro)].referenciada) {
            mMolduras[static_cast<size_t>(mPonteiro)].referenciada = false;
            r.segundaChance.push_back(mPonteiro);
            mPonteiro = (mPonteiro + 1) % n;
        }
        r.moldura = mPonteiro;
        r.vitima = mMolduras[static_cast<size_t>(mPonteiro)].pagina;
        mMolduras[static_cast<size_t>(mPonteiro)] = Moldura{pagina, true};
        mPonteiro = (mPonteiro + 1) % n;
        return r;
    }

    std::vector<int> Relogio::Encolher(const int molduras) {
        std::vector<int> despejadas;
        const auto n = static_cast<size_t>(std::max(1, molduras));
        while (mMolduras.size() > n) {
            if (mMolduras.back().pagina >= 0) despejadas.push_back(mMolduras.back().pagina);
            mMolduras.pop_back();
        }
        mPonteiro %= static_cast<int>(mMolduras.size());
        return despejadas;
    }

    int Relogio::MolduraDa(const int pagina) const {
        for (size_t m = 0; m < mMolduras.size(); ++m) {
            if (mMolduras[m].pagina == pagina) return static_cast<int>(m);
        }
        return -1;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n == Dificuldade::Nivel::Dificil) {
            d.entreAcessos = 0.85f;
            d.leituraDoProfessor = 1.5f;
        }
        return d;
    }

    float EntreLeiturasDoProfessor(const Degrau& d, const int etapa) {
        static constexpr std::array<float, 3> kRitmo = {1.0f, 0.85f, 0.7f};
        return d.leituraDoProfessor * kRitmo[static_cast<size_t>(std::clamp(etapa, 0, 2))];
    }

    int PaginaDoProfessor(const int sorteio, const int doAluno) {
        const int p = std::clamp(sorteio, 0, kPaginas - 2);
        return p >= doAluno ? p + 1 : p;
    }

    int PaginaDoX(const float px, const float x, const float largura) {
        if (largura <= 0.0f) return 0;
        return std::clamp(static_cast<int>((px - x) / LarguraDaPagina(largura)), 0, kPaginas - 1);
    }

    float InicioDaPagina(const int p, const float x, const float largura) {
        return x + LarguraDaPagina(largura) * static_cast<float>(std::clamp(p, 0, kPaginas - 1));
    }

    float LarguraDaPagina(const float largura) {
        return largura / static_cast<float>(kPaginas);
    }

    int MoldurasNaEtapa(const int etapa, const Dificuldade::Nivel n) {
        static constexpr std::array<int, 3> kNormal = {5, 4, 3};
        static constexpr std::array<int, 3> kDificil = {4, 3, 3};
        const auto k = static_cast<size_t>(std::clamp(etapa, 0, 2));
        return Dificuldade::Escolher(n, kNormal[k], kDificil[k]);
    }
}
