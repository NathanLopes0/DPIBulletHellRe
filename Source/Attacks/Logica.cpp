//
// Os bits e as portas logicas do Ricardo.
//

#include "Logica.h"

#include <algorithm>
#include <cmath>

namespace Logica {

    namespace {

        std::uint32_t Mascara(const int bits) {
            if (bits <= 0) return 0u;
            if (bits >= 32) return 0xFFFFFFFFu;
            return (1u << bits) - 1u;
        }

        /// UM PASSO QUE VISITA TODAS AS PALAVRAS: somar um numero impar,
        /// modulo 2^bits, da a volta inteira antes de repetir.
        constexpr std::uint32_t kPassoDaVolta = 40503u;
    }

    bool Saida(const Operacao op, const bool a, const bool b) {
        switch (op) {
            case Operacao::E:           return a && b;
            case Operacao::Ou:          return a || b;
            case Operacao::OuExclusivo: return a != b;
            case Operacao::Nao:         return !a;
        }
        return a;
    }

    const char* NomeDaOperacao(const Operacao op) {
        switch (op) {
            case Operacao::E:           return "AND";
            case Operacao::Ou:          return "OR";
            case Operacao::OuExclusivo: return "XOR";
            case Operacao::Nao:         return "NOT";
        }
        return "";
    }

    std::optional<Porta> LerPorta(const std::string& texto, const int bits) {
        if (bits <= 0 || bits > 31) return std::nullopt;

        // O NOME: a primeira palavra do texto.
        std::size_t i = texto.find_first_not_of(' ');
        if (i == std::string::npos) return std::nullopt;
        const std::size_t fimDoNome = texto.find(' ', i);
        const std::string nome = texto.substr(i, fimDoNome == std::string::npos ? std::string::npos : fimDoNome - i);

        Porta p;
        if      (nome == "AND") p.operacao = Operacao::E;
        else if (nome == "OR")  p.operacao = Operacao::Ou;
        else if (nome == "XOR") p.operacao = Operacao::OuExclusivo;
        else if (nome == "NOT") p.operacao = Operacao::Nao;
        else return std::nullopt;

        // A ENTRADA B: so digitos e espacos, e exatamente 'bits' digitos.
        int digitos = 0;
        std::uint32_t b = 0;
        for (std::size_t k = (fimDoNome == std::string::npos ? texto.size() : fimDoNome); k < texto.size(); ++k) {
            const char c = texto[k];
            if (c == ' ') continue;
            if (c != '0' && c != '1') return std::nullopt;
            b = (b << 1) | (c == '1' ? 1u : 0u);
            ++digitos;
        }
        if (p.operacao == Operacao::Nao) {
            if (digitos != 0) return std::nullopt;   // o NOT nao tem B
        }
        else if (digitos != bits) {
            return std::nullopt;
        }
        p.b = b;
        return p;
    }

    bool Bit(const std::uint32_t palavra, const int coluna, const int bits) {
        if (coluna < 0 || coluna >= bits) return false;
        return ((palavra >> (bits - 1 - coluna)) & 1u) != 0u;
    }

    std::uint32_t Aplicar(const Porta& p, const std::uint32_t a, const int bits) {
        std::uint32_t saida = 0;
        for (int c = 0; c < bits; ++c) {
            saida = (saida << 1) | (Saida(p.operacao, Bit(a, c, bits), Bit(p.b, c, bits)) ? 1u : 0u);
        }
        return saida & Mascara(bits);
    }

    std::string AnimacaoDaPorta(const Porta& p, const int coluna, const int bits) {
        std::string nome = NomeDaOperacao(p.operacao);
        if (p.operacao != Operacao::Nao) nome += Bit(p.b, coluna, bits) ? '1' : '0';
        return nome;
    }

    int Coluna(const float x, const float largura, const int bits) {
        if (bits <= 0 || largura <= 0.0f) return 0;
        const int c = static_cast<int>(std::floor(x / (largura / static_cast<float>(bits))));
        return std::clamp(c, 0, bits - 1);
    }

    float CentroDaColuna(const int coluna, const float largura, const int bits) {
        if (bits <= 0) return 0.0f;
        return (static_cast<float>(coluna) + 0.5f) * largura / static_cast<float>(bits);
    }

    int Uns(const std::uint32_t palavra, const int bits) {
        int n = 0;
        for (int c = 0; c < bits; ++c) n += Bit(palavra, c, bits) ? 1 : 0;
        return n;
    }

    std::string Texto(const std::uint32_t palavra, const int bits) {
        std::string s;
        for (int c = 0; c < bits; ++c) {
            if (c > 0 && c % 4 == 0) s += ' ';
            s += Bit(palavra, c, bits) ? '1' : '0';
        }
        return s;
    }

    bool Justa(const Porta& p, const std::uint32_t a, const int bits) {
        const std::uint32_t entrada = a & Mascara(bits);
        const std::uint32_t saida = Aplicar(p, entrada, bits);
        const int unsAntes = Uns(entrada, bits);
        const int unsDepois = Uns(saida, bits);
        return unsAntes >= kMinimoDeUns && bits - unsAntes >= kMinimoDeZeros
            && unsDepois >= kMinimoDeUns && bits - unsDepois >= kMinimoDeZeros
            && saida != entrada;
    }

    std::uint32_t Sortear(const Porta& p, const int bits, const std::uint32_t sorteio) {
        const std::uint32_t m = Mascara(bits);
        std::uint32_t a = sorteio & m;
        const std::uint64_t voltas = static_cast<std::uint64_t>(m) + 1u;
        for (std::uint64_t k = 0; k < voltas; ++k) {
            if (Justa(p, a, bits)) return a;
            a = (a + kPassoDaVolta) & m;
        }
        return sorteio & m;
    }

    int ColunaDoDeslocamento(const float t, const float segundosPorPasso,
                             const int primeira, const int ultima, const int inicio) {
        const int n = ultima - primeira;
        if (n <= 0) return primeira;
        if (segundosPorPasso <= 0.0f) return std::clamp(inicio, primeira, ultima);

        // O VAI E VEM e uma onda triangular sobre os passos: 2n passos por
        // volta, subindo n e descendo n. O inicio entra como passos ja dados.
        const int dados = static_cast<int>(std::floor(std::max(0.0f, t) / segundosPorPasso));
        const int ja = std::clamp(inicio, primeira, ultima) - primeira;
        const int m = (dados + ja) % (2 * n);
        return primeira + (m <= n ? m : 2 * n - m);
    }

    std::string Anuncio(const Porta& p) {
        if (p.operacao == Operacao::Nao) return NomeDaOperacao(p.operacao);
        return std::string(NomeDaOperacao(p.operacao)) + " " + Texto(p.b, kBits);
    }

    Plano Sortear(Variacao::Sorteio& s) {
        static constexpr std::uint32_t kGrupos[] = {0x0u, 0xFu, 0xCu, 0x3u};
        std::vector<Operacao> ops = {Operacao::E, Operacao::Ou, Operacao::OuExclusivo, Operacao::Nao};
        s.Embaralhar(ops);
        Plano plano;
        for (std::size_t k = 0; k < ops.size(); ++k) {
            Porta p;
            p.operacao = ops[k];
            if (p.operacao != Operacao::Nao) {
                // ATE ACHAR UM B QUE ADMITA PALAVRA JUSTA - quase sempre na
                // primeira: so os extremos (quase todo 0 num AND) nao admitem.
                for (int tentativa = 0; tentativa < 64; ++tentativa) {
                    std::uint32_t b = 0;
                    for (int g = 0; g < 4; ++g) b = (b << 4) | kGrupos[s.Inteiro(0, 3)];
                    const int uns = Uns(b, kBits);
                    p.b = b;
                    if (uns < 4 || uns > 12) continue;
                    if (Justa(p, Sortear(p, kBits, s.Bruto()), kBits)) break;
                }
            }
            plano.portas[k] = p;
        }
        return plano;
    }
}
