//
// A fita da maquina de Turing: o castor mais ocupado de quatro estados.
//

#include "doctest.h"

#include "../Source/Attacks/Vladimir/Fita.h"

#include <algorithm>
#include <cstdint>
#include <string>

using namespace Fita;

namespace {
    struct Corrida {
        Estado fim;
        int escritas = 0;
        int apagadas = 0;
        int relidas = 0;
        int menor = kCasas;
        int maior = -1;
    };

    Corrida Rodar(const bool espelho) {
        Corrida c;
        c.fim = Inicio(espelho);
        for (int k = 0; k < 1000 && !c.fim.parou; ++k) {
            const Passo p = Avancar(c.fim, espelho);
            c.menor = std::min(c.menor, p.casa);
            c.maior = std::max(c.maior, p.casa);
            if (!p.leu && p.escreveu) ++c.escritas;
            if (p.leu && !p.escreveu) ++c.apagadas;
            if (p.leu && p.escreveu) ++c.relidas;
        }
        return c;
    }
}

TEST_CASE("Fita: o castor de 4 estados para em 107 passos com 13 marcas, sem sair da fita") {
    const Corrida c = Rodar(false);
    CHECK(c.fim.parou);
    CHECK(c.fim.passos == kPassos);
    CHECK(Marcas(c.fim) == kMarcas);
    // A FAIXA E A DA TELA: as 14 casas, todas usadas.
    CHECK(c.menor == 0);
    CHECK(c.maior == kCasas - 1);
    // O QUE A FASE FAZ COM ELE: 46 marcas novas, 33 apagadas (os "b" que
    // perseguem) e 28 relidas (as copias).
    CHECK(c.escritas == 46);
    CHECK(c.apagadas == 33);
    CHECK(c.relidas == 28);
}

TEST_CASE("Fita: o espelho e a mesma maquina com esquerda e direita trocadas") {
    const Corrida normal = Rodar(false);
    const Corrida espelho = Rodar(true);
    CHECK(espelho.fim.passos == kPassos);
    CHECK(espelho.menor == 0);
    CHECK(espelho.maior == kCasas - 1);
    for (int k = 0; k < kCasas; ++k) {
        CHECK(espelho.fim.fita[static_cast<size_t>(k)] == normal.fim.fita[static_cast<size_t>(kCasas - 1 - k)]);
    }
}

TEST_CASE("Fita: parada, a maquina nao anda mais") {
    Corrida c = Rodar(false);
    const Estado antes = c.fim;
    const Passo p = Avancar(c.fim, false);
    CHECK(p.para == kParada);
    CHECK(c.fim.passos == antes.passos);
    CHECK(c.fim.fita == antes.fita);
}

TEST_CASE("Fita: o ultimo passo cai antes do fim da fase, e a fita cabe na tela") {
    CHECK(kPrimeiroPasso + static_cast<float>(kPassos - 1) * kPasso < Fase::kDuracao);
    CHECK(XDaCasa(0, 1200.0f) > 100.0f);
    CHECK(XDaCasa(kCasas - 1, 1200.0f) < 1100.0f);
    CHECK(XDaCasa(1, 1200.0f) - XDaCasa(0, 1200.0f) == doctest::Approx(kEntreCasas));
}

TEST_CASE("Fita: o placar e a transicao, com o branco, o h de quem para e a seta da tela") {
    Passo p;
    p.de = 0; p.leu = 0; p.escreveu = 1; p.para = 1; p.sentido = 1;
    CHECK(Transicao(p) == "\xCE\xB4(q\xE2\x82\x80, \xE2\x96\xA1) = (q\xE2\x82\x81, a, \xE2\x86\x92)");
    p.de = 2; p.leu = 0; p.escreveu = 1; p.para = kParada; p.sentido = -1;
    CHECK(Transicao(p) == "\xCE\xB4(q\xE2\x82\x82, \xE2\x96\xA1) = (pare, a, \xE2\x86\x90)");

    Estado e = Inicio(false);
    const Passo primeiro = Avancar(e, false);
    CHECK(Placar(e, primeiro) == Transicao(primeiro) + "\npasso 1 \xC2\xB7 1 marcas");
    const Corrida c = Rodar(false);
    CHECK(Placar(c.fim, primeiro).find("\xCE\xA3(4) = 13") != std::string::npos);
}

TEST_CASE("Fita: cada luta sorteia a maquina como ela e ou espelhada, as duas acontecem") {
    int espelhadas = 0;
    constexpr int kLutas = 400;
    for (std::uint32_t semente = 0; semente < kLutas; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDaFita);
        if (Sortear(s).espelho) ++espelhadas;
    }
    CHECK(espelhadas > kLutas / 4);
    CHECK(espelhadas < 3 * kLutas / 4);
}

TEST_CASE("Fita: so no Dificil a marca apagada persegue e a relida atira") {
    CHECK_FALSE(DegrauDo(Dificuldade::Nivel::Normal).apagadaPersegue);
    CHECK_FALSE(DegrauDo(Dificuldade::Nivel::Normal).relerAtira);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).apagadaPersegue);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).relerAtira);
}
