//
// O automato finito da fase 1 do Vladimir.
//

#include "doctest.h"

#include <string>

#include "../Source/Attacks/Automato.h"

namespace {
    bool TerminaEmAb(const std::string& w) {
        return w.size() >= 2 && w.compare(w.size() - 2, 2, "ab") == 0;
    }
}

TEST_CASE("Automato: a tabela reconhece exatamente as palavras que terminam em ab") {
    // TODAS as palavras de ate oito letras, e nao uns exemplos: a tabela tem
    // seis transicoes, e um erro numa delas so aparece em algumas palavras.
    int conferidas = 0;
    for (int tamanho = 0; tamanho <= 8; ++tamanho) {
        for (int bits = 0; bits < (1 << tamanho); ++bits) {
            std::string w;
            for (int i = 0; i < tamanho; ++i) w += ((bits >> i) & 1) ? 'b' : 'a';
            CAPTURE(w);
            CHECK(Automato::Aceita(Automato::Rodar(w)) == TerminaEmAb(w));
            ++conferidas;
        }
    }
    CHECK(conferidas == 511);
}

TEST_CASE("Automato: os lacos do diagrama sao q0 lendo b e q1 lendo a") {
    CHECK(Automato::Proximo(0, 'b') == 0);
    CHECK(Automato::Proximo(1, 'a') == 1);
    CHECK(Automato::Proximo(2, 'a') == 1);
    CHECK(Automato::Proximo(2, 'b') == 0);
    // Fora do alfabeto, ou fora dos estados, nao anda.
    CHECK(Automato::Proximo(1, 'c') == 1);
    CHECK(Automato::Proximo(7, 'a') == 7);
}

TEST_CASE("Automato: a leitura anda um simbolo por passo, e o placar e a configuracao") {
    Automato::Leitura l{"abb", 0, Automato::kInicial};
    CHECK(Automato::Configuracao(l) == "(q\xE2\x82\x80, abb)");

    auto p = Automato::Ler(l);
    CHECK(p.letra == 'a');
    CHECK(p.de == 0);
    CHECK(p.para == 1);
    CHECK_FALSE(p.Laco());
    CHECK(Automato::Configuracao(l) == "(q\xE2\x82\x81, bb)");

    p = Automato::Ler(l);
    CHECK(p.para == 2);
    p = Automato::Ler(l);
    CHECK(p.letra == 'b');
    CHECK(p.para == 0);
    CHECK(Automato::Terminou(l));
    // A palavra vazia e escrita epsilon, e o veredito vem atras.
    CHECK(Automato::Configuracao(l) == "(q\xE2\x82\x80, \xCE\xB5)");
    CHECK(Automato::Veredito(l) == "(q\xE2\x82\x80, \xCE\xB5) rejeita");

    // Lida a palavra, ler de novo nao anda.
    p = Automato::Ler(l);
    CHECK(p.letra == 0);
    CHECK(p.Laco());

    Automato::Leitura ab{"ab", 0, Automato::kInicial};
    Automato::Ler(ab);
    Automato::Ler(ab);
    CHECK(Automato::Veredito(ab) == "(q\xE2\x82\x82, \xCE\xB5) aceita");
}

TEST_CASE("Automato: o intercambio troca so o que falta ler") {
    Automato::Leitura l{"aabab", 0, Automato::kInicial};
    Automato::Ler(l);
    Automato::Ler(l);           // lidos "aa", esta em q1
    Automato::Trocar(l);
    CHECK(l.palavra == "aaaba");
    CHECK(Automato::Resto(l) == "aba");
    CHECK(l.estado == 1);       // o estado nao muda: a troca e da palavra

    Automato::Leitura lida{"ab", 2, 2};
    Automato::Trocar(lida);
    CHECK(lida.palavra == "ab");
}

TEST_CASE("Automato: a palavra sorteada e de {a, b}, do tamanho da faixa, e a mesma para a mesma semente") {
    bool viuCurta = false, viuLonga = false, viuAceita = false, viuRejeita = false;
    for (std::uint32_t s = 0; s < 4000; s += 7) {
        const std::string w = Automato::Sortear(s * 2654435761u);
        CAPTURE(w);
        CHECK(static_cast<int>(w.size()) >= Automato::kMenorPalavra);
        CHECK(static_cast<int>(w.size()) <= Automato::kMaiorPalavra);
        CHECK(w.find_first_not_of("ab") == std::string::npos);
        viuCurta = viuCurta || static_cast<int>(w.size()) == Automato::kMenorPalavra;
        viuLonga = viuLonga || static_cast<int>(w.size()) == Automato::kMaiorPalavra;
        const bool aceita = Automato::Aceita(Automato::Rodar(w));
        viuAceita = viuAceita || aceita;
        viuRejeita = viuRejeita || !aceita;
    }
    CHECK(viuCurta);
    CHECK(viuLonga);
    CHECK(viuAceita);
    CHECK(viuRejeita);
    CHECK(Automato::Sortear(12345u) == Automato::Sortear(12345u));
}
