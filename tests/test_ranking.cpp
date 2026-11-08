// A ordem das melhores notas.

#include "doctest.h"
#include "../Source/Regras/Dificuldade.h"

#include <string>
#include <vector>

#include "../Source/Aluno/Ranking.h"

namespace {

    /// Uma turma: pares de matricula e nota na materia 0.
    std::vector<Exportacao::FichaDeAluno> Turma(
            const std::vector<std::pair<std::string, float>>& notas) {

        std::vector<Exportacao::FichaDeAluno> fichas;
        for (const auto& [matricula, nota] : notas) {
            Exportacao::FichaDeAluno f;
            f.matricula = matricula;
            if (nota > 0.0f) f.progresso.RegistrarNota(0, nota, Dificuldade::Nivel::Normal);
            fichas.push_back(f);
        }
        return fichas;
    }
}

TEST_CASE("Ranking: a maior nota vem primeiro") {

    const auto r = Ranking::DaMateria(Turma({{"1", 71.0f}, {"2", 100.0f}, {"3", 94.5f}}), 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 3);
    CHECK(r[0].matricula == "2");
    CHECK(r[1].matricula == "3");
    CHECK(r[2].matricula == "1");
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 2);
    CHECK(r[2].posicao == 3);
}

TEST_CASE("Ranking: quem nunca jogou a materia nao aparece") {

    const auto r = Ranking::DaMateria(Turma({{"1", 80.0f}, {"2", 0.0f}, {"3", 60.0f}}), 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 2);
    for (const auto& l : r) { CAPTURE(l.matricula); CHECK(l.matricula != "2"); }
}

TEST_CASE("Ranking: empate divide a posicao, e a seguinte pula") {

    // E como se le um placar.
    const auto r = Ranking::DaMateria(
        Turma({{"10", 90.0f}, {"20", 90.0f}, {"30", 80.0f}}), 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 3);
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 1);
    CHECK(r[2].posicao == 3);   // pula a 2
}

TEST_CASE("Ranking: a ordem entre empatados e sempre a mesma") {

    // A listagem de fichas vem do sistema de arquivos, que nao garante ordem.
    const auto a = Ranking::DaMateria(Turma({{"10", 90.0f}, {"20", 90.0f}}), 0, Dificuldade::Nivel::Normal);
    const auto b = Ranking::DaMateria(Turma({{"20", 90.0f}, {"10", 90.0f}}), 0, Dificuldade::Nivel::Normal);

    REQUIRE(a.size() == 2);
    REQUIRE(b.size() == 2);
    CHECK(a[0].matricula == b[0].matricula);
    CHECK(a[1].matricula == b[1].matricula);
}

TEST_CASE("Ranking: o limite corta o fim, nao o comeco") {

    const auto r = Ranking::DaMateria(
        Turma({{"1", 10.0f}, {"2", 100.0f}, {"3", 50.0f}, {"4", 90.0f}}), 0, Dificuldade::Nivel::Normal, 2);

    REQUIRE(r.size() == 2);
    CHECK(r[0].matricula == "2");
    CHECK(r[1].matricula == "4");
}

TEST_CASE("Ranking: limite zero devolve a lista inteira") {

    CHECK(Ranking::DaMateria(Turma({{"1", 10.0f}, {"2", 20.0f}}), 0, Dificuldade::Nivel::Normal, 0).size() == 2);
}

TEST_CASE("Ranking: limite maior que a turma nao inventa linha") {

    CHECK(Ranking::DaMateria(Turma({{"1", 10.0f}}), 0, Dificuldade::Nivel::Normal, 50).size() == 1);
}

TEST_CASE("Ranking: turma vazia devolve lista vazia") {

    CHECK(Ranking::DaMateria({}, 0, Dificuldade::Nivel::Normal).empty());
    CHECK(Ranking::PosicaoDe({}, 0, Dificuldade::Nivel::Normal, "89384") == 0);
}

TEST_CASE("Ranking: a posicao de quem nao esta no topo e a da lista completa") {

    // O ranking tem de falar com quem nao esta bem.
    const auto turma = Turma({{"1", 100.0f}, {"2", 90.0f}, {"3", 80.0f},
                              {"4", 70.0f},  {"5", 60.0f}});

    // O topo mostra tres, mas a posicao do quinto continua sendo 5.
    CHECK(Ranking::DaMateria(turma, 0, Dificuldade::Nivel::Normal, 3).size() == 3);
    CHECK(Ranking::PosicaoDe(turma, 0, Dificuldade::Nivel::Normal, "5") == 5);
    CHECK(Ranking::PosicaoDe(turma, 0, Dificuldade::Nivel::Normal, "1") == 1);
}

TEST_CASE("Ranking: posicao de quem nunca jogou a materia e zero") {

    const auto turma = Turma({{"1", 100.0f}, {"2", 0.0f}});
    CHECK(Ranking::PosicaoDe(turma, 0, Dificuldade::Nivel::Normal, "2") == 0);
    CHECK(Ranking::PosicaoDe(turma, 0, Dificuldade::Nivel::Normal, "999") == 0);
}

TEST_CASE("Ranking: cada materia tem a sua ordem") {

    // As notas sao por materia, e misturar duas faria o ranking de uma mostrar a outra.
    std::vector<Exportacao::FichaDeAluno> fichas;

    Exportacao::FichaDeAluno a; a.matricula = "1";
    a.progresso.RegistrarNota(0, 100.0f, Dificuldade::Nivel::Normal);
    a.progresso.RegistrarNota(1, 50.0f, Dificuldade::Nivel::Normal);

    Exportacao::FichaDeAluno b; b.matricula = "2";
    b.progresso.RegistrarNota(0, 60.0f, Dificuldade::Nivel::Normal);
    b.progresso.RegistrarNota(1, 95.0f, Dificuldade::Nivel::Normal);

    fichas.push_back(a);
    fichas.push_back(b);

    CHECK(Ranking::DaMateria(fichas, 0, Dificuldade::Nivel::Normal)[0].matricula == "1");
    CHECK(Ranking::DaMateria(fichas, 1, Dificuldade::Nivel::Normal)[0].matricula == "2");
}

TEST_CASE("Ranking: o teto por dano fica abaixo da nota cheia, tambem aqui") {

    const auto r = Ranking::DaMateria(Turma({{"1", 99.99f}, {"2", 100.0f}}), 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 2);
    CHECK(r[0].matricula == "2");
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 2);
}

// ---- O ranking geral ----

namespace {

    /// Um aluno com notas em varias materias: {materia, nota}.
    Exportacao::FichaDeAluno Aluno(const std::string& matricula,
                                   const std::vector<std::pair<int, float>>& notas) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        for (const auto& [materia, nota] : notas) f.progresso.RegistrarNota(materia, nota, Dificuldade::Nivel::Normal);
        return f;
    }
}

TEST_CASE("Ranking geral: a media divide pelo CURSO, nao pelas materias jogadas") {

    // A decisao que define o ranking inteiro.
    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("preguicoso", {{0, 100.0f}}),
        Aluno("aplicado",   {{0, 95.0f}, {1, 95.0f}, {2, 95.0f}, {3, 95.0f}}),
    };

    const auto r = Ranking::Geral(turma, 11, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 2);
    CHECK(r[0].matricula == "aplicado");
    CHECK(r[1].matricula == "preguicoso");

    // 4 x 95 / 11 = 34,55 contra 100 / 11 = 9,09.
    CHECK(r[0].nota == doctest::Approx(380.0f / 11.0f));
    CHECK(r[1].nota == doctest::Approx(100.0f / 11.0f));
}

TEST_CASE("Ranking geral: materia nao jogada vale zero, e nao e ignorada") {

    const std::vector<Exportacao::FichaDeAluno> turma = { Aluno("1", {{0, 100.0f}, {1, 100.0f}}) };

    CHECK(Ranking::Geral(turma, 2, Dificuldade::Nivel::Normal)[0].nota  == doctest::Approx(100.0f));
    CHECK(Ranking::Geral(turma, 4, Dificuldade::Nivel::Normal)[0].nota  == doctest::Approx(50.0f));
    CHECK(Ranking::Geral(turma, 10, Dificuldade::Nivel::Normal)[0].nota == doctest::Approx(20.0f));
}

TEST_CASE("Ranking geral: avancar no curso sempre sobe a media") {

    // A propriedade que faz o numero medir avanco.
    auto so3 = Aluno("1", {{0, 90.0f}, {1, 90.0f}, {2, 90.0f}});
    auto com4 = so3;
    com4.progresso.RegistrarNota(3, 10.0f, Dificuldade::Nivel::Normal);   // Uma materia ruim a mais.

    const float antes  = Ranking::Geral({so3}, 11, Dificuldade::Nivel::Normal)[0].nota;
    const float depois = Ranking::Geral({com4}, 11, Dificuldade::Nivel::Normal)[0].nota;

    CAPTURE(antes);
    CAPTURE(depois);
    CHECK(depois > antes);
}

TEST_CASE("Ranking geral: quem nao jogou nada nao entra") {

    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("jogou", {{0, 50.0f}}),
        Aluno("nao", {}),
    };

    const auto r = Ranking::Geral(turma, 11, Dificuldade::Nivel::Normal);
    REQUIRE(r.size() == 1);
    CHECK(r[0].matricula == "jogou");
    CHECK(Ranking::PosicaoNoGeral(turma, 11, Dificuldade::Nivel::Normal, "nao") == 0);
}

TEST_CASE("Ranking geral: empate divide a posicao, como na materia") {

    // As duas listas passam pela mesma ordenacao, entao a regra de empate tem de ser a mesma.
    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("10", {{0, 80.0f}}), Aluno("20", {{1, 80.0f}}), Aluno("30", {{0, 40.0f}}),
    };

    const auto r = Ranking::Geral(turma, 11, Dificuldade::Nivel::Normal);
    REQUIRE(r.size() == 3);
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 1);
    CHECK(r[2].posicao == 3);
}

TEST_CASE("Ranking geral: a posicao vem da lista completa, nao do topo exibido") {

    std::vector<Exportacao::FichaDeAluno> turma;
    for (int i = 0; i < 12; ++i) {
        turma.push_back(Aluno(std::to_string(100 + i), {{0, 100.0f - static_cast<float>(i)}}));
    }

    CHECK(Ranking::Geral(turma, 11, Dificuldade::Nivel::Normal, 5).size() == 5);
    CHECK(Ranking::PosicaoNoGeral(turma, 11, Dificuldade::Nivel::Normal, "111") == 12);
}

TEST_CASE("Ranking geral: curso de zero materias nao divide por zero") {

    // Nao deveria acontecer - materias.json sempre tem materia.
    CHECK(Ranking::Geral({Aluno("1", {{0, 50.0f}})}, 0, Dificuldade::Nivel::Normal).empty());
    CHECK(Ranking::PosicaoNoGeral({Aluno("1", {{0, 50.0f}})}, 0, Dificuldade::Nivel::Normal, "1") == 0);
}

TEST_CASE("Ranking geral: a contagem de materias jogadas acompanha a media") {

    // 17,10 sozinho parece uma nota pessima.
    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("1", {{0, 100.0f}, {1, 88.0f}}),
        Aluno("2", {{0, 90.0f}}),
    };

    const auto r = Ranking::Geral(turma, 11, Dificuldade::Nivel::Normal);
    REQUIRE(r.size() == 2);
    CHECK(r[0].materiasJogadas == 2);
    CHECK(r[1].materiasJogadas == 1);
}

TEST_CASE("Ranking de materia: a contagem nao e usada, e vale 1") {

    const auto r = Ranking::DaMateria(Turma({{"1", 80.0f}}), 0, Dificuldade::Nivel::Normal);
    REQUIRE(r.size() == 1);
    CHECK(r[0].materiasJogadas == 1);
}

// ---- O ranking de duplas ----

namespace {

    Exportacao::FichaDeAluno& Acha(std::vector<Exportacao::FichaDeAluno>& fichas,
                                   const std::string& matricula) {
        for (auto& f : fichas) {
            if (f.matricula == matricula) return f;
        }
        Exportacao::FichaDeAluno nova;
        nova.matricula = matricula;
        fichas.push_back(nova);
        return fichas.back();
    }

    /// Uma batalha em dupla, gravada dos dois lados - como o jogo grava.
    void Dupla(std::vector<Exportacao::FichaDeAluno>& fichas, const std::string& a,
               const std::string& b, const int materia, const float nota) {
        Acha(fichas, a).progresso.RegistrarNotaDeDupla(materia, nota, Dificuldade::Nivel::Normal, b);
        Acha(fichas, b).progresso.RegistrarNotaDeDupla(materia, nota, Dificuldade::Nivel::Normal, a);
    }
}

TEST_CASE("Duplas: a mesma dupla nas duas fichas e UMA linha") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "89384", "90112", 0, 83.0f);

    const auto r = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 1);
    CHECK(r[0].um == "89384");
    CHECK(r[0].dois == "90112");
    CHECK(r[0].nota == doctest::Approx(83.0f));
    CHECK(r[0].posicao == 1);
}

TEST_CASE("Duplas: a ordem das matriculas na linha nao depende de quem foi o jogador 1") {

    std::vector<Exportacao::FichaDeAluno> deUmJeito;
    Dupla(deUmJeito, "90112", "89384", 0, 83.0f);

    std::vector<Exportacao::FichaDeAluno> doOutro;
    Dupla(doOutro, "89384", "90112", 0, 83.0f);

    const auto a = Ranking::DuplasDaMateria(deUmJeito, 0, Dificuldade::Nivel::Normal);
    const auto b = Ranking::DuplasDaMateria(doOutro, 0, Dificuldade::Nivel::Normal);

    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    CHECK(a[0].um == b[0].um);
    CHECK(a[0].dois == b[0].dois);
    CHECK(a[0].um == "89384");      // a menor primeiro
}

TEST_CASE("Duplas: a nota de solo nao entra aqui, e a de dupla nao entra no solo") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "sozinho").progresso.RegistrarNota(0, 100.0f, Dificuldade::Nivel::Normal);
    Dupla(fichas, "acompanhado", "parceiro", 0, 70.0f);

    // E ele tambem jogou sozinho, com nota maior que a da dupla.
    Acha(fichas, "acompanhado").progresso.RegistrarNota(0, 85.0f, Dificuldade::Nivel::Normal);

    const auto solo = Ranking::DaMateria(fichas, 0, Dificuldade::Nivel::Normal);
    REQUIRE(solo.size() == 2);
    CHECK(solo[0].matricula == "sozinho");
    CHECK(solo[1].matricula == "acompanhado");
    CHECK(solo[1].nota == doctest::Approx(85.0f));      // a de solo, nao a da dupla
    for (const auto& l : solo) { CAPTURE(l.matricula); CHECK(l.matricula != "parceiro"); }

    const auto duplas = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);
    REQUIRE(duplas.size() == 1);
    CHECK(duplas[0].um == "acompanhado");
    CHECK(duplas[0].dois == "parceiro");
    CHECK(duplas[0].nota == doctest::Approx(70.0f));    // a da dupla, nao a de solo
}

TEST_CASE("Duplas: o mesmo aluno pode estar nas duas listas") {

    // Jogar sozinho e jogar acompanhado sao duas disputas, e quem fez as duas disputa as duas.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "89384").progresso.RegistrarNota(0, 95.0f, Dificuldade::Nivel::Normal);
    Dupla(fichas, "89384", "90112", 0, 72.0f);

    CHECK(Ranking::PosicaoDe(fichas, 0, Dificuldade::Nivel::Normal, "89384") == 1);

    const auto duplas = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);
    REQUIRE(duplas.size() == 1);
    CHECK(duplas[0].nota == doctest::Approx(72.0f));
}

TEST_CASE("Duplas: uma dupla registrada de um lado so continua valendo") {

    // Os dois fizeram aquela nota juntos.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 80.0f);
    Acha(fichas, "A").progresso.RegistrarNotaDeDupla(0, 90.0f, Dificuldade::Nivel::Normal, "C");   // A foi melhor com C

    const auto r = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);

    // (A,C) com 90 e (A,B) com 80, que sobrou so na ficha de B.
    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "A");
    CHECK(r[0].dois == "C");
    CHECK(r[0].nota == doctest::Approx(90.0f));
    CHECK(r[1].um == "A");
    CHECK(r[1].dois == "B");
    CHECK(r[1].nota == doctest::Approx(80.0f));
}

TEST_CASE("Duplas: fichas que discordam valem a maior") {

    // No caminho normal os dois numeros sao iguais - e a mesma batalha gravada dos dois lados.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "A").progresso.RegistrarNotaDeDupla(0, 90.0f, Dificuldade::Nivel::Normal, "B");
    Acha(fichas, "B").progresso.RegistrarNotaDeDupla(0, 80.0f, Dificuldade::Nivel::Normal, "A");

    const auto r = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 1);
    CHECK(r[0].nota == doctest::Approx(90.0f));
}

TEST_CASE("Duplas: dupla com visitante nao entra na lista") {

    // Visitante nao tem matricula, e "89384 + " nao diz quem jogou.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "89384").progresso.RegistrarNotaDeDupla(0, 99.0f, Dificuldade::Nivel::Normal, "");

    CHECK(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal).empty());

    // Mas a nota esta na ficha, e o mapa do curso a ve.
    CHECK(fichas[0].progresso.Aprovado(0));
}

TEST_CASE("Duplas: ficha que se nomeia como o proprio parceiro nao entra") {

    // So sai de ficha editada a mao: Sessao::JaEstaEmUso impede duas posicoes com a mesma matricula.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "89384").progresso.RegistrarNotaDeDupla(0, 100.0f, Dificuldade::Nivel::Normal, "89384");

    CHECK(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal).empty());
}

TEST_CASE("Duplas: empate divide a posicao, com a mesma regra do solo") {

    // A regra de empate e uma (Numerar).
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 90.0f);
    Dupla(fichas, "20", "21", 0, 90.0f);
    Dupla(fichas, "30", "31", 0, 80.0f);

    const auto r = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 3);
    CHECK(r[0].posicao == 1);
    CHECK(r[1].posicao == 1);
    CHECK(r[2].posicao == 3);   // pula a 2
}

TEST_CASE("Duplas: o limite corta o fim, e zero devolve todas") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 10.0f);
    Dupla(fichas, "20", "21", 0, 100.0f);
    Dupla(fichas, "30", "31", 0, 50.0f);

    const auto cortado = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal, 2);
    REQUIRE(cortado.size() == 2);
    CHECK(cortado[0].um == "20");
    CHECK(cortado[1].um == "30");

    CHECK(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal, 0).size() == 3);
    CHECK(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal, 50).size() == 3);
}

TEST_CASE("Duplas: cada materia tem a sua ordem") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 100.0f);
    Dupla(fichas, "A", "B", 1, 40.0f);
    Dupla(fichas, "C", "D", 1, 95.0f);

    REQUIRE(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal).size() == 1);
    CHECK(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal)[0].um == "A");

    const auto segunda = Ranking::DuplasDaMateria(fichas, 1, Dificuldade::Nivel::Normal);
    REQUIRE(segunda.size() == 2);
    CHECK(segunda[0].um == "C");
}

TEST_CASE("Duplas: turma vazia devolve lista vazia") {

    CHECK(Ranking::DuplasDaMateria({}, 0, Dificuldade::Nivel::Normal).empty());
    CHECK(Ranking::DuplasGeral({}, 11, Dificuldade::Nivel::Normal).empty());
}

TEST_CASE("Duplas, geral: a media divide pelo CURSO, nao pelas materias jogadas juntas") {

    // A mesma decisao de Ranking::Geral, e pelo mesmo motivo.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "p1", "p2", 0, 100.0f);                     // uma materia, nota cheia

    Dupla(fichas, "f1", "f2", 0, 95.0f);                      // quatro materias
    Dupla(fichas, "f1", "f2", 1, 95.0f);
    Dupla(fichas, "f1", "f2", 2, 95.0f);
    Dupla(fichas, "f1", "f2", 3, 95.0f);

    const auto r = Ranking::DuplasGeral(fichas, 11, Dificuldade::Nivel::Normal);

    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "f1");
    CHECK(r[0].nota == doctest::Approx(380.0f / 11.0f));
    CHECK(r[1].um == "p1");
    CHECK(r[1].nota == doctest::Approx(100.0f / 11.0f));
}

TEST_CASE("Duplas, geral: a contagem e das materias jogadas JUNTAS") {

    // "2 de 11 jogadas, media 17,10" conta a historia certa, como no geral de solo.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 90.0f);
    Dupla(fichas, "A", "B", 1, 80.0f);
    Dupla(fichas, "A", "C", 2, 70.0f);

    const auto r = Ranking::DuplasGeral(fichas, 11, Dificuldade::Nivel::Normal);

    // Tres fichas, duas duplas: (A,B) com duas materias e (A,C) com uma.
    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "A");
    CHECK(r[0].dois == "B");
    CHECK(r[0].materiasJogadas == 2);
    CHECK(r[0].nota == doctest::Approx(170.0f / 11.0f));
    CHECK(r[1].dois == "C");
    CHECK(r[1].materiasJogadas == 1);
}

TEST_CASE("Duplas, geral: a media de dupla nao entra no geral de solo") {

    // O geral le MelhorNotaSolo, e nao MelhorNota.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 100.0f);

    CHECK(Ranking::Geral(fichas, 11, Dificuldade::Nivel::Normal).empty());
    CHECK(Ranking::PosicaoNoGeral(fichas, 11, Dificuldade::Nivel::Normal, "A") == 0);

    const auto r = Ranking::DuplasGeral(fichas, 11, Dificuldade::Nivel::Normal);
    REQUIRE(r.size() == 1);
    CHECK(r[0].nota == doctest::Approx(100.0f / 11.0f));
}

TEST_CASE("Duplas, geral: curso de zero materias nao divide por zero") {

    // Dividir por zero poria a nota em NaN, e NaN ordenado produz lista aleatoria.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 50.0f);

    CHECK(Ranking::DuplasGeral(fichas, 0, Dificuldade::Nivel::Normal).empty());
}

TEST_CASE("Duplas, geral: o limite corta o fim") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 10.0f);
    Dupla(fichas, "20", "21", 0, 100.0f);
    Dupla(fichas, "30", "31", 0, 50.0f);

    const auto r = Ranking::DuplasGeral(fichas, 11, Dificuldade::Nivel::Normal, 2);
    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "20");
    CHECK(r[1].um == "30");
}

TEST_CASE("Duplas: a materia conta 1 como no solo") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 80.0f);

    const auto r = Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal);
    REQUIRE(r.size() == 1);
    CHECK(r[0].materiasJogadas == 1);
}

TEST_CASE("Duplas: a posicao de uma dupla nao depende da ordem dos argumentos") {

    // Quem pergunta e a tela, que tem o jogador 1 e o jogador 2 em maos.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 90.0f);
    Dupla(fichas, "20", "21", 0, 80.0f);

    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "20", "21") == 2);
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "21", "20") == 2);
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "11", "10") == 1);
}

TEST_CASE("Duplas: a posicao vem da lista COMPLETA, e nao do topo exibido") {

    // O mesmo motivo de PosicaoDe.
    std::vector<Exportacao::FichaDeAluno> fichas;
    for (int i = 0; i < 12; ++i) {
        const std::string a = "a" + std::to_string(100 + i);
        const std::string b = "b" + std::to_string(100 + i);
        Dupla(fichas, a, b, 0, 100.0f - static_cast<float>(i));
    }

    CHECK(Ranking::DuplasDaMateria(fichas, 0, Dificuldade::Nivel::Normal, 5).size() == 5);
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "a111", "b111") == 12);
    CHECK(Ranking::PosicaoDaDuplaNoGeral(fichas, 11, Dificuldade::Nivel::Normal, "a111", "b111") == 12);
}

TEST_CASE("Duplas: quem nunca jogou junto com aquela pessoa fica em zero") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 90.0f);
    Dupla(fichas, "C", "D", 0, 80.0f);

    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "A", "C") == 0);      // jogaram, mas nao juntos
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "A", "Z") == 0);      // Z nem existe
    CHECK(Ranking::PosicaoDaDupla(fichas, 1, Dificuldade::Nivel::Normal, "A", "B") == 0);      // outra materia
    CHECK(Ranking::PosicaoDaDupla({}, 0, Dificuldade::Nivel::Normal, "A", "B") == 0);          // turma vazia
    CHECK(Ranking::PosicaoDaDuplaNoGeral(fichas, 0, Dificuldade::Nivel::Normal, "A", "B") == 0);   // curso de zero materias
}

TEST_CASE("Duplas: a posicao de quem jogou sozinho nao vaza para a lista de duplas") {

    // As duas perguntas tem sujeitos diferentes.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "A").progresso.RegistrarNota(0, 95.0f, Dificuldade::Nivel::Normal);
    Acha(fichas, "B").progresso.RegistrarNota(0, 90.0f, Dificuldade::Nivel::Normal);

    CHECK(Ranking::PosicaoDe(fichas, 0, Dificuldade::Nivel::Normal, "A") == 1);
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "A", "B") == 0);
}

TEST_CASE("Duplas: empatadas dividem a posicao tambem na busca") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 90.0f);
    Dupla(fichas, "20", "21", 0, 90.0f);
    Dupla(fichas, "30", "31", 0, 80.0f);

    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "10", "11") == 1);
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "20", "21") == 1);
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Dificuldade::Nivel::Normal, "30", "31") == 3);
}

// As quatro listas.

TEST_CASE("Ranking: as quatro listas, na ordem das abas, e o manche da a volta") {
    using Dificuldade::Nivel;
    using Modo = Progresso::Modo;
    REQUIRE(Ranking::kQuantasListas == 4);
    CHECK(Ranking::NomeDaLista(Ranking::kListas[0]) == "Normal");
    CHECK(Ranking::NomeDaLista(Ranking::kListas[1]) == "Dif\xc3\xad" "cil");
    // Oito bytes, contados e nao escritos.
    CHECK(Ranking::NomeDaLista(Ranking::kListas[1]).size() == 8u);
    CHECK(Ranking::NomeDaLista(Ranking::kListas[2]) == "Normal em dupla");
    CHECK(Ranking::NomeDaLista(Ranking::kListas[3]) == "Dif\xc3\xad" "cil em dupla");

    for (int i = 0; i < Ranking::kQuantasListas; ++i) {
        CHECK(Ranking::IndiceDaLista(Ranking::kListas[i].modo, Ranking::kListas[i].nivel) == i);
    }
    CHECK(Ranking::IndiceDaLista(Modo::Dupla, Nivel::Dificil) == 3);
    CHECK(Ranking::MoverLista(3, 1) == 0);
    CHECK(Ranking::MoverLista(0, -1) == 3);
    CHECK(Ranking::MoverLista(1, 1) == 2);
}

TEST_CASE("Ranking: a nota do Dificil fica na lista do Dificil, e so nela") {
    using Dificuldade::Nivel;

    std::vector<Exportacao::FichaDeAluno> fichas(2);
    fichas[0].matricula = "1";
    fichas[0].progresso.RegistrarNota(0, 90.0f, Nivel::Normal);
    fichas[1].matricula = "2";
    fichas[1].progresso.RegistrarNota(0, 75.0f, Nivel::Dificil);

    const auto normal = Ranking::DaMateria(fichas, 0, Nivel::Normal);
    const auto dificil = Ranking::DaMateria(fichas, 0, Nivel::Dificil);
    REQUIRE(normal.size() == 1u);
    REQUIRE(dificil.size() == 1u);
    CHECK(normal[0].matricula == "1");
    CHECK(dificil[0].matricula == "2");

    // E a posicao e a da lista: o 2 e o primeiro do Dificil, e nem aparece no Normal.
    CHECK(Ranking::PosicaoDe(fichas, 0, Nivel::Dificil, "2") == 1);
    CHECK(Ranking::PosicaoDe(fichas, 0, Nivel::Normal, "2") == 0);
    CHECK(Ranking::Geral(fichas, 1, Nivel::Dificil).size() == 1u);
}

TEST_CASE("Ranking: a dupla do Dificil fica na lista de duplas do Dificil") {
    using Dificuldade::Nivel;

    std::vector<Exportacao::FichaDeAluno> fichas(2);
    fichas[0].matricula = "1";
    fichas[0].progresso.RegistrarNotaDeDupla(0, 70.0f, Nivel::Dificil, "2");
    fichas[1].matricula = "2";
    fichas[1].progresso.RegistrarNotaDeDupla(0, 70.0f, Nivel::Dificil, "1");

    CHECK(Ranking::DuplasDaMateria(fichas, 0, Nivel::Normal).empty());
    const auto d = Ranking::DuplasDaMateria(fichas, 0, Nivel::Dificil);
    REQUIRE(d.size() == 1u);                 // A mesma dupla nas duas fichas e uma linha.
    CHECK(d[0].um == "1");
    CHECK(d[0].dois == "2");
    CHECK(Ranking::PosicaoDaDupla(fichas, 0, Nivel::Dificil, "2", "1") == 1);
    CHECK(Ranking::DuplasGeral(fichas, 1, Nivel::Dificil).size() == 1u);
}
