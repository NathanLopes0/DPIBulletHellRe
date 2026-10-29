//
// A fase 1 do Lucas: uma pagina sendo renderizada no navegador.
//

#include "../../doctest.h"

#include <cstdint>
#include <set>
#include <string>

#include "../../../Source/Attacks/Lucas/Pagina.h"

using namespace Pagina;

namespace {
    bool Dentro(const Retangulo& a, const Retangulo& b) {
        return a.x >= b.x && a.y >= b.y && a.x + a.w <= b.x + b.w && a.y + a.h <= b.y + b.h;
    }
}

TEST_CASE("Pagina: o layout tem o header no alto e o nav, o main e o aside lado a lado, dentro da janela") {
    const auto l = Layout(1200.0f);
    const Retangulo janela{0.0f, 0.0f, 1200.0f, 800.0f};
    for (const Retangulo& r : l) CHECK(Dentro(r, janela));
    const Retangulo& header = l[static_cast<size_t>(Regiao::Header)];
    const Retangulo& nav = l[static_cast<size_t>(Regiao::Nav)];
    const Retangulo& main = l[static_cast<size_t>(Regiao::Main)];
    const Retangulo& aside = l[static_cast<size_t>(Regiao::Aside)];
    CHECK(header.y + header.h <= nav.y);
    CHECK(nav.x + nav.w <= main.x);
    CHECK(main.x + main.w <= aside.x);
    CHECK(main.w == doctest::Approx(2.0f * nav.w));
    // O CORPO DEIXA O FUNDO LIVRE para o aluno.
    CHECK(main.y + main.h <= 560.0f);
}

TEST_CASE("Pagina: cada pagina poe os elementos no fluxo das regioes, em ordem de documento") {
    const auto l = Layout(1200.0f);
    std::set<std::string> placares;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDaPagina);
        const auto pagina = Gerar(s, 1200.0f);
        CAPTURE(semente);
        REQUIRE(pagina.size() >= 6u);
        CHECK(pagina.size() <= 8u);
        std::set<Tipo> tipos;
        for (size_t k = 0; k < pagina.size(); ++k) {
            const Elemento& e = pagina[k];
            tipos.insert(e.tipo);
            CHECK(Dentro(e.caixa, l[static_cast<size_t>(e.regiao)]));
            if (e.tipo == Tipo::Link) CHECK(e.ancora != e.regiao);
            if (k == 0) continue;
            const Elemento& antes = pagina[k - 1];
            // A ORDEM DO DOCUMENTO: nav, main, aside.
            CHECK(static_cast<int>(antes.regiao) <= static_cast<int>(e.regiao));
            // O FLUXO: na mesma regiao, um embaixo do outro, sem encostar.
            if (antes.regiao == e.regiao) CHECK(e.caixa.y >= antes.caixa.y + antes.caixa.h);
        }
        CHECK(tipos.size() >= 3u);
        placares.insert(Placar(pagina.front()));
    }
    CHECK(placares.size() > 5u);
}

TEST_CASE("Pagina: o placar e a tag com o atributo e o caminho, e a tabela tem 3 x 4 celulas dentro dela") {
    Elemento img;
    img.tipo = Tipo::Imagem;
    img.regiao = Regiao::Main;
    img.atributo = "src=\"gato.png\"";
    CHECK(Placar(img) == "<img src=\"gato.png\">\nbody > main");
    Elemento tabela;
    tabela.tipo = Tipo::Tabela;
    tabela.regiao = Regiao::Aside;
    CHECK(Placar(tabela) == "<table>\nbody > aside");

    const Retangulo caixa{100.0f, 200.0f, 200.0f, 126.0f};
    const auto c = Celulas(caixa);
    REQUIRE(c.size() == static_cast<size_t>(kLinhasDaTabela));
    for (const auto& linha : c) {
        REQUIRE(linha.size() == static_cast<size_t>(kColunasDaTabela));
        for (const auto& p : linha) {
            CHECK(p[0] > caixa.x);
            CHECK(p[0] < caixa.x + caixa.w);
            CHECK(p[1] > caixa.y);
            CHECK(p[1] < caixa.y + caixa.h);
        }
    }
    CHECK(c[1][0][1] > c[0][0][1]);   // linha por linha, de cima para baixo
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).porElemento < DegrauDo(Dificuldade::Nivel::Normal).porElemento);
}
