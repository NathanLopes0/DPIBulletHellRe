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

    Endereco Traduzir(const std::uint32_t virtual_) {
        Endereco e;
        e.pagina = static_cast<int>(virtual_ >> kBitsDoDeslocamento);
        e.deslocamento = static_cast<int>(virtual_ & ((1u << kBitsDoDeslocamento) - 1u));
        return e;
    }

    std::uint32_t Montar(const int pagina, const int deslocamento) {
        const auto mascara = (1u << kBitsDoDeslocamento) - 1u;
        return (static_cast<std::uint32_t>(std::max(pagina, 0)) << kBitsDoDeslocamento) |
               (static_cast<std::uint32_t>(std::max(deslocamento, 0)) & mascara);
    }

    std::string Hexa(const std::uint32_t valor, const int digitos) {
        static constexpr char kAlgarismos[] = "0123456789ABCDEF";
        std::string s;
        for (int i = digitos - 1; i >= 0; --i) s += kAlgarismos[(valor >> (4 * i)) & 0xFu];
        return "0x" + s;
    }

    namespace {
        /// "0x3A7F = página 3 + 0xA7F". \xc3\xa1 e o a com acento.
        std::string Conta(const std::uint32_t endereco) {
            const Endereco e = Traduzir(endereco);
            return Hexa(endereco, 4) + " = p\xc3\xa1gina " + std::to_string(e.pagina) + " + " +
                   Hexa(static_cast<std::uint32_t>(e.deslocamento), 3);
        }
    }

    std::string PlacarDoAcerto(const std::uint32_t endereco, const int moldura) {
        // \xc2\xb7 e o ponto do meio.
        return Conta(endereco) + "  \xc2\xb7  acerto na moldura " + std::to_string(moldura);
    }

    std::string PlacarDaFalta(const std::uint32_t endereco, const int vitima, const int moldura) {
        if (vitima < 0) return Conta(endereco) + "  \xc2\xb7  falta! vai para a moldura " + std::to_string(moldura);
        return Conta(endereco) + "  \xc2\xb7  falta! sai a p\xc3\xa1gina " + std::to_string(vitima);
    }

    std::string PlacarDoThrashing(const int faltas, const int referencias) {
        // \xc3\xba e o u com acento; \xc3\xaa o e com circunflexo.
        return "Thrashing: " + std::to_string(faltas) + (faltas == 1 ? " falta" : " faltas") + " nas \xc3\xbaltimas " +
               std::to_string(referencias) + " refer\xc3\xaancias";
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
