// A ordem das melhores notas. Camada pura.
//
// A curva da nota foi feita inteira para que a nota ordenasse um ranking de
// verdade. Estes testes protegem o outro lado disso: que a ordenacao conte a
// mesma historia que a nota conta.

#include "doctest.h"

#include <string>
#include <vector>

#include "../Source/Ranking.h"

namespace {

    /// Uma turma: pares de matricula e nota na materia 0.
    std::vector<Exportacao::FichaDeAluno> Turma(
            const std::vector<std::pair<std::string, float>>& notas) {

        std::vector<Exportacao::FichaDeAluno> fichas;
        for (const auto& [matricula, nota] : notas) {
            Exportacao::FichaDeAluno f;
            f.matricula = matricula;
            if (nota > 0.0f) f.progresso.RegistrarNota(0, nota);
            fichas.push_back(f);
        }
        return fichas;
    }
}

TEST_CASE("Ranking: a maior nota vem primeiro") {

    const auto r = Ranking::DaMateria(Turma({{"1", 71.0f}, {"2", 100.0f}, {"3", 94.5f}}), 0);

    REQUIRE(r.size() == 3);
    CHECK(r[0].matricula == "2");
    CHECK(r[1].matricula == "3");
    CHECK(r[2].matricula == "1");
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 2);
    CHECK(r[2].posicao == 3);
}

TEST_CASE("Ranking: quem nunca jogou a materia nao aparece") {

    // UMA LINHA POR ALUNO QUE NUNCA ABRIU A FASE encheria a tela de gente que
    // nao disputou, e empurraria para fora quem disputou.
    const auto r = Ranking::DaMateria(Turma({{"1", 80.0f}, {"2", 0.0f}, {"3", 60.0f}}), 0);

    REQUIRE(r.size() == 2);
    for (const auto& l : r) { CAPTURE(l.matricula); CHECK(l.matricula != "2"); }
}

TEST_CASE("Ranking: empate divide a posicao, e a seguinte pula") {

    // E como se le um placar. Desempatar pela matricula faria o aluno mais
    // antigo ganhar de graca de quem tirou a mesma nota.
    const auto r = Ranking::DaMateria(
        Turma({{"10", 90.0f}, {"20", 90.0f}, {"30", 80.0f}}), 0);

    REQUIRE(r.size() == 3);
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 1);
    CHECK(r[2].posicao == 3);   // pula a 2
}

TEST_CASE("Ranking: a ordem entre empatados e sempre a mesma") {

    // A listagem de fichas vem do sistema de arquivos, que nao garante ordem.
    // Sem um criterio estavel, a tela trocaria a ordem dos empatados a cada
    // abertura e pareceria defeito.
    const auto a = Ranking::DaMateria(Turma({{"10", 90.0f}, {"20", 90.0f}}), 0);
    const auto b = Ranking::DaMateria(Turma({{"20", 90.0f}, {"10", 90.0f}}), 0);

    REQUIRE(a.size() == 2);
    REQUIRE(b.size() == 2);
    CHECK(a[0].matricula == b[0].matricula);
    CHECK(a[1].matricula == b[1].matricula);
}

TEST_CASE("Ranking: o limite corta o fim, nao o comeco") {

    const auto r = Ranking::DaMateria(
        Turma({{"1", 10.0f}, {"2", 100.0f}, {"3", 50.0f}, {"4", 90.0f}}), 0, 2);

    REQUIRE(r.size() == 2);
    CHECK(r[0].matricula == "2");
    CHECK(r[1].matricula == "4");
}

TEST_CASE("Ranking: limite zero devolve a lista inteira") {

    CHECK(Ranking::DaMateria(Turma({{"1", 10.0f}, {"2", 20.0f}}), 0, 0).size() == 2);
}

TEST_CASE("Ranking: limite maior que a turma nao inventa linha") {

    CHECK(Ranking::DaMateria(Turma({{"1", 10.0f}}), 0, 50).size() == 1);
}

TEST_CASE("Ranking: turma vazia devolve lista vazia") {

    CHECK(Ranking::DaMateria({}, 0).empty());
    CHECK(Ranking::PosicaoDe({}, 0, "89384") == 0);
}

TEST_CASE("Ranking: a posicao de quem nao esta no topo e a da lista completa") {

    // O RANKING TEM DE FALAR COM QUEM NAO ESTA BEM. Mostrando so os primeiros, ele
    // so conversa com quem ja esta no topo - que e justamente quem menos precisa.
    const auto turma = Turma({{"1", 100.0f}, {"2", 90.0f}, {"3", 80.0f},
                              {"4", 70.0f},  {"5", 60.0f}});

    // O topo mostra tres, mas a posicao do quinto continua sendo 5.
    CHECK(Ranking::DaMateria(turma, 0, 3).size() == 3);
    CHECK(Ranking::PosicaoDe(turma, 0, "5") == 5);
    CHECK(Ranking::PosicaoDe(turma, 0, "1") == 1);
}

TEST_CASE("Ranking: posicao de quem nunca jogou a materia e zero") {

    const auto turma = Turma({{"1", 100.0f}, {"2", 0.0f}});
    CHECK(Ranking::PosicaoDe(turma, 0, "2") == 0);
    CHECK(Ranking::PosicaoDe(turma, 0, "999") == 0);
}

TEST_CASE("Ranking: cada materia tem a sua ordem") {

    // As notas sao por materia, e misturar duas faria o ranking de uma mostrar a
    // outra - do tipo de erro que so aparece quando alguem joga a segunda fase.
    std::vector<Exportacao::FichaDeAluno> fichas;

    Exportacao::FichaDeAluno a; a.matricula = "1";
    a.progresso.RegistrarNota(0, 100.0f);
    a.progresso.RegistrarNota(1, 50.0f);

    Exportacao::FichaDeAluno b; b.matricula = "2";
    b.progresso.RegistrarNota(0, 60.0f);
    b.progresso.RegistrarNota(1, 95.0f);

    fichas.push_back(a);
    fichas.push_back(b);

    CHECK(Ranking::DaMateria(fichas, 0)[0].matricula == "1");
    CHECK(Ranking::DaMateria(fichas, 1)[0].matricula == "2");
}

TEST_CASE("Ranking: o teto por dano fica abaixo da nota cheia, tambem aqui") {

    // 99,99 e 100 sao resultados diferentes, e o ranking e o lugar onde essa
    // diferenca finalmente aparece para o jogador.
    const auto r = Ranking::DaMateria(Turma({{"1", 99.99f}, {"2", 100.0f}}), 0);

    REQUIRE(r.size() == 2);
    CHECK(r[0].matricula == "2");
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 2);
}
