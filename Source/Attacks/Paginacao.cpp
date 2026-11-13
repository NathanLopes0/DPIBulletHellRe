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

    int Relogio::MolduraDa(const int pagina) const {
        for (size_t m = 0; m < mMolduras.size(); ++m) {
            if (mMolduras[m].pagina == pagina) return static_cast<int>(m);
        }
        return -1;
    }

    std::vector<int> ConjuntoDeTrabalho(const int etapa, const int molduras, const int base) {
        // A mudanca e a outra metade do espaco de enderecos; o thrashing comeca entre as duas.
        static constexpr std::array<int, 3> kDeslocamento = {0, kPaginas / 2, kPaginas / 4};
        const int k = std::clamp(etapa, 0, 2);
        const int tamanho = k == 2 ? molduras + 2 : molduras - 1;
        std::vector<int> conjunto;
        for (int i = 0; i < std::min(tamanho, kPaginas); ++i) {
            conjunto.push_back(((base + kDeslocamento[static_cast<size_t>(k)] + i) % kPaginas + kPaginas) % kPaginas);
        }
        return conjunto;
    }

    int ProximaReferencia(Variacao::Sorteio& s, const std::vector<int>& conjunto, const float fora, int& cursor,
                          const bool emOrdem) {
        if (conjunto.empty() || s.Chance(fora)) return s.Inteiro(0, kPaginas - 1);
        if (!emOrdem) return s.Um(conjunto);
        cursor = (cursor + 1) % static_cast<int>(conjunto.size());
        return conjunto[static_cast<size_t>(cursor)];
    }

    float EntreReferencias(const int etapa, const Dificuldade::Nivel n) {
        static constexpr std::array<float, 3> kNormal = {0.55f, 0.5f, 0.42f};
        const float normal = kNormal[static_cast<size_t>(std::clamp(etapa, 0, 2))];
        return Dificuldade::Escolher(n, normal, normal * 0.85f);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.molduras = 4;
        d.fora = 0.14f;
        return d;
    }
}
