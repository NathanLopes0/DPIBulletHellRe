//
// O modo daltonismo: a correcao das cores e o lugar dele nas opcoes.
//

#include "doctest.h"

#include <cstdlib>

#include "../Source/Interface/Daltonismo.h"
#include "../Source/Motor/Configuracao.h"

using namespace Daltonismo;

namespace {
    int Distancia(const Rgb a, const Rgb b) {
        return std::abs(a.r - b.r) + std::abs(a.g - b.g) + std::abs(a.b - b.b);
    }

    /// Quem nao separa vermelho de verde ainda ve o eixo azul-amarelo: a correcao tem de afastar as
    /// duas cores nesse eixo, e nao no brilho.
    int NoEixoQueSobra(const Rgb c) {
        return c.b - (c.r + c.g) / 2;
    }

    int Separacao(const Rgb a, const Rgb b, const Modo m) {
        return std::abs(NoEixoQueSobra(Simular(a, m)) - NoEixoQueSobra(Simular(b, m)));
    }
}

TEST_CASE("Daltonismo: desligado, nada muda") {
    const Rgb c{200, 40, 90};
    CHECK(Distancia(Corrigir(c, Modo::Desligado), c) == 0);
    CHECK(Distancia(Simular(c, Modo::Desligado), c) == 0);
}

TEST_CASE("Daltonismo: o cinza, o preto e o branco quase nao mudam") {
    for (const Modo m : {Modo::Deuteranopia, Modo::Protanopia, Modo::Tritanopia}) {
        CAPTURE(static_cast<int>(m));
        for (const int v : {0, 60, 128, 200, 255}) {
            const Rgb cinza{static_cast<unsigned char>(v), static_cast<unsigned char>(v), static_cast<unsigned char>(v)};
            CAPTURE(v);
            CHECK(Distancia(Corrigir(cinza, m), cinza) <= 12);
        }
    }
}

TEST_CASE("Daltonismo: o vermelho e o verde que o olho confunde se separam depois da correcao") {
    // A barra da nota: vermelho de quem reprova, verde de quem passa.
    const Rgb vermelho{220, 60, 50};
    const Rgb verde{60, 200, 70};
    for (const Modo m : {Modo::Deuteranopia, Modo::Protanopia}) {
        CAPTURE(static_cast<int>(m));
        const int antes = Separacao(vermelho, verde, m);
        const int depois = Separacao(Corrigir(vermelho, m), Corrigir(verde, m), m);
        CAPTURE(antes);
        CHECK(depois > antes + 20);
    }
    // Na tritanopia, o azul e o verde-agua (o par que ela confunde) se afastam; e o par da dupla, o
    // azul e o laranja dos alunos, nao se aproxima demais.
    const auto vista = [](const Rgb a, const Rgb b) {
        return Distancia(Simular(a, Modo::Tritanopia), Simular(b, Modo::Tritanopia));
    };
    const auto corrigida = [&](const Rgb a, const Rgb b) {
        return vista(Corrigir(a, Modo::Tritanopia), Corrigir(b, Modo::Tritanopia));
    };
    const Rgb azul{60, 120, 230};
    const Rgb verdeAgua{60, 200, 170};
    CHECK(corrigida(azul, verdeAgua) > vista(azul, verdeAgua));
    const Rgb aluno1{110, 190, 255};
    const Rgb aluno2{255, 160, 80};
    CHECK(corrigida(aluno1, aluno2) * 10 >= vista(aluno1, aluno2) * 9);
}

TEST_CASE("Daltonismo: a lista da a volta, e o nome vai e volta do arquivo") {
    CHECK(Mover(Modo::Desligado, 1) == Modo::Deuteranopia);
    CHECK(Mover(Modo::Tritanopia, 1) == Modo::Desligado);
    CHECK(Mover(Modo::Desligado, -1) == Modo::Tritanopia);
    for (int k = 0; k < kModos; ++k) {
        const Modo m = static_cast<Modo>(k);
        Modo lido = Modo::Tritanopia;
        CHECK(DoArquivo(NomeNoArquivo(m), lido));
        CHECK(lido == m);
    }
    Modo lido = Modo::Protanopia;
    CHECK_FALSE(DoArquivo("verde", lido));
    CHECK(lido == Modo::Protanopia);
}

TEST_CASE("Configuracao: o daltonismo e gravado e lido, e o valor estragado fica desligado") {
    Configuracao::Dados d;
    d.daltonismo = Modo::Protanopia;
    const auto lida = Configuracao::Ler(Configuracao::Serializar(d));
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.daltonismo == Modo::Protanopia);
    CHECK(Configuracao::Ler("").dados.daltonismo == Modo::Desligado);
    const auto estragada = Configuracao::Ler("daltonismo=roxo\n");
    CHECK(estragada.dados.daltonismo == Modo::Desligado);
    CHECK(estragada.problemas.size() == 1u);
}
