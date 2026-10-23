//
// Os sequenciadores da final do Luizinho: as excecoes.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>

#include "../Source/Attacks/Excecao.h"

using namespace Excecao;

namespace {
    int Contar(const std::vector<Passo>& passos, const Acontece o) {
        return static_cast<int>(std::count_if(passos.begin(), passos.end(), [o](const Passo& p) { return p.o == o; }));
    }
}

TEST_CASE("Excecao: a hierarquia - o catch do tipo de cima trata o de baixo") {
    CHECK(EhUm(Tipo::NullPointerException, Tipo::RuntimeException));
    CHECK(EhUm(Tipo::NullPointerException, Tipo::Exception));
    CHECK(EhUm(Tipo::IOException, Tipo::Exception));
    CHECK(EhUm(Tipo::Reprovado, Tipo::Exception));
    CHECK(EhUm(Tipo::IOException, Tipo::IOException));
    CHECK_FALSE(EhUm(Tipo::IOException, Tipo::RuntimeException));
    CHECK_FALSE(EhUm(Tipo::NullPointerException, Tipo::IOException));
    CHECK_FALSE(EhUm(Tipo::Exception, Tipo::IOException));
    CHECK(Pai(Tipo::Exception) == Tipo::Exception);
}

TEST_CASE("Excecao: a excecao sobe ate quem a trata, e cada faixa sem tratamento cai") {
    // Rodada 0: o catch de f trata a IOException; o finally de g roda no caminho.
    auto p = Propagar(RodadaDaVez(0));
    REQUIRE(p.size() == 4u);
    CHECK(p[0].quadro == 3);
    CHECK(p[0].o == Acontece::Desempilha);   // h, quem lancou
    CHECK(p[1].o == Acontece::Finally);      // g
    CHECK(p[2].o == Acontece::Desempilha);   // g
    CHECK(p[2].quadro == 2);
    CHECK(p[3].o == Acontece::Captura);      // f
    CHECK(p[3].quadro == 1);
    CHECK(QuemCaptura(RodadaDaVez(0)) == 1);

    // A MESMA PILHA COM OUTRA EXCECAO vai ate o main.
    CHECK(QuemCaptura(RodadaDaVez(1)) == 0);
    CHECK(Contar(Propagar(RodadaDaVez(1)), Acontece::Desempilha) == 3);

    // A HIERARQUIA: o catch (RuntimeException) de f trata a NullPointerException.
    CHECK(QuemCaptura(RodadaDaVez(2)) == 1);
    CHECK(Contar(Propagar(RodadaDaVez(2)), Acontece::Desempilha) == 1);

    // NINGUEM TRATA: a pilha inteira cai, e a excecao escapa do main.
    const auto tudo = Propagar(RodadaDaVez(3));
    CHECK(QuemCaptura(RodadaDaVez(3)) == -1);
    CHECK(Contar(tudo, Acontece::Desempilha) == 4);
    CHECK(Contar(tudo, Acontece::Finally) == 1);
    CHECK(tudo.back().o == Acontece::Escapa);

    // A REPESCAGEM E UM CATCH.
    CHECK(RodadaDaVez(4).lancada == Tipo::Reprovado);
    CHECK(QuemCaptura(RodadaDaVez(4)) == 0);
    CHECK(PlacarDoDesfecho(RodadaDaVez(4)).find("repescagem();") != std::string::npos);
}

TEST_CASE("Excecao: a excecao sobe uma faixa a cada kPorQuadro, e a rodada cabe no relogio") {
    CHECK(kPrimeiro + static_cast<float>(kRodadas) * kPorRodada == doctest::Approx(Fase::kDuracao));
    for (int k = 0; k < kRodadas; ++k) {
        CAPTURE(k);
        const Rodada r = RodadaDaVez(k);
        REQUIRE(!r.pilha.empty());
        CHECK(static_cast<int>(r.pilha.size()) <= kMaiorPilha);
        CHECK(r.pilha.front().funcao == "main");
        // As chamadas empilham antes do lancamento.
        CHECK(static_cast<float>(r.pilha.size() - 1) * kEntreChamadas < kLancamento);
        const auto passos = Propagar(r);
        const int ultimo = static_cast<int>(r.pilha.size()) - 1;
        for (const Passo& p : passos) {
            if (p.quadro >= 0) CHECK(p.quando == doctest::Approx(static_cast<float>(ultimo - p.quadro) * kPorQuadro));
        }
        // O desfecho e o retorno de quem sobrou acontecem antes da rodada seguinte.
        CHECK(kLancamento + passos.back().quando + kRetorno < kPorRodada);
    }
    CHECK(RodadaDaVez(kRodadas).lancada == RodadaDaVez(0).lancada);
}

TEST_CASE("Excecao: as faixas da pilha ficam no alto, uma embaixo da outra, longe do aluno") {
    for (int k = 0; k < kMaiorPilha; ++k) {
        const Retangulo f = Faixa(k, 1200.0f);
        CHECK(f.x == doctest::Approx(kMargem));
        CHECK(f.x + f.w == doctest::Approx(1200.0f - kMargem));
        // Abaixo do placar (Battle: 12 + 46).
        CHECK(f.y >= 60.0f);
        if (k > 0) {
            const Retangulo cima = Faixa(k - 1, 1200.0f);
            CHECK(f.y >= cima.y + cima.h);
        }
    }
    // A MAIS FUNDA PARA BEM ACIMA DO ALUNO (Player: y ate 698).
    const Retangulo funda = Faixa(kMaiorPilha - 1, 1200.0f);
    CHECK(funda.y + funda.h < 400.0f);
}

TEST_CASE("Excecao: a cortina poe um destroco em cima do aluno, de borda a borda") {
    for (const float x : {600.0f, 21.0f, 1179.0f, 333.3f, 0.0f, 1200.0f}) {
        CAPTURE(x);
        const auto xs = Cortina(x, 1200.0f);
        REQUIRE(xs.size() >= 2u);
        const float alvo = std::clamp(x, kMargem, 1200.0f - kMargem);
        bool emCima = false;
        for (const float c : xs) emCima = emCima || std::fabs(c - alvo) < 0.01f;
        CHECK(emCima);
        CHECK(xs.front() >= kMargem - 0.01f);
        CHECK(xs.front() < kMargem + kEntreDestrocos);
        CHECK(xs.back() <= 1200.0f - kMargem + 0.01f);
        CHECK(xs.back() > 1200.0f - kMargem - kEntreDestrocos);
        for (size_t i = 1; i < xs.size(); ++i) CHECK(xs[i] - xs[i - 1] == doctest::Approx(kEntreDestrocos));
    }
}

TEST_CASE("Excecao: a excecao sobe sem cobrir o canto de nenhuma faixa") {
    // O canto fica na esquerda da faixa (Luizinho: 10 px da borda, a largura de
    // Escopo::LarguraDoCanto, 9,5 px por letra mais 18); a excecao, de 168 px,
    // sobe kAEsquerda a esquerda do professor, que fica de kXMinimo a kXMaximo.
    float maiorCanto = 0.0f;
    for (int k = 0; k < kRodadas; ++k) {
        for (const Quadro& q : RodadaDaVez(k).pilha) {
            const float w = static_cast<float>(Rotulo(q).size()) * 9.5f + 18.0f;
            maiorCanto = std::max(maiorCanto, kMargem + 10.0f + w);
        }
    }
    CHECK(kXMinimo - kAEsquerda - 168.0f / 2.0f > maiorCanto);
    CHECK(kXMaximo - kAEsquerda + 168.0f / 2.0f < 1200.0f - kMargem);
}

TEST_CASE("Excecao: os textos") {
    CHECK(Rotulo(Quadro{"f", Tipo::IOException, false}) == "f() \xc2\xb7 catch (IOException e)");
    CHECK(Rotulo(Quadro{"g", std::nullopt, true}) == "g() \xc2\xb7 finally");
    CHECK(Rotulo(Quadro{"h", std::nullopt, false}) == "h()");
    CHECK(PlacarDasChamadas(RodadaDaVez(0)) == "main() \xe2\x86\x92 f() \xe2\x86\x92 g() \xe2\x86\x92 h()");
    CHECK(PlacarDoLancamento(RodadaDaVez(0)).find("h(): throw new IOException();") != std::string::npos);
    CHECK(PlacarDoDesfecho(RodadaDaVez(0)) == "throw new IOException();\ncapturada em f()");
    CHECK(PlacarDoDesfecho(RodadaDaVez(3)) == "Exception in thread \"main\"\njava.lang.NullPointerException");
    CHECK(CaraDaExcecao(Tipo::IOException) == "IOException");
}
