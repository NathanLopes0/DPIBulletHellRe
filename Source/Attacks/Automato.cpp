//
// O automato finito da fase 1 do Vladimir.
//

#include "Automato.h"

#include "Alfabeto.h"

namespace Automato {

    namespace {
        /// A tabela de "termina em ab", linha por estado: {lendo a, lendo b}.
        constexpr int kTabela[kEstados][2] = {
            {1, 0},   // q0
            {1, 2},   // q1
            {1, 0},   // q2
        };

        /// "₀" em UTF-8 e E2 82 80; os outros digitos seguem em ordem.
        std::string Subscrito(const int d) {
            std::string s = "\xE2\x82";
            s += static_cast<char>(0x80 + d);
            return s;
        }

        /// O epsilon, a palavra vazia: CE B5 em UTF-8.
        constexpr const char* kVazia = "\xCE\xB5";
    }

    int Proximo(const int q, const char letra) {
        if (q < 0 || q >= kEstados) return q;
        if (letra == 'a') return kTabela[q][0];
        if (letra == 'b') return kTabela[q][1];
        return q;
    }

    bool Aceita(const int q) {
        return q == kAceitacao;
    }

    int Rodar(const std::string& palavra) {
        int q = kInicial;
        for (const char c : palavra) q = Proximo(q, c);
        return q;
    }

    std::string Sortear(const std::uint32_t sorteio) {
        // O TAMANHO E AS LETRAS SAEM DE PARTES DIFERENTES DO SORTEIO: os bits
        // de baixo sao as letras, e o que sobra decide o tamanho.
        const int faixa = kMaiorPalavra - kMenorPalavra + 1;
        const int tamanho = kMenorPalavra + static_cast<int>((sorteio >> 16) % static_cast<std::uint32_t>(faixa));
        std::string palavra;
        for (int i = 0; i < tamanho; ++i) palavra += ((sorteio >> i) & 1u) ? 'b' : 'a';
        return palavra;
    }

    std::string Resto(const Leitura& l) {
        return l.lidos >= l.palavra.size() ? std::string() : l.palavra.substr(l.lidos);
    }

    bool Terminou(const Leitura& l) {
        return l.lidos >= l.palavra.size();
    }

    Passo Ler(Leitura& l) {
        Passo p;
        p.de = l.estado;
        p.para = l.estado;
        if (Terminou(l)) return p;
        p.letra = l.palavra[l.lidos++];
        p.para = Proximo(l.estado, p.letra);
        l.estado = p.para;
        return p;
    }

    void Trocar(Leitura& l) {
        if (Terminou(l)) return;
        l.palavra = l.palavra.substr(0, l.lidos) + Alfabeto::TrocarPalavra(Resto(l));
    }

    std::string NomeDoEstado(const int q) {
        return "q" + Subscrito(q < 0 || q > 9 ? 0 : q);
    }

    std::string Configuracao(const Leitura& l) {
        const std::string resto = Resto(l);
        return "(" + NomeDoEstado(l.estado) + ", " + (resto.empty() ? std::string(kVazia) : resto) + ")";
    }

    std::string Veredito(const Leitura& l) {
        return Configuracao(l) + (Aceita(l.estado) ? " aceita" : " rejeita");
    }
}
