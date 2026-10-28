//
// O clock da fase 2 do Ricardo.
//

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <vector>
#include "../Source/Attacks/Fase.h"

#include "../Source/Attacks/Relogio.h"

TEST_CASE("Relogio: o placar e o clock - le o numero antes de Hz") {
    CHECK(*Relogio::LerFrequencia("clock 1 Hz") == doctest::Approx(1.0f));
    CHECK(*Relogio::LerFrequencia("clock 2 Hz") == doctest::Approx(2.0f));
    CHECK(*Relogio::LerFrequencia("overclock 8 Hz") == doctest::Approx(8.0f));
    CHECK(*Relogio::LerFrequencia("4Hz") == doctest::Approx(4.0f));
    CHECK(*Relogio::LerFrequencia("clock 1,5 Hz") == doctest::Approx(1.5f));   // a virgula do placar
    CHECK_FALSE(Relogio::LerFrequencia("clock"));
    CHECK_FALSE(Relogio::LerFrequencia("clock Hz"));
    CHECK_FALSE(Relogio::LerFrequencia("clock 0 Hz"));
    CHECK_FALSE(Relogio::LerFrequencia("clock 1.2.3 Hz"));
    CHECK_FALSE(Relogio::LerFrequencia(""));
}

TEST_CASE("Relogio: as bordas de subida ficam em k/f, e o zero nao e borda") {
    // A 1 Hz, de quadro em quadro por 3 s: tres bordas, em 1, 2 e 3.
    int bordas = 0;
    float primeira = -1.0f;
    float antes = 0.0f;
    for (int q = 1; q <= 3 * 60; ++q) {
        const float agora = static_cast<float>(q) / 60.0f;
        if (Relogio::Subiu(antes, agora, 1.0f)) {
            ++bordas;
            if (primeira < 0.0f) primeira = agora;
        }
        antes = agora;
    }
    CHECK(bordas == 3);
    CHECK(primeira == doctest::Approx(1.0f).epsilon(0.02));

    // A 8 Hz, num segundo, oito.
    bordas = 0;
    antes = 0.0f;
    for (int q = 1; q <= 60; ++q) {
        const float agora = static_cast<float>(q) / 60.0f;
        if (Relogio::Subiu(antes, agora, 8.0f)) ++bordas;
        antes = agora;
    }
    CHECK(bordas == 8);

    CHECK_FALSE(Relogio::Subiu(0.0f, 0.01f, 1.0f));     // o comeco da fase nao e borda
    CHECK_FALSE(Relogio::Subiu(0.5f, 0.5f, 1.0f));      // sem tempo, sem borda
    CHECK_FALSE(Relogio::Subiu(0.2f, 0.9f, 0.0f));      // sem frequencia, sem borda
    CHECK(Relogio::Subiu(0.99f, 1.01f, 1.0f));
}

TEST_CASE("Relogio: todo segundo inteiro e borda de qualquer clock inteiro") {
    for (const float f : {1.0f, 2.0f, 4.0f, 8.0f}) {
        CAPTURE(f);
        for (int s = 1; s <= 16; ++s) {
            CHECK(Relogio::Subiu(static_cast<float>(s) - 0.004f, static_cast<float>(s) + 0.004f, f));
        }
    }
}

TEST_CASE("Relogio: o nivel e alto na primeira metade do periodo") {
    CHECK(Relogio::Alto(0.1f, 1.0f));
    CHECK(Relogio::Alto(0.49f, 1.0f));
    CHECK_FALSE(Relogio::Alto(0.51f, 1.0f));
    CHECK_FALSE(Relogio::Alto(0.99f, 1.0f));
    CHECK(Relogio::Alto(1.01f, 1.0f));
    CHECK(Relogio::Alto(0.05f, 8.0f));
    CHECK_FALSE(Relogio::Alto(0.07f, 8.0f));   // meio periodo de 8 Hz e 0,0625
}

TEST_CASE("Relogio: o passo do registrador acaba antes da borda seguinte") {
    for (const float f : {1.0f, 2.0f, 4.0f, 8.0f, 16.0f}) {
        CAPTURE(f);
        CHECK(Relogio::DuracaoDoPasso(f) > 0.0f);
        CHECK(Relogio::DuracaoDoPasso(f) < 0.5f / f);
    }
    CHECK(Relogio::DuracaoDoPasso(1.0f) == doctest::Approx(0.08f));
}

TEST_CASE("Relogio: o contador Johnson enche pela esquerda e esvazia pela esquerda") {
    const std::uint32_t esperado[] = {0x0, 0x8, 0xC, 0xE, 0xF, 0x7, 0x3, 0x1, 0x0, 0x8};
    for (int k = 0; k < 10; ++k) {
        CAPTURE(k);
        CHECK(Relogio::Johnson(k, 4) == esperado[k]);
    }
    CHECK(Relogio::Johnson(8, 8) == 0xFFu);
    CHECK(Relogio::Johnson(9, 8) == 0x7Fu);
    CHECK(Relogio::Johnson(-1, 4) == 0x1u);   // a volta tambem vale para tras

    // De um estado ao seguinte muda UM bit so - e o que faz do Johnson um
    // contador sem tropeco, e da fresta do leque uma coisa que anda.
    for (int k = 0; k < 16; ++k) {
        const std::uint32_t d = Relogio::Johnson(k, 8) ^ Relogio::Johnson(k + 1, 8);
        CHECK(d != 0u);
        CHECK((d & (d - 1u)) == 0u);
    }
}

TEST_CASE("Relogio: o tempo de cada clock da luta - inteiros de 4 a 6 que somam a fase") {
    std::set<std::vector<int>> vistas;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s(semente);
        const Relogio::Plano p = Relogio::Sortear(s);
        int soma = 0;
        for (const int d : p.duracoes) {
            CHECK(d >= 4);
            CHECK(d <= 6);
            soma += d;
        }
        CHECK(static_cast<float>(soma) == doctest::Approx(Fase::kDuracao));
        CHECK(p.inicioDoContador >= 0);
        CHECK(p.inicioDoContador <= Relogio::kMaiorInicioDoContador);
        vistas.insert(std::vector<int>(p.duracoes.begin(), p.duracoes.end()));
    }
    CHECK(vistas.size() > 8u);
    // TODA TROCA CAI NUMA BORDA: os fins sao segundos inteiros, e todo
    // segundo inteiro e borda de 1, 2, 4 e 8 Hz.
    for (const float f : {1.0f, 2.0f, 4.0f, 8.0f}) {
        for (int t = 4; t <= 16; ++t) CHECK(Relogio::Subiu(static_cast<float>(t) - 0.001f, static_cast<float>(t), f));
    }
    CHECK(Relogio::DegrauDo(Dificuldade::Nivel::Normal).passo == doctest::Approx(Relogio::kPasso));
    CHECK(Relogio::DegrauDo(Dificuldade::Nivel::Dificil).passo > Relogio::kPasso);
}
