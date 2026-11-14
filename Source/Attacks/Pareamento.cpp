//
// A fase 2 da Sabrina: o codigo - as bases, o pareamento, a transcricao e a traducao.
//

#include "Pareamento.h"

#include <algorithm>
#include <cmath>

namespace Pareamento {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    char Letra(const Base b) {
        switch (b) {
            case Base::A: return 'A';
            case Base::C: return 'C';
            case Base::G: return 'G';
            case Base::T: return 'T';
            case Base::U: return 'U';
        }
        return '?';
    }

    const char* Cara(const Base b) {
        switch (b) {
            case Base::A: return "A";
            case Base::C: return "C";
            case Base::G: return "G";
            case Base::T: return "T";
            case Base::U: return "U";
        }
        return "A";
    }

    int Pontes(const Base b) {
        return (b == Base::C || b == Base::G) ? 3 : 2;
    }

    Base Complementar(const Base b, const bool rna) {
        switch (b) {
            case Base::A: return rna ? Base::U : Base::T;
            case Base::C: return Base::G;
            case Base::G: return Base::C;
            case Base::T:
            case Base::U: return Base::A;
        }
        return Base::A;
    }

    bool Pareiam(const Base a, const Base b) {
        return Classe(Complementar(a, false)) == Classe(b);
    }

    Base NoAlfabeto(const Base b, const bool rna) {
        if (b == Base::T || b == Base::U) return rna ? Base::U : Base::T;
        return b;
    }

    int Classe(const Base b) {
        switch (b) {
            case Base::A: return 0;
            case Base::C: return 1;
            case Base::G: return 2;
            case Base::T:
            case Base::U: return 3;
        }
        return 0;
    }

    int ClasseDoTiro(const Base b) {
        return Classe(Complementar(b, false));
    }

    Base DaClasse(const int c, const bool rna) {
        switch (((c % 4) + 4) % 4) {
            case 0: return Base::A;
            case 1: return Base::C;
            case 2: return Base::G;
            default: return rna ? Base::U : Base::T;
        }
    }

    Base Outra(Variacao::Sorteio& s, const Base menos, const bool rna) {
        const auto& alfabeto = rna ? kRna : kDna;
        const int pular = Classe(menos);
        // Tres escolhas: as do alfabeto que nao sao 'menos'.
        int k = s.Inteiro(0, 2);
        if (k >= pular) ++k;
        return alfabeto[static_cast<size_t>(k)];
    }

    std::vector<Base> Sequencia(Variacao::Sorteio& s, const int n, const float repetir, const bool rna) {
        std::vector<Base> v;
        if (n <= 0) return v;
        const auto& alfabeto = rna ? kRna : kDna;
        v.reserve(static_cast<size_t>(n));
        v.push_back(alfabeto[static_cast<size_t>(s.Inteiro(0, 3))]);
        for (int k = 1; k < n; ++k) v.push_back(s.Chance(repetir) ? v.back() : Outra(s, v.back(), rna));
        return v;
    }

    std::vector<Base> Onda(Variacao::Sorteio& s, const Base dominante, const int n, const float fracao, const bool rna) {
        std::vector<Base> v;
        if (n <= 0) return v;
        const int quantas = std::clamp(static_cast<int>(std::lround(static_cast<float>(n) * fracao)), 0, n);
        const Base d = NoAlfabeto(dominante, rna);
        v.assign(static_cast<size_t>(quantas), d);
        while (static_cast<int>(v.size()) < n) v.push_back(Outra(s, d, rna));
        s.Embaralhar(v);
        return v;
    }

    Base Transcrever(const Base molde) {
        return Complementar(molde, true);
    }

    std::vector<Base> Molde(const std::vector<Base>& rna) {
        std::vector<Base> v;
        v.reserve(rna.size());
        for (const Base b : rna) v.push_back(Complementar(b, false));
        return v;
    }

    namespace {
        /// O codigo genetico padrao: a primeira base anda devagar, a terceira depressa, na ordem U C A G.
        constexpr const char* kCodigo = "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG";

        int NaTabela(const Base b) {
            switch (b) {
                case Base::T:
                case Base::U: return 0;
                case Base::C: return 1;
                case Base::A: return 2;
                case Base::G: return 3;
            }
            return 0;
        }
    }

    char Traduzir(const Base b1, const Base b2, const Base b3) {
        return kCodigo[16 * NaTabela(b1) + 4 * NaTabela(b2) + NaTabela(b3)];
    }

    Familia FamiliaDe(const char aminoacido) {
        switch (aminoacido) {
            case 'G': case 'A': case 'V': case 'L': case 'I': case 'M': case 'F': case 'W': case 'P':
                return Familia::Hidrofobico;
            case 'K': case 'R': case 'H':
                return Familia::Positivo;
            case 'D': case 'E':
                return Familia::Negativo;
            case '*':
                return Familia::Parada;
            default:
                return Familia::Polar;
        }
    }

    const char* NomeDo(const char aminoacido) {
        switch (aminoacido) {
            case 'A': return "Ala";
            case 'R': return "Arg";
            case 'N': return "Asn";
            case 'D': return "Asp";
            case 'C': return "Cys";
            case 'Q': return "Gln";
            case 'E': return "Glu";
            case 'G': return "Gly";
            case 'H': return "His";
            case 'I': return "Ile";
            case 'L': return "Leu";
            case 'K': return "Lys";
            case 'M': return "Met";
            case 'F': return "Phe";
            case 'P': return "Pro";
            case 'S': return "Ser";
            case 'T': return "Thr";
            case 'W': return "Trp";
            case 'Y': return "Tyr";
            case 'V': return "Val";
            default: return "Pare";
        }
    }

    const char* NomeDa(const Familia f) {
        switch (f) {
            case Familia::Hidrofobico: return "hidrof\xc3\xb3" "bico";
            case Familia::Polar: return "polar";
            case Familia::Positivo: return "positivo";
            case Familia::Negativo: return "negativo";
            case Familia::Parada: return "parada";
        }
        return "";
    }

    std::vector<Base> Gene(Variacao::Sorteio& s, const int codons) {
        std::vector<Base> v = {Base::A, Base::U, Base::G};
        for (int c = 1; c < codons - 1; ++c) {
            Base b1, b2, b3;
            do {
                b1 = kRna[static_cast<size_t>(s.Inteiro(0, 3))];
                b2 = kRna[static_cast<size_t>(s.Inteiro(0, 3))];
                b3 = kRna[static_cast<size_t>(s.Inteiro(0, 3))];
            } while (Traduzir(b1, b2, b3) == '*');
            v.insert(v.end(), {b1, b2, b3});
        }
        if (codons >= 2) {
            // UAA, UAG ou UGA.
            const int p = s.Inteiro(0, 2);
            v.push_back(Base::U);
            v.push_back(p == 2 ? Base::G : Base::A);
            v.push_back(p == 1 ? Base::G : Base::A);
        }
        return v;
    }

    Vector2 NaOrbita(const int k, const int n, const float t, const Vector2 centro, const Vector2 raios,
                     const float rapidez) {
        const float a = rapidez * t + Math::TwoPi * static_cast<float>(k) / static_cast<float>(std::max(1, n));
        return centro + Vector2(raios.x * Math::Cos(a), raios.y * Math::Sin(a));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.rapidezDaOrbita = 0.55f;
        d.voltaDaCapsula = 1.3f;
        d.entreOndas = 1.35f;
        d.avisoDaOnda = 0.9f;
        d.linhasDaOnda = 2;
        d.basesPorLinha = 24;
        d.fracaoDominante = 0.75f;
        d.velocidadeDaOnda = 175.0f;
        d.porBase = 0.24f;
        d.tirosPorBase = 3;
        d.velocidadeDoRna = 205.0f;
        d.repetir = 0.35f;
        d.porCodon = 0.52f;
        d.tirosPorBaseNaTraducao = 3;
        d.velocidadeHidrofobica = 300.0f;
        d.velocidadePolar = 160.0f;
        d.velocidadePositiva = 185.0f;
        d.giroPositivo = 2.3f;
        d.anelNegativo = 12;
        d.velocidadeNegativa = 175.0f;
        d.quedaDaCadeia = 240.0f;
        return d;
    }
}
