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

TEST_CASE("Objetivo: as etapas alternam derrubar e andar, a partir da sorteada") {
    for (const Etapa e : {Etapa::Proposta, Etapa::Desenvolvimento, Etapa::Defesa, Etapa::Correcoes}) {
        CHECK(DeAndar(e) != Tipo::Derrubar);
        for (int primeira = 0; primeira < 2; ++primeira) {
            CHECK(DaVez(e, 0, primeira) != DaVez(e, 1, primeira));
            CHECK(DaVez(e, 0, primeira) == DaVez(e, 2, primeira));
        }
        CHECK(DaVez(e, 0, 0) == Tipo::Derrubar);
        CHECK(DaVez(e, 0, 1) == DeAndar(e));
    }
    // CADA ETAPA ANDA DO SEU JEITO.
    std::set<int> jeitos;
    for (const Etapa e : {Etapa::Proposta, Etapa::Desenvolvimento, Etapa::Defesa, Etapa::Correcoes}) {
        jeitos.insert(static_cast<int>(DeAndar(e)));
    }
    CHECK(jeitos.size() == 4u);
}

TEST_CASE("Objetivo: os objetivos de andar ficam no chao, e a reuniao e a entrega atravessam o campo") {
    const float largura = 1200.0f;
    for (const auto nivel : Dificuldade::kNiveis) {
        for (std::uint32_t semente = 0; semente < 200; ++semente) {
            Variacao::Sorteio s = Variacao::SorteioDa(semente, "andar");
            for (const float xDoAluno : {200.0f, 600.0f, 1000.0f}) {
                const Vector2 r = LugarDaReuniao(s, largura, xDoAluno);
                CHECK(r.x - kRaioDaReuniao >= kMargem - 1.0f);
                CHECK(r.x + kRaioDaReuniao <= largura - kMargem + 1.0f);
                CHECK(r.y >= kChaoDeCima);
                CHECK(r.y <= kChaoDeBaixo);
                CHECK(std::fabs(r.x - xDoAluno) > 60.0f);
            }
            const auto livros = Referencias(s, largura, nivel);
            CHECK(static_cast<int>(livros.size()) == Placas(Tipo::Referencias, nivel, 0));
            for (size_t i = 0; i < livros.size(); ++i) {
                CHECK(livros[i].y >= kChaoDeCima);
                CHECK(livros[i].y <= kChaoDeBaixo);
                for (size_t j = 0; j < i; ++j) CHECK(std::fabs(livros[i].x - livros[j].x) > 2.0f * kRaioDaColeta);
            }
            const Entrega e = Entregar(s, largura);
            CHECK(std::fabs(e.versao.x - e.secretaria.x) > largura * 0.5f);
            CHECK(e.secretaria.y <= kChaoDeBaixo);
            CHECK(e.versao.y < e.secretaria.y);
        }
    }
}

TEST_CASE("Objetivo: raspar e passar perto sem encostar") {
    CHECK_FALSE(Raspou(20.0f, 30.0f));                    // encostou: e acerto
    CHECK(Raspou(31.0f, 30.0f));
    CHECK(Raspou(30.0f + kFolgaDaRaspada, 30.0f));
    CHECK_FALSE(Raspou(31.0f + kFolgaDaRaspada, 30.0f));  // longe demais
    CHECK(Raspadas(Dificuldade::Nivel::Dificil) > Raspadas(Dificuldade::Nivel::Normal));
    CHECK(TempoDeReuniao(Dificuldade::Nivel::Dificil) > TempoDeReuniao(Dificuldade::Nivel::Normal));
}

TEST_CASE("Objetivo: as placas do escudo caem com o progresso, e so a ultima abre") {
    for (const int placas : {1, 2, 3, 5, 8}) {
        CHECK(PlacasDePe(placas, 0.0f) == placas);
        CHECK(PlacasDePe(placas, 1.0f) == 0);
        CHECK(PlacasDePe(placas, 0.999f) >= 1);
        int antes = placas;
        for (int k = 1; k <= placas; ++k) {
            const int agora = PlacasDePe(placas, static_cast<float>(k) / static_cast<float>(placas));
            CHECK(agora == placas - k);
            CHECK(agora <= antes);
            antes = agora;
        }
    }
    CHECK(Placas(Tipo::Derrubar, Dificuldade::Nivel::Normal, 3) == 3);
    CHECK(Placas(Tipo::Entrega, Dificuldade::Nivel::Normal, 0) == 2);
    // A RASPADA PAGA MENOS, e a reuniao paga de uma vez.
    CHECK(PorPasso(Tipo::Arguicao, Dificuldade::Nivel::Normal) < PorPasso(Tipo::Derrubar, Dificuldade::Nivel::Normal));
    CHECK(PorPasso(Tipo::Reuniao, Dificuldade::Nivel::Normal) > PorPasso(Tipo::Derrubar, Dificuldade::Nivel::Normal));
    CHECK(Pede(Tipo::Referencias, 1) == "falta 1 referência");
    CHECK(Pede(Tipo::Entrega, 1) != Pede(Tipo::Entrega, 0));
}
