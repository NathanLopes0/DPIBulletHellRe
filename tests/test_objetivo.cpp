//
// O escudo do Hugo e o objetivo de cada etapa do TCC.
//

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <set>
#include <string>

#include "../Source/Attacks/Objetivo.h"
#include "../Source/Attacks/Tcc.h"

using namespace Objetivo;

TEST_CASE("Objetivo: cada etapa tem os alvos dela, dentro da faixa e longe uns dos outros") {
    CHECK(DaFase("StateOne") == Etapa::Proposta);
    CHECK(DaFase("StateTwo") == Etapa::Desenvolvimento);
    CHECK(DaFase("StateThree") == Etapa::Defesa);
    CHECK(DaFase("StateFinal") == Etapa::Correcoes);
    CHECK(DaFase("x") == Etapa::Nenhuma);

    const float largura = 1200.0f;
    for (const auto nivel : Dificuldade::kNiveis) {
        for (std::uint32_t semente = 0; semente < 100; ++semente) {
            Variacao::Sorteio s = Variacao::SorteioDa(semente, "objetivo");
            for (const Etapa e : {Etapa::Proposta, Etapa::Desenvolvimento, Etapa::Defesa, Etapa::Correcoes}) {
                const auto alvos = Gerar(e, s, largura, nivel, static_cast<int>(semente % 3));
                REQUIRE(alvos.size() >= 2u);
                int certos = 0;
                for (size_t i = 0; i < alvos.size(); ++i) {
                    const Alvo& a = alvos[i];
                    CHECK(a.lugar.x >= kMargem);
                    CHECK(a.lugar.x <= largura - kMargem);
                    CHECK(a.lugar.y >= kYDeCima - 60.0f);
                    CHECK(a.lugar.y <= kYDeBaixo);
                    if (a.certo) ++certos;
                    for (size_t j = 0; j < i; ++j) CHECK(std::fabs(a.lugar.x - alvos[j].lugar.x) > 80.0f);
                    // BALANCANDO, o alvo nao sai de perto do lugar dele.
                    for (float t = 0.0f; t < 10.0f; t += 0.37f) {
                        CHECK((Balanco(e, a.lugar, static_cast<int>(i), t) - a.lugar).Length() < 80.0f);
                    }
                }
                // NA DEFESA so uma resposta e certa - e e a do placar.
                if (e == Etapa::Defesa) {
                    CHECK(certos == 1);
                    CHECK(alvos[semente % 3].certo);
                } else {
                    CHECK(certos == static_cast<int>(alvos.size()));
                }
            }
        }
    }
    Variacao::Sorteio s = Variacao::SorteioDa(1u, "objetivo");
    CHECK(Gerar(Etapa::Desenvolvimento, s, largura, Dificuldade::Nivel::Dificil).size() >
          Gerar(Etapa::Desenvolvimento, s, largura, Dificuldade::Nivel::Normal).size());
}

TEST_CASE("Objetivo: a banca pergunta, e a resposta certa e embaralhada entre A, B e C") {
    std::set<int> posicoesDaCerta;
    for (const Golpe::Materia m : Tcc::kMaterias) {
        for (std::uint32_t semente = 0; semente < 20; ++semente) {
            Variacao::Sorteio s = Variacao::SorteioDa(semente, "defesa");
            const Questao q = Perguntar(m, s);
            CHECK(q.certa >= 0);
            CHECK(q.certa < 3);
            posicoesDaCerta.insert(q.certa);
            std::set<std::string> distintas(q.respostas.begin(), q.respostas.end());
            CHECK(distintas.size() == 3u);
            // A PERGUNTA E A DA BANCA (Tcc::Pergunta), e a certa e a mesma em
            // toda ordem.
            CHECK(q.pergunta == Tcc::Pergunta(m));
            Variacao::Sorteio outro = Variacao::SorteioDa(semente + 1000, "defesa");
            const Questao r = Perguntar(m, outro);
            CHECK(r.respostas[static_cast<size_t>(r.certa)] == q.respostas[static_cast<size_t>(q.certa)]);
        }
    }
    CHECK(posicoesDaCerta.size() == 3u);

}

TEST_CASE("Objetivo: a janela e mais curta no Dificil, e mais longa nas correcoes") {
    CHECK(Janela(Etapa::Proposta, Dificuldade::Nivel::Dificil) < Janela(Etapa::Proposta, Dificuldade::Nivel::Normal));
    CHECK(Janela(Etapa::Correcoes, Dificuldade::Nivel::Normal) > Janela(Etapa::Defesa, Dificuldade::Nivel::Normal));
    CHECK(Falta(Etapa::Proposta, 1) == "falta 1 assinatura");
    CHECK(Falta(Etapa::Desenvolvimento, 3) == "faltam 3 bugs");
    CHECK(Falta(Etapa::Correcoes, 2) == "faltam 2 correções");
}
