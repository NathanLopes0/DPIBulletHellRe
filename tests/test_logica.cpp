//
// Os bits e as portas logicas do Ricardo.
//

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <vector>
#include "../Source/Attacks/Engine/Fase.h"

#include "../Source/Attacks/Shared/Logica.h"

using Logica::Operacao;

TEST_CASE("Logica: as tabelas-verdade das quatro portas") {
    // a, b -> AND, OR, XOR
    const bool tabela[4][5] = {
        {false, false, false, false, false},
        {false, true,  false, true,  true },
        {true,  false, false, true,  true },
        {true,  true,  true,  true,  false},
    };
    for (const auto& linha : tabela) {
        CAPTURE(linha[0]);
        CAPTURE(linha[1]);
        CHECK(Logica::Saida(Operacao::E, linha[0], linha[1]) == linha[2]);
        CHECK(Logica::Saida(Operacao::Ou, linha[0], linha[1]) == linha[3]);
        CHECK(Logica::Saida(Operacao::OuExclusivo, linha[0], linha[1]) == linha[4]);
        CHECK(Logica::Saida(Operacao::Nao, linha[0], linha[1]) == !linha[0]);   // o NOT ignora b
    }
}

TEST_CASE("Logica: o placar e a porta - le o nome e a entrada B, e recusa o resto") {
    const auto e = Logica::LerPorta("AND 0000 1111 1111 0000", 16);
    REQUIRE(e);
    CHECK(e->operacao == Operacao::E);
    CHECK(e->b == 0x0FF0u);

    const auto ou = Logica::LerPorta("OR 1100000000000011", 16);   // sem os espacos tambem
    REQUIRE(ou);
    CHECK(ou->operacao == Operacao::Ou);
    CHECK(ou->b == 0xC003u);

    const auto x = Logica::LerPorta("  XOR 1111 1111 0000 0000", 16);
    REQUIRE(x);
    CHECK(x->operacao == Operacao::OuExclusivo);
    CHECK(x->b == 0xFF00u);

    const auto nao = Logica::LerPorta("NOT", 16);
    REQUIRE(nao);
    CHECK(nao->operacao == Operacao::Nao);

    CHECK_FALSE(Logica::LerPorta("AND 0000 1111", 16));              // B curto
    CHECK_FALSE(Logica::LerPorta("AND 0000 1111 1111 0000 1", 16));  // B longo
    CHECK_FALSE(Logica::LerPorta("AND 0000 1111 1121 0000", 16));    // um 2 no B
    CHECK_FALSE(Logica::LerPorta("NOT 1111 0000 1111 0000", 16));    // o NOT nao tem B
    CHECK_FALSE(Logica::LerPorta("NAND 0000 1111 1111 0000", 16));   // porta que nao ha
    CHECK_FALSE(Logica::LerPorta("and 0000 1111 1111 0000", 16));    // o placar escreve em maiusculas
    CHECK_FALSE(Logica::LerPorta("", 16));
    CHECK_FALSE(Logica::LerPorta("portas logicas", 16));
}

TEST_CASE("Logica: a coluna 0 e o bit mais alto, a esquerda, como se escreve") {
    const std::uint32_t p = 0b1000000000000001u;
    CHECK(Logica::Bit(p, 0, 16));
    CHECK_FALSE(Logica::Bit(p, 1, 16));
    CHECK(Logica::Bit(p, 15, 16));
    CHECK_FALSE(Logica::Bit(p, 16, 16));    // fora da palavra
    CHECK_FALSE(Logica::Bit(p, -1, 16));
    CHECK(Logica::Texto(0xB2C0u, 16) == "1011 0010 1100 0000");
    CHECK(Logica::Uns(0xB2C0u, 16) == 6);
}

TEST_CASE("Logica: a fileira de portas aplica a porta de cada coluna ao bit dela") {
    const Logica::Porta e{Operacao::E, 0x0FF0u};
    CHECK(Logica::Aplicar(e, 0xB2C3u, 16) == (0xB2C3u & 0x0FF0u));
    const Logica::Porta ou{Operacao::Ou, 0xC003u};
    CHECK(Logica::Aplicar(ou, 0x1230u, 16) == (0x1230u | 0xC003u));
    const Logica::Porta x{Operacao::OuExclusivo, 0xFF00u};
    CHECK(Logica::Aplicar(x, 0x1234u, 16) == (0x1234u ^ 0xFF00u));
    const Logica::Porta nao{Operacao::Nao, 0u};
    CHECK(Logica::Aplicar(nao, 0x1234u, 16) == (~0x1234u & 0xFFFFu));   // so os 16 bits
}

TEST_CASE("Logica: a animacao da porta e o nome e o B da coluna") {
    const Logica::Porta e{Operacao::E, 0x8000u};
    CHECK(Logica::AnimacaoDaPorta(e, 0, 16) == "AND1");
    CHECK(Logica::AnimacaoDaPorta(e, 1, 16) == "AND0");
    const Logica::Porta nao{Operacao::Nao, 0xFFFFu};
    CHECK(Logica::AnimacaoDaPorta(nao, 0, 16) == "NOT");
}

TEST_CASE("Logica: as colunas dividem a tela em partes iguais") {
    CHECK(Logica::Coluna(0.0f, 1200.0f, 16) == 0);
    CHECK(Logica::Coluna(74.9f, 1200.0f, 16) == 0);
    CHECK(Logica::Coluna(75.0f, 1200.0f, 16) == 1);
    CHECK(Logica::Coluna(1199.0f, 1200.0f, 16) == 15);
    CHECK(Logica::Coluna(-30.0f, 1200.0f, 16) == 0);      // fora da tela: a da borda
    CHECK(Logica::Coluna(1500.0f, 1200.0f, 16) == 15);
    CHECK(Logica::CentroDaColuna(0, 1200.0f, 16) == doctest::Approx(37.5f));
    CHECK(Logica::CentroDaColuna(15, 1200.0f, 16) == doctest::Approx(1162.5f));
    for (int c = 0; c < 16; ++c) CHECK(Logica::Coluna(Logica::CentroDaColuna(c, 1200.0f, 16), 1200.0f, 16) == c);
}

TEST_CASE("Logica: uma palavra justa e uma parede com porta antes e depois, e a porta muda algo") {
    const Logica::Porta nao{Operacao::Nao, 0u};
    CHECK(Logica::Justa(nao, 0xF0F0u, 16));
    CHECK_FALSE(Logica::Justa(nao, 0xFFF0u, 16));   // depois do NOT, so 4 acesos
    CHECK_FALSE(Logica::Justa(nao, 0xFFFEu, 16));   // antes, um buraco so

    const Logica::Porta identidade{Operacao::Ou, 0u};
    CHECK_FALSE(Logica::Justa(identidade, 0xF0F0u, 16));   // a porta nao muda nada
}

TEST_CASE("Logica: o sorteio sempre da uma palavra justa quando a porta admite alguma") {
    const Logica::Porta portas[] = {
        {Operacao::E, 0x0FF0u}, {Operacao::Ou, 0xC003u},
        {Operacao::Nao, 0u}, {Operacao::OuExclusivo, 0xFF00u},
    };
    for (const auto& p : portas) {
        CAPTURE(Logica::NomeDaOperacao(p.operacao));
        for (std::uint32_t s = 0; s < 70000u; s += 997u) {
            CHECK(Logica::Justa(p, Logica::Sortear(p, 16, s), 16));
        }
    }

    // UMA PORTA SEM PALAVRA JUSTA - o AND com B quase todo zero nunca deixa
    // cinco bits acesos - devolve o sorteio, e nao fica rodando para sempre.
    const Logica::Porta impossivel{Operacao::E, 0x000Fu};
    CHECK(Logica::Sortear(impossivel, 16, 0x1234u) == 0x1234u);
}

TEST_CASE("Logica: o sorteio nao e sempre a mesma palavra") {
    const Logica::Porta nao{Operacao::Nao, 0u};
    int diferentes = 0;
    std::uint32_t antes = Logica::Sortear(nao, 16, 0u);
    for (std::uint32_t s = 1; s < 50u; ++s) {
        const std::uint32_t agora = Logica::Sortear(nao, 16, s * 1311u);
        if (agora != antes) ++diferentes;
        antes = agora;
    }
    CHECK(diferentes > 40);
}

TEST_CASE("Logica: o deslocamento anda uma coluna por passo, vai ate a ponta e volta") {
    // de 2 a 5, comecando no 3: 3 4 5 4 3 2 3 4 ...
    const int esperado[] = {3, 4, 5, 4, 3, 2, 3, 4, 5};
    for (int k = 0; k < 9; ++k) {
        CAPTURE(k);
        CHECK(Logica::ColunaDoDeslocamento(0.5f * static_cast<float>(k) + 0.1f, 0.5f, 2, 5, 3) == esperado[k]);
    }
    CHECK(Logica::ColunaDoDeslocamento(0.0f, 0.5f, 2, 5, 9) == 5);   // um inicio fora: a ponta
    CHECK(Logica::ColunaDoDeslocamento(3.0f, 0.5f, 4, 4, 4) == 4);   // uma coluna so: parado
    CHECK(Logica::ColunaDoDeslocamento(3.0f, 0.0f, 2, 5, 3) == 3);   // sem passo, sem andar
}

TEST_CASE("Logica: as portas da luta - as quatro operacoes, um B que admite parede com porta, e o placar que volta a porta") {
    std::set<std::vector<int>> ordens;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        CAPTURE(semente);
        Variacao::Sorteio s(semente);
        const Logica::Plano plano = Logica::Sortear(s);
        std::vector<int> ops;
        for (const Logica::Porta& p : plano.portas) {
            ops.push_back(static_cast<int>(p.operacao));
            // O PLACAR E A PORTA: o texto dela le de volta a mesma porta.
            const auto lida = Logica::LerPorta(Logica::Anuncio(p), Logica::kBits);
            REQUIRE(lida.has_value());
            CHECK(lida->operacao == p.operacao);
            if (p.operacao == Logica::Operacao::Nao) continue;
            CHECK(lida->b == p.b);
            CHECK(Logica::Uns(p.b, Logica::kBits) >= 4);
            CHECK(Logica::Uns(p.b, Logica::kBits) <= 12);
            // E ELA ADMITE PALAVRA JUSTA, de qualquer sorteio.
            for (std::uint32_t k = 0; k < 8; ++k) {
                CHECK(Logica::Justa(p, Logica::Sortear(p, Logica::kBits, semente * 7919u + k * 104729u), Logica::kBits));
            }
        }
        std::vector<int> ordenadas = ops;
        std::sort(ordenadas.begin(), ordenadas.end());
        CHECK(ordenadas == std::vector<int>{0, 1, 2, 3});
        ordens.insert(ops);
    }
    CHECK(ordens.size() == 24u);
    CHECK(Logica::Anuncio({Logica::Operacao::E, 0x0FF0u}) == "AND 0000 1111 1111 0000");
    CHECK(Logica::Anuncio({Logica::Operacao::Nao, 0u}) == "NOT");
}
