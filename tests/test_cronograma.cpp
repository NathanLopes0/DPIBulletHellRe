//
// A fase 1 do Hugo: a proposta, e o cronograma dela.
//

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <set>
#include <string>

#include "../Source/Attacks/Cronograma.h"

using namespace Cronograma;

TEST_CASE("Cronograma: as barras cabem no grafico, nao se encostam na linha, e os prazos tem folga") {
    const float largura = 1200.0f;
    std::set<std::string> graficos;
    for (const auto nivel : Dificuldade::kNiveis) {
        const Degrau d = DegrauDo(nivel);
        for (std::uint32_t semente = 0; semente < 300; ++semente) {
            CAPTURE(semente);
            Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDaProposta);
            const auto tarefas = Gerar(s, largura, d);
            REQUIRE(static_cast<int>(tarefas.size()) == d.tarefas);
            std::set<std::string> nomes;
            std::string assinatura;
            for (size_t i = 0; i < tarefas.size(); ++i) {
                const Tarefa& t = tarefas[i];
                CHECK(t.de >= kMargem);
                CHECK(t.ate <= largura - kMargem);
                CHECK(t.ate - t.de >= kMenor - 1e-3f);
                CHECK(t.ate - t.de <= kMaior + 1e-3f);
                CHECK(t.linha >= 0);
                CHECK(t.linha < kLinhas);
                nomes.insert(t.nome);
                assinatura += std::to_string(t.linha) + ":" + std::to_string(static_cast<int>(t.de)) + " ";
                // EM ORDEM DE PRAZO, e cada queda com o seu tempo.
                if (i > 0) CHECK(t.ate - tarefas[i - 1].ate >= kSeparacao - 1e-3f);
                for (size_t j = 0; j < i; ++j) {
                    const Tarefa& u = tarefas[j];
                    if (u.linha != t.linha) continue;
                    CHECK((t.de >= u.ate + kFolgaNaLinha || u.de >= t.ate + kFolgaNaLinha));
                }
            }
            CHECK(nomes.size() == tarefas.size());
            graficos.insert(assinatura);
        }
    }
    CHECK(graficos.size() > 250u);   // o mesmo numero de barras nos dois modos: as sementes repetem
}

TEST_CASE("Cronograma: o hoje anda pelo grafico, e cada barra cai quando ele passa do fim dela") {
    const float largura = 1200.0f;
    const Degrau d = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(XDoHoje(0.0f, largura, d) == doctest::Approx(kMargem));
    CHECK(XDoHoje(d.varredura, largura, d) == doctest::Approx(largura - kMargem));
    CHECK(XDoHoje(99.0f, largura, d) == doctest::Approx(largura - kMargem));
    for (float x = kMargem; x <= largura - kMargem; x += 37.0f) {
        CHECK(XDoHoje(Quando(x, largura, d), largura, d) == doctest::Approx(x));
    }
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).varredura < d.varredura);

    Tarefa t;
    t.de = 300.0f;
    t.ate = 520.0f;
    t.nome = "Experimentos";
    const auto xs = Paginas(t, d);
    REQUIRE(xs.size() >= 2u);
    CHECK(xs.front() == doctest::Approx(300.0f));
    CHECK(xs.back() == doctest::Approx(520.0f));
    for (size_t i = 1; i < xs.size(); ++i) CHECK(xs[i] - xs[i - 1] <= d.entrePaginas + 1e-3f);

    CHECK(Semana(kMargem, largura) == 1);
    CHECK(Semana(largura - kMargem, largura) == kSemanas);
    CHECK(Placar(t, largura) == "Experimentos\nvenceu na semana " + std::to_string(Semana(520.0f, largura)));
    CHECK(YDaLinha(1) - YDaLinha(0) == doctest::Approx(kEntreLinhas));
}
