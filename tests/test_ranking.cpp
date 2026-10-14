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

// ------------------------------------------------------- o ranking geral

namespace {

    /// Um aluno com notas em varias materias: {materia, nota}.
    Exportacao::FichaDeAluno Aluno(const std::string& matricula,
                                   const std::vector<std::pair<int, float>>& notas) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        for (const auto& [materia, nota] : notas) f.progresso.RegistrarNota(materia, nota);
        return f;
    }
}

TEST_CASE("Ranking geral: a media divide pelo CURSO, nao pelas materias jogadas") {

    // A DECISAO QUE DEFINE O RANKING INTEIRO. Dividindo pelas jogadas, quem fez
    // UMA materia e tirou 100 ficaria em primeiro, na frente de quem fez oito
    // com 95 de media - e o ranking premiaria jogar pouco.
    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("preguicoso", {{0, 100.0f}}),
        Aluno("aplicado",   {{0, 95.0f}, {1, 95.0f}, {2, 95.0f}, {3, 95.0f}}),
    };

    const auto r = Ranking::Geral(turma, 11);

    REQUIRE(r.size() == 2);
    CHECK(r[0].matricula == "aplicado");
    CHECK(r[1].matricula == "preguicoso");

    // 4 x 95 / 11 = 34,55   contra   100 / 11 = 9,09
    CHECK(r[0].nota == doctest::Approx(380.0f / 11.0f));
    CHECK(r[1].nota == doctest::Approx(100.0f / 11.0f));
}

TEST_CASE("Ranking geral: materia nao jogada vale zero, e nao e ignorada") {

    // Se as nao jogadas fossem ignoradas, a media voltaria a ser a das jogadas e
    // o incentivo se inverteria de novo.
    const std::vector<Exportacao::FichaDeAluno> turma = { Aluno("1", {{0, 100.0f}, {1, 100.0f}}) };

    CHECK(Ranking::Geral(turma, 2)[0].nota  == doctest::Approx(100.0f));
    CHECK(Ranking::Geral(turma, 4)[0].nota  == doctest::Approx(50.0f));
    CHECK(Ranking::Geral(turma, 10)[0].nota == doctest::Approx(20.0f));
}

TEST_CASE("Ranking geral: avancar no curso sempre sobe a media") {

    // A PROPRIEDADE QUE FAZ O NUMERO MEDIR AVANCO. Jogar uma materia a mais
    // nunca pode baixar a posicao de ninguem - senao o jogo estaria pedindo para
    // o aluno parar de jogar.
    auto so3 = Aluno("1", {{0, 90.0f}, {1, 90.0f}, {2, 90.0f}});
    auto com4 = so3;
    com4.progresso.RegistrarNota(3, 10.0f);   // uma materia RUIM a mais

    const float antes  = Ranking::Geral({so3}, 11)[0].nota;
    const float depois = Ranking::Geral({com4}, 11)[0].nota;

    CAPTURE(antes);
    CAPTURE(depois);
    CHECK(depois > antes);
}

TEST_CASE("Ranking geral: quem nao jogou nada nao entra") {

    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("jogou", {{0, 50.0f}}),
        Aluno("nao", {}),
    };

    const auto r = Ranking::Geral(turma, 11);
    REQUIRE(r.size() == 1);
    CHECK(r[0].matricula == "jogou");
    CHECK(Ranking::PosicaoNoGeral(turma, 11, "nao") == 0);
}

TEST_CASE("Ranking geral: empate divide a posicao, como na materia") {

    // As duas listas passam pela mesma ordenacao, entao a regra de empate tem de
    // ser a mesma. Se divergisse, o mesmo par apareceria empatado numa tela e
    // desempatado na outra.
    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("10", {{0, 80.0f}}), Aluno("20", {{1, 80.0f}}), Aluno("30", {{0, 40.0f}}),
    };

    const auto r = Ranking::Geral(turma, 11);
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

    CHECK(Ranking::Geral(turma, 11, 5).size() == 5);
    CHECK(Ranking::PosicaoNoGeral(turma, 11, "111") == 12);
}

TEST_CASE("Ranking geral: curso de zero materias nao divide por zero") {

    // Nao deveria acontecer - materias.json sempre tem materia - mas dividir por
    // zero poria uma nota em NaN, e NaN ordenado produz lista aleatoria.
    CHECK(Ranking::Geral({Aluno("1", {{0, 50.0f}})}, 0).empty());
    CHECK(Ranking::PosicaoNoGeral({Aluno("1", {{0, 50.0f}})}, 0, "1") == 0);
}

TEST_CASE("Ranking geral: a contagem de materias jogadas acompanha a media") {

    // 17,10 sozinho parece uma nota pessima. "2 de 11 jogadas, media 17,10"
    // conta a historia certa, e e por isso que a contagem viaja junto.
    const std::vector<Exportacao::FichaDeAluno> turma = {
        Aluno("1", {{0, 100.0f}, {1, 88.0f}}),
        Aluno("2", {{0, 90.0f}}),
    };

    const auto r = Ranking::Geral(turma, 11);
    REQUIRE(r.size() == 2);
    CHECK(r[0].materiasJogadas == 2);
    CHECK(r[1].materiasJogadas == 1);
}

TEST_CASE("Ranking de materia: a contagem nao e usada, e vale 1") {

    const auto r = Ranking::DaMateria(Turma({{"1", 80.0f}}), 0);
    REQUIRE(r.size() == 1);
    CHECK(r[0].materiasJogadas == 1);
}

// -------------------------------------------------- o ranking de DUPLAS

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

    /// Uma batalha em dupla, gravada DOS DOIS LADOS - como o jogo grava: cada
    /// aluno guarda na propria ficha o recorde de dupla e o nome de quem estava
    /// com ele. E por isso que a mesma dupla aparece duas vezes nas fichas, e e
    /// isso que o ranking tem de juntar.
    void Dupla(std::vector<Exportacao::FichaDeAluno>& fichas, const std::string& a,
               const std::string& b, const int materia, const float nota) {
        Acha(fichas, a).progresso.RegistrarNotaDeDupla(materia, nota, b);
        Acha(fichas, b).progresso.RegistrarNotaDeDupla(materia, nota, a);
    }
}

TEST_CASE("Duplas: a mesma dupla nas duas fichas e UMA linha") {

    // A GRAVACAO E DOS DOIS LADOS, entao sem juntar pela chave ordenada a dupla
    // (A,B) da ficha de A e a (B,A) da ficha de B virariam duas linhas - a mesma
    // dupla competindo contra si mesma, com a mesma nota, ocupando duas
    // posicoes do topo.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "89384", "90112", 0, 83.0f);

    const auto r = Ranking::DuplasDaMateria(fichas, 0);

    REQUIRE(r.size() == 1);
    CHECK(r[0].um == "89384");
    CHECK(r[0].dois == "90112");
    CHECK(r[0].nota == doctest::Approx(83.0f));
    CHECK(r[0].posicao == 1);
}

TEST_CASE("Duplas: a ordem das matriculas na linha nao depende de quem foi o jogador 1") {

    // Senao a mesma dupla apareceria como "A + B" numa partida e "B + A" na
    // outra, e quem procura o proprio nome nao saberia em qual olhar.
    std::vector<Exportacao::FichaDeAluno> deUmJeito;
    Dupla(deUmJeito, "90112", "89384", 0, 83.0f);

    std::vector<Exportacao::FichaDeAluno> doOutro;
    Dupla(doOutro, "89384", "90112", 0, 83.0f);

    const auto a = Ranking::DuplasDaMateria(deUmJeito, 0);
    const auto b = Ranking::DuplasDaMateria(doOutro, 0);

    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    CHECK(a[0].um == b[0].um);
    CHECK(a[0].dois == b[0].dois);
    CHECK(a[0].um == "89384");      // a menor primeiro
}

TEST_CASE("Duplas: a nota de solo nao entra aqui, e a de dupla nao entra no solo") {

    // O MOTIVO DE EXISTIREM DUAS LISTAS (requisitos-coop, D9). O co-op e
    // levemente mais facil, entao uma lista unica faria as notas de dupla
    // empurrarem para baixo quem jogou sozinho.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "sozinho").progresso.RegistrarNota(0, 100.0f);
    Dupla(fichas, "acompanhado", "parceiro", 0, 70.0f);

    // E ELE TAMBEM JOGOU SOZINHO, com nota MAIOR que a da dupla. E o que faz
    // este caso medir a separacao: se a lista de duplas lesse MelhorNota (a
    // maior das duas), ela mostraria 85 - uma nota que nenhuma dupla fez.
    Acha(fichas, "acompanhado").progresso.RegistrarNota(0, 85.0f);

    const auto solo = Ranking::DaMateria(fichas, 0);
    REQUIRE(solo.size() == 2);
    CHECK(solo[0].matricula == "sozinho");
    CHECK(solo[1].matricula == "acompanhado");
    CHECK(solo[1].nota == doctest::Approx(85.0f));      // a de solo, nao a da dupla
    for (const auto& l : solo) { CAPTURE(l.matricula); CHECK(l.matricula != "parceiro"); }

    const auto duplas = Ranking::DuplasDaMateria(fichas, 0);
    REQUIRE(duplas.size() == 1);
    CHECK(duplas[0].um == "acompanhado");
    CHECK(duplas[0].dois == "parceiro");
    CHECK(duplas[0].nota == doctest::Approx(70.0f));    // a da dupla, nao a de solo
}

TEST_CASE("Duplas: o mesmo aluno pode estar nas duas listas") {

    // Jogar sozinho e jogar acompanhado sao duas disputas, e quem fez as duas
    // disputa as duas. Nao e nota dobrada: sao numeros diferentes, de batalhas
    // diferentes, em telas diferentes.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "89384").progresso.RegistrarNota(0, 95.0f);
    Dupla(fichas, "89384", "90112", 0, 72.0f);

    CHECK(Ranking::PosicaoDe(fichas, 0, "89384") == 1);

    const auto duplas = Ranking::DuplasDaMateria(fichas, 0);
    REQUIRE(duplas.size() == 1);
    CHECK(duplas[0].nota == doctest::Approx(72.0f));
}

TEST_CASE("Duplas: uma dupla registrada de um lado so continua valendo") {

    // Os dois FIZERAM aquela nota juntos. Se depois um deles for melhor com
    // outra pessoa, o recorde dele passa a nomear a outra dupla e a primeira
    // fica gravada so num lado - exigir os dois lados apagaria do ranking uma
    // dupla que existiu.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 80.0f);
    Acha(fichas, "A").progresso.RegistrarNotaDeDupla(0, 90.0f, "C");   // A foi melhor com C

    const auto r = Ranking::DuplasDaMateria(fichas, 0);

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

    // No caminho normal os dois numeros sao iguais - e a mesma batalha gravada
    // dos dois lados. Divergem quando uma das fichas nao chegou ao disco (queda
    // de energia no gabinete entre uma gravacao e a outra); a dupla FEZ a nota
    // maior, e o outro lado so nao registrou.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "A").progresso.RegistrarNotaDeDupla(0, 90.0f, "B");
    Acha(fichas, "B").progresso.RegistrarNotaDeDupla(0, 80.0f, "A");

    const auto r = Ranking::DuplasDaMateria(fichas, 0);

    REQUIRE(r.size() == 1);
    CHECK(r[0].nota == doctest::Approx(90.0f));
}

TEST_CASE("Duplas: dupla com visitante nao entra na lista") {

    // Visitante nao tem matricula, e "89384 + " nao diz quem jogou. A nota
    // continua valendo para o progresso de quem se identificou (RF6) - e a
    // LINHA do ranking que precisa de dois nomes.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "89384").progresso.RegistrarNotaDeDupla(0, 99.0f, "");

    CHECK(Ranking::DuplasDaMateria(fichas, 0).empty());

    // Mas a nota esta na ficha, e o mapa do curso a ve.
    CHECK(fichas[0].progresso.Aprovado(0));
}

TEST_CASE("Duplas: ficha que se nomeia como o proprio parceiro nao entra") {

    // So sai de ficha editada a mao: Sessao::JaEstaEmUso impede duas posicoes
    // com a mesma matricula. Sem o corte, um erro de digitacao na pasta do
    // professor poria "89384 + 89384" no topo do ranking.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Acha(fichas, "89384").progresso.RegistrarNotaDeDupla(0, 100.0f, "89384");

    CHECK(Ranking::DuplasDaMateria(fichas, 0).empty());
}

TEST_CASE("Duplas: empate divide a posicao, com a mesma regra do solo") {

    // A regra de empate e UMA (Numerar). Se cada lista numerasse a sua, o mesmo
    // empate sairia "1o e 1o" numa tela e "1o e 2o" na outra, e seria a tela, e
    // nao a nota, decidindo quem ganhou.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 90.0f);
    Dupla(fichas, "20", "21", 0, 90.0f);
    Dupla(fichas, "30", "31", 0, 80.0f);

    const auto r = Ranking::DuplasDaMateria(fichas, 0);

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

    const auto cortado = Ranking::DuplasDaMateria(fichas, 0, 2);
    REQUIRE(cortado.size() == 2);
    CHECK(cortado[0].um == "20");
    CHECK(cortado[1].um == "30");

    CHECK(Ranking::DuplasDaMateria(fichas, 0, 0).size() == 3);
    CHECK(Ranking::DuplasDaMateria(fichas, 0, 50).size() == 3);
}

TEST_CASE("Duplas: cada materia tem a sua ordem") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 100.0f);
    Dupla(fichas, "A", "B", 1, 40.0f);
    Dupla(fichas, "C", "D", 1, 95.0f);

    REQUIRE(Ranking::DuplasDaMateria(fichas, 0).size() == 1);
    CHECK(Ranking::DuplasDaMateria(fichas, 0)[0].um == "A");

    const auto segunda = Ranking::DuplasDaMateria(fichas, 1);
    REQUIRE(segunda.size() == 2);
    CHECK(segunda[0].um == "C");
}

TEST_CASE("Duplas: turma vazia devolve lista vazia") {

    CHECK(Ranking::DuplasDaMateria({}, 0).empty());
    CHECK(Ranking::DuplasGeral({}, 11).empty());
}

TEST_CASE("Duplas, geral: a media divide pelo CURSO, nao pelas materias jogadas juntas") {

    // A mesma decisao de Ranking::Geral, e pelo mesmo motivo: assim subir exige
    // jogar mais materias COM A MESMA PESSOA, que e o que uma lista de duplas
    // deveria premiar. Dividindo pelas jogadas, a dupla de uma materia so
    // passaria na frente da que jogou o curso inteiro.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "p1", "p2", 0, 100.0f);                     // uma materia, nota cheia

    Dupla(fichas, "f1", "f2", 0, 95.0f);                      // quatro materias
    Dupla(fichas, "f1", "f2", 1, 95.0f);
    Dupla(fichas, "f1", "f2", 2, 95.0f);
    Dupla(fichas, "f1", "f2", 3, 95.0f);

    const auto r = Ranking::DuplasGeral(fichas, 11);

    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "f1");
    CHECK(r[0].nota == doctest::Approx(380.0f / 11.0f));
    CHECK(r[1].um == "p1");
    CHECK(r[1].nota == doctest::Approx(100.0f / 11.0f));
}

TEST_CASE("Duplas, geral: a contagem e das materias jogadas JUNTAS") {

    // "2 de 11 jogadas, media 17,10" conta a historia certa, como no geral de
    // solo. Aqui ela conta algo a mais: quanto do curso esta dupla fez junta.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 90.0f);
    Dupla(fichas, "A", "B", 1, 80.0f);
    Dupla(fichas, "A", "C", 2, 70.0f);

    const auto r = Ranking::DuplasGeral(fichas, 11);

    // Tres fichas, DUAS duplas: (A,B) com duas materias e (A,C) com uma.
    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "A");
    CHECK(r[0].dois == "B");
    CHECK(r[0].materiasJogadas == 2);
    CHECK(r[0].nota == doctest::Approx(170.0f / 11.0f));
    CHECK(r[1].dois == "C");
    CHECK(r[1].materiasJogadas == 1);
}

TEST_CASE("Duplas, geral: a media de dupla nao entra no geral de solo") {

    // O geral le MelhorNotaSolo, e nao MelhorNota. MelhorNota devolve a maior
    // das duas porque o MAPA DO CURSO precisa dela - passar e passar (D10) -, e
    // usa-la aqui faria as notas de dupla subirem o ranking de quem jogou
    // sozinho, que e o contrario do que as duas listas existem para fazer.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 100.0f);

    CHECK(Ranking::Geral(fichas, 11).empty());
    CHECK(Ranking::PosicaoNoGeral(fichas, 11, "A") == 0);

    const auto r = Ranking::DuplasGeral(fichas, 11);
    REQUIRE(r.size() == 1);
    CHECK(r[0].nota == doctest::Approx(100.0f / 11.0f));
}

TEST_CASE("Duplas, geral: curso de zero materias nao divide por zero") {

    // Dividir por zero poria a nota em NaN, e NaN ordenado produz lista
    // aleatoria. Mesma protecao de Ranking::Geral.
    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 50.0f);

    CHECK(Ranking::DuplasGeral(fichas, 0).empty());
}

TEST_CASE("Duplas, geral: o limite corta o fim") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "10", "11", 0, 10.0f);
    Dupla(fichas, "20", "21", 0, 100.0f);
    Dupla(fichas, "30", "31", 0, 50.0f);

    const auto r = Ranking::DuplasGeral(fichas, 11, 2);
    REQUIRE(r.size() == 2);
    CHECK(r[0].um == "20");
    CHECK(r[1].um == "30");
}

TEST_CASE("Duplas: a materia conta 1 como no solo") {

    std::vector<Exportacao::FichaDeAluno> fichas;
    Dupla(fichas, "A", "B", 0, 80.0f);

    const auto r = Ranking::DuplasDaMateria(fichas, 0);
    REQUIRE(r.size() == 1);
    CHECK(r[0].materiasJogadas == 1);
}
