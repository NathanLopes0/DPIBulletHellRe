//
// A fase 3 do Luizinho: a complexidade.
//

#include "doctest.h"

#include <algorithm>
#include <set>

#include "../Source/Attacks/Complexidade.h"

using namespace Complexidade;

TEST_CASE("Complexidade: quantas operacoes cada ordem faz") {
    CHECK(Operacoes(Ordem::Constante, 64) == 1);
    CHECK(Operacoes(Ordem::Logaritmica, 16) == 5);
    CHECK(Operacoes(Ordem::Logaritmica, 64) == 7);
    CHECK(Operacoes(Ordem::Linear, 10) == 10);
    CHECK(Operacoes(Ordem::Linearitmica, 8) == 24);
    CHECK(Operacoes(Ordem::Quadratica, 6) == 36);
    CHECK(Operacoes(Ordem::Exponencial, 5) == 32);
    CHECK(Passadas(Ordem::Linear, 10) == 1);
    CHECK(Passadas(Ordem::Linearitmica, 8) == 3);
    CHECK(Passadas(Ordem::Quadratica, 6) == 6);
    CHECK(Passadas(Ordem::Logaritmica, 32) == 0);
    // As passadas pelo leque fazem as operacoes da ordem.
    for (const Ordem o : {Ordem::Linear, Ordem::Linearitmica, Ordem::Quadratica}) {
        for (int n = 2; n <= 9; ++n) CHECK(Passadas(o, n) * n == Operacoes(o, n));
    }
}

TEST_CASE("Complexidade: a notacao, o algoritmo e a cara do custo") {
    CHECK(Notacao(Ordem::Quadratica) == "O(n\xC2\xB2)");
    CHECK(Notacao(Ordem::Exponencial) == "O(2\xE2\x81\xBF)");
    CHECK(Algoritmo(Ordem::Logaritmica) == "busca bin\xC3\xA1ria");
    CHECK(CaraDaOperacao(Ordem::Constante) == "Barata");
    CHECK(CaraDaOperacao(Ordem::Linearitmica) == "Media");
    CHECK(CaraDaOperacao(Ordem::Exponencial) == "Cara");
    CHECK(Placar(Rodada{Ordem::Logaritmica, 32}) == "busca bin\xC3\xA1ria: O(log n)\nn = 32 \xE2\x86\x92 6 opera\xC3\xA7\xC3\xB5" "es");
    CHECK(Placar(Rodada{Ordem::Constante, 8}) == "acesso v[i]: O(1)\nn = 8 \xE2\x86\x92 1 opera\xC3\xA7\xC3\xA3o");
}

TEST_CASE("Complexidade: nenhuma rodada passa de 64 operacoes, e a busca binaria le um vetor muito maior") {
    const int aMais = DegrauDo(Dificuldade::Nivel::Dificil).nAMais;
    for (int o = 0; o < kOrdens; ++o) {
        const auto ordem = static_cast<Ordem>(o);
        CAPTURE(Notacao(ordem));
        CHECK(MenorN(ordem) <= MaiorN(ordem));
        // O maior n, com o que o Dificil acrescenta nas que percorrem o vetor.
        const bool percorre = ordem == Ordem::Linear || ordem == Ordem::Linearitmica || ordem == Ordem::Quadratica;
        CHECK(Operacoes(ordem, MaiorN(ordem) + (percorre ? aMais : 0)) <= 64);
    }
    CHECK(MaiorN(Ordem::Logaritmica) > 4 * MaiorN(Ordem::Quadratica));
}

TEST_CASE("Complexidade: a rodada fica na faixa da ordem, sobe com a vez e nao repete a anterior") {
    std::set<Ordem> vistas;
    for (std::uint32_t s = 0; s < 400; ++s) {
        const int anterior = static_cast<int>(s % kOrdens);
        const Rodada r = Proxima(static_cast<int>(s % 8), s, Plano{}, anterior);
        vistas.insert(r.ordem);
        CHECK(static_cast<int>(r.ordem) != anterior);
        CHECK(r.n >= MenorN(r.ordem));
        CHECK(r.n <= MaiorN(r.ordem));
    }
    CHECK(vistas.size() == static_cast<size_t>(kOrdens));
    // A mesma sorte, mais tarde na fase: o n nao diminui.
    for (std::uint32_t s = 0; s < 100; ++s) CHECK(Proxima(6, s).n >= Proxima(0, s).n);
    // O Dificil alonga so as que percorrem o vetor.
    for (std::uint32_t s = 0; s < 100; ++s) {
        const Rodada a = Proxima(3, s), b = Proxima(3, s, Plano{}, -1, 1);
        CHECK(a.ordem == b.ordem);
        const bool percorre = a.ordem == Ordem::Linear || a.ordem == Ordem::Linearitmica || a.ordem == Ordem::Quadratica;
        CHECK(b.n == a.n + (percorre ? 1 : 0));
    }
}

TEST_CASE("Complexidade: o leque vai de um lado ao outro, e cada angulo cai na casa certa") {
    for (const int n : {1, 3, 8, 64}) {
        CAPTURE(n);
        float anterior = -1e9f;
        for (int i = 0; i < n; ++i) {
            const float a = AnguloDaCasa(i, n);
            CHECK(a > anterior);
            CHECK(a > 90.0f - kAbertura / 2.0f);
            CHECK(a < 90.0f + kAbertura / 2.0f);
            CHECK(CasaDoAngulo(a, n) == i);
            anterior = a;
        }
        CHECK(CasaDoAngulo(-180.0f, n) == 0);
        CHECK(CasaDoAngulo(260.0f, n) == n - 1);
    }
}

TEST_CASE("Complexidade: o tiro se espalha dentro da casa, e o que acha o aluno mira nele sem sair dela") {
    for (const int n : {4, 12, 64}) {
        CAPTURE(n);
        const float meiaCasa = kAbertura / static_cast<float>(n) / 2.0f;
        for (int i = 0; i < n; ++i) {
            for (const float sorte : {0.0f, 0.25f, 0.5f, 0.75f, 0.999f}) CHECK(CasaDoAngulo(AnguloNaCasa(i, n, sorte), n) == i);
            CHECK(AnguloNaCasa(i, n, 0.5f) == doctest::Approx(AnguloDaCasa(i, n)));
            CHECK(AnguloNaCasa(i, n, 0.0f) < AnguloNaCasa(i, n, 1.0f));
            // Quem esta na casa leva o tiro em cheio; quem esta fora, na borda da casa.
            const float dentro = AnguloDaCasa(i, n) + 0.6f * meiaCasa;
            CHECK(PresoNaCasa(dentro, i, n) == doctest::Approx(dentro));
            CHECK(PresoNaCasa(-180.0f, i, n) == doctest::Approx(AnguloDaCasa(i, n) - meiaCasa));
            CHECK(PresoNaCasa(260.0f, i, n) == doctest::Approx(AnguloDaCasa(i, n) + meiaCasa));
        }
    }
}

TEST_CASE("Complexidade: a busca binaria acha qualquer casa em no maximo log n leituras") {
    for (const int n : {1, 2, 7, 16, 64}) {
        for (int alvo = 0; alvo < n; ++alvo) {
            Intervalo i{0, n - 1};
            int leituras = 0;
            bool achou = false;
            while (!i.Vazio()) {
                const int meio = Meio(i);
                ++leituras;
                if (meio == alvo) achou = true;
                i = Descartar(i, alvo);
            }
            CHECK(achou);
            CHECK(leituras <= Operacoes(Ordem::Logaritmica, n));
        }
    }
}

TEST_CASE("Complexidade: as curvas do grafico crescem na ordem certa e param no teto") {
    for (float n = 1.0f; n <= kNoGrafico; n += 0.5f) {
        CHECK(NaCurva(Ordem::Constante, n) <= NaCurva(Ordem::Logaritmica, n) + 0.001f);
        CHECK(NaCurva(Ordem::Logaritmica, n) <= NaCurva(Ordem::Linear, n) + 1.0f);
        CHECK(NaCurva(Ordem::Linear, n) <= NaCurva(Ordem::Linearitmica, n) + 0.001f);
        for (int o = 0; o < kOrdens; ++o) CHECK(NaCurva(static_cast<Ordem>(o), n) <= kTetoDoGrafico);
    }
    CHECK(NaCurva(Ordem::Exponencial, 8.0f) == doctest::Approx(kTetoDoGrafico));
}

TEST_CASE("Complexidade: a duracao segue as operacoes, e nenhuma rodada passa de quatro segundos e meio") {
    CHECK(Duracao(Rodada{Ordem::Constante, 8}) == doctest::Approx(0.0f));
    CHECK(Duracao(Rodada{Ordem::Linear, 10}) == doctest::Approx(kNaPassada * 9.0f));
    // No ritmo do Normal, ate com o n do Dificil.
    const int aMais = DegrauDo(Dificuldade::Nivel::Dificil).nAMais;
    for (int o = 0; o < kOrdens; ++o) {
        const auto ordem = static_cast<Ordem>(o);
        CAPTURE(Notacao(ordem));
        CHECK(Duracao(Rodada{ordem, MaiorN(ordem) + aMais}) < 4.5f);
    }
}

TEST_CASE("Complexidade: os pesos de cada luta e o degrau do Dificil") {
    for (std::uint32_t s = 0; s < 100; ++s) {
        Variacao::Sorteio sorteio(s);
        const Plano p = Sortear(sorteio);
        CHECK(p.pesoDasBaratas >= 0.2f);
        CHECK(p.pesoDasCaras <= 0.4f);
    }
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(n.ritmo == doctest::Approx(1.0f));
    CHECK(n.nAMais == 0);
    CHECK(d.rapidez > n.rapidez);
    CHECK(d.ritmo < n.ritmo);
    CHECK(n.tamanho == doctest::Approx(1.0f));
    CHECK(d.tamanho > n.tamanho);
    CHECK(d.depois < n.depois);
    CHECK(d.nAMais > n.nAMais);
}
