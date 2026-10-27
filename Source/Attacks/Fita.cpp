//
// A fita da maquina de Turing do Vladimir: o castor mais ocupado de quatro
// estados, rodando de verdade.
//

#include "Fita.h"

namespace Fita {

    const Tabela& Castor() {
        // A (q0), B (q1), C (q2), D (q3): {escreve, sentido, proximo} lendo
        // branco e lendo a marca.
        static const Tabela tabela = {{
            {{ {1, +1, 1}, {1, -1, 1} }},          // A: 1RB  1LB
            {{ {1, -1, 0}, {0, -1, 2} }},          // B: 1LA  0LC
            {{ {1, +1, kParada}, {1, -1, 3} }},    // C: 1RH  1LD
            {{ {1, +1, 3}, {0, +1, 0} }},          // D: 1RD  0RA
        }};
        return tabela;
    }

    float XDaCasa(const int casa, const float largura) {
        return largura / 2.0f + (static_cast<float>(casa) - static_cast<float>(kCasas - 1) / 2.0f) * kEntreCasas;
    }

    float XDoProfessor(const float xDaCabeca, const float largura) {
        return largura / 2.0f + kInclinacao * (xDaCabeca - largura / 2.0f);
    }

    Estado Inicio(const bool espelho) {

        Estado e;
        e.cabeca = espelho ? kCasas - 1 - kOrigem : kOrigem;
        return e;
    }

    Passo Avancar(Estado& e, const bool espelho) {
        Passo p;
        p.casa = e.cabeca;
        p.de = e.q;
        if (e.parou || e.cabeca < 0 || e.cabeca >= kCasas) {
            e.parou = true;
            p.para = kParada;
            return p;
        }
        const auto casa = static_cast<size_t>(e.cabeca);
        p.leu = e.fita[casa];
        const Regra& r = Castor()[static_cast<size_t>(e.q)][static_cast<size_t>(p.leu)];
        p.escreveu = r.escreve;
        p.para = r.proximo;
        p.sentido = espelho ? -r.sentido : r.sentido;
        e.fita[casa] = r.escreve;
        e.cabeca += p.sentido;
        e.q = r.proximo;
        ++e.passos;
        if (r.proximo == kParada) e.parou = true;
        return p;
    }

    int Marcas(const Estado& e) {
        int n = 0;
        for (const int s : e.fita) n += s;
        return n;
    }

    namespace {
        // Os subscritos de 0 a 3 sao E2 82 80..83. Quem para vai para "pare":
        // a Carlito nao tem o h subscrito do q-halt.
        std::string Q(const int q) {
            if (q == kParada) return "pare";
            return std::string("q\xE2\x82") + static_cast<char>(0x80 + q);
        }
        std::string Simbolo(const int s) { return s ? "a" : "\xE2\x96\xA1"; }   // o branco e o quadrado vazio
    }

    std::string Transicao(const Passo& p) {
        const std::string seta = p.sentido > 0 ? "\xE2\x86\x92" : "\xE2\x86\x90";
        return "\xCE\xB4(" + Q(p.de) + ", " + Simbolo(p.leu) + ") = (" + Q(p.para) + ", " +
               Simbolo(p.escreveu) + ", " + seta + ")";
    }

    std::string Placar(const Estado& e, const Passo& p) {
        if (e.parou) {
            // \xce\xa3 e o sigma maiusculo.
            return Transicao(p) + "\nparou: " + std::to_string(e.passos) + " passos, \xCE\xA3(4) = " +
                   std::to_string(Marcas(e));
        }
        return Transicao(p) + "\npasso " + std::to_string(e.passos) + " \xC2\xB7 " + std::to_string(Marcas(e)) +
               " marcas";
    }


    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        p.espelho = s.Chance(0.5f);
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.apagadaPersegue = Dificuldade::Escolher(n, false, true);
        d.relerAtira = Dificuldade::Escolher(n, false, true);

        return d;
    }

}
