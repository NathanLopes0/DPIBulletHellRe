//
// O registro de cada tentativa de batalha: os dados para a analise do TCC.
//

#include "../doctest.h"

#include <string>
#include <vector>

#include "../../Source/Data/Tentativas.h"

using namespace Tentativas;

namespace {

    Tentativa UmaDupla() {
        Tentativa t;
        t.id = NovoId("2026-10-28 14:03:22", 0x0f3a9c21u);
        t.versaoDoJogo = "abc1234";
        t.inicio = "2026-10-28 14:03:22";
        t.fim = "2026-10-28 14:05:10";
        t.duracao = 101.5f;
        t.tempoEmPausa = 4.25f;
        t.materia = "INF213";
        t.nomeDaMateria = "Estruturas, de Dados";   // a virgula vai entre aspas
        t.professor = "Salles";
        t.dificuldade = "dificil";
        t.modo = "dupla";
        t.resultado = Resultado::Aprovado;
        t.notaFinal = 62.5f;
        t.notaMaxima = 64.0f;
        t.notaMinima = 31.0f;
        t.fasesLimpas = 1;
        t.acertosSofridos = 3;
        t.semente = 42;
        Jogador a;
        a.posicao = 1; a.matricula = "12345"; a.parceiro = "67890";
        a.tiros = 200; a.acertosNoProfessor = 50; a.acertosSofridos = 2;
        a.pontosExtrasPegos = 1; a.pontosExtrasUsados = 2;
        Jogador b;
        b.posicao = 2; b.matricula = "67890"; b.parceiro = "12345";
        b.acertosSofridos = 1;
        t.jogadores = {a, b};
        Fase f1;
        f1.ordem = 1; f1.nome = "StateOne"; f1.anuncio = "push()"; f1.fim = 17.0f;
        f1.notaNoInicio = 40.0f; f1.notaNoFim = 52.5f; f1.completa = true; f1.limpa = true;
        Fase f2;
        f2.ordem = 2; f2.nome = "StateTwo"; f2.inicio = 17.0f; f2.fim = 30.0f;
        f2.notaNoInicio = 52.5f; f2.notaNoFim = 49.0f; f2.acertosSofridos = 3;
        t.fases = {f1, f2};
        return t;
    }

    /// O valor da coluna 'nome' numa linha, pelo cabecalho.
    std::string Coluna(const std::string& cabecalho, const std::string& linha, const std::string& nome) {
        const auto nomes = Campos(cabecalho);
        const auto valores = Campos(linha);
        for (size_t i = 0; i < nomes.size(); ++i) {
            if (nomes[i] == nome) return i < valores.size() ? valores[i] : "<curta>";
        }
        return "<sem coluna>";
    }
}

TEST_CASE("Tentativas: o id ordena no tempo e o dia da semana sai da data") {
    CHECK(NovoId("2026-10-28 14:03:22", 0x0f3a9c21u) == "20261028-140322-0f3a9c21");
    CHECK(NovoId("2026-10-28 14:03:22", 1u) < NovoId("2026-10-28 14:03:23", 0u));
    CHECK(DiaDaSemana("2026-10-28") == "quarta");
    CHECK(DiaDaSemana("2026-11-01 08:00:00") == "domingo");
    CHECK(DiaDaSemana("2024-02-29") == "quinta");   // bissexto
    CHECK(DiaDaSemana("2000-01-01") == "sabado");
    CHECK(DiaDaSemana("").empty());
    CHECK(DiaDaSemana("ontem").empty());
}

TEST_CASE("Tentativas: uma linha por aluno, com a vez de cada um e as colunas no lugar") {
    const Tentativa t = UmaDupla();
    const auto linhas = LinhasDaTentativa(t, {3, 1});
    REQUIRE(linhas.size() == 2u);

    const std::string cab = CabecalhoDasTentativas();
    // TODA LINHA TEM AS COLUNAS DO CABECALHO, nem mais nem menos - mesmo com a
    // virgula do nome da materia.
    CHECK(Campos(linhas[0]).size() == Campos(cab).size());
    CHECK(Campos(linhas[1]).size() == Campos(cab).size());

    CHECK(Coluna(cab, linhas[0], "id") == Coluna(cab, linhas[1], "id"));
    CHECK(Coluna(cab, linhas[0], "matricula") == "12345");
    CHECK(Coluna(cab, linhas[0], "parceiro") == "67890");
    CHECK(Coluna(cab, linhas[0], "vez") == "3");
    CHECK(Coluna(cab, linhas[1], "vez") == "1");
    CHECK(Coluna(cab, linhas[0], "nome_da_materia") == "Estruturas, de Dados");
    CHECK(Coluna(cab, linhas[0], "resultado") == "aprovado");
    CHECK(Coluna(cab, linhas[0], "aprovado") == "1");
    CHECK(Coluna(cab, linhas[0], "data") == "2026-10-28");
    CHECK(Coluna(cab, linhas[0], "hora") == "14:03:22");
    CHECK(Coluna(cab, linhas[0], "dia_da_semana") == "quarta");
    CHECK(Coluna(cab, linhas[0], "duracao_s") == "101.50");   // ponto, nunca virgula
    CHECK(Coluna(cab, linhas[0], "nota_final") == "62.50");
    CHECK(Coluna(cab, linhas[0], "fases_jogadas") == "2");
    CHECK(Coluna(cab, linhas[0], "acertos_sofridos_dupla") == "3");
    CHECK(Coluna(cab, linhas[0], "acertos_sofridos") == "2");
    CHECK(Coluna(cab, linhas[0], "precisao") == "0.250");
    // QUEM NAO ATIROU NAO TEM PRECISAO, e nao precisao zero: e "sem dado".
    CHECK(Coluna(cab, linhas[1], "precisao").empty());
    CHECK(Coluna(cab, linhas[0], "formato") == std::to_string(kVersaoDoFormato));
}

TEST_CASE("Tentativas: uma linha por fase, com a variacao da nota") {
    const Tentativa t = UmaDupla();
    const auto linhas = LinhasDasFases(t);
    REQUIRE(linhas.size() == 2u);
    const std::string cab = CabecalhoDasFases();
    CHECK(Campos(linhas[1]).size() == Campos(cab).size());
    CHECK(Coluna(cab, linhas[0], "fase") == "StateOne");
    CHECK(Coluna(cab, linhas[0], "anuncio") == "push()");
    CHECK(Coluna(cab, linhas[0], "variacao") == "12.50");
    CHECK(Coluna(cab, linhas[0], "limpa") == "1");
    CHECK(Coluna(cab, linhas[1], "variacao") == "-3.50");
    CHECK(Coluna(cab, linhas[1], "duracao_s") == "13.00");
    CHECK(Coluna(cab, linhas[1], "completa") == "0");
    CHECK(Coluna(cab, linhas[1], "matricula_2") == "67890");
}

TEST_CASE("Tentativas: os campos respeitam aspas") {
    const auto c = Campos("a,\"b, c\",\"d \"\"e\"\"\",,f\r");
    REQUIRE(c.size() == 5u);
    CHECK(c[1] == "b, c");
    CHECK(c[2] == "d \"e\"");
    CHECK(c[3].empty());
    CHECK(c[4] == "f");
}

TEST_CASE("Tentativas: o arquivo lido de volta da a vez e o resumo") {
    Tentativa t = UmaDupla();
    std::string csv = CabecalhoDasTentativas() + "\n";
    const auto acrescentar = [&](const Resultado r, const std::string& inicio, const float nota) {
        t.resultado = r;
        t.inicio = inicio;
        t.notaFinal = nota;
        for (const auto& l : LinhasDaTentativa(t, {0, 0})) csv += l + "\n";
    };
    acrescentar(Resultado::Reprovado, "2026-10-27 10:00:00", 20.0f);
    acrescentar(Resultado::Abandonou, "2026-10-27 10:05:00", 35.0f);
    acrescentar(Resultado::Aprovado, "2026-10-28 09:00:00", 70.0f);
    acrescentar(Resultado::Aprovado, "2026-10-28 09:10:00", 80.0f);

    const auto linhas = Ler(csv);
    REQUIRE(linhas.size() == 8u);   // duas por tentativa
    CHECK(linhas[0].matricula == "12345");
    CHECK(linhas[0].nomeDaMateria == "Estruturas, de Dados");
    CHECK(linhas[7].nota == doctest::Approx(80.0f));

    CHECK(ProximaVez(linhas, "12345", "INF213") == 5);
    CHECK(ProximaVez(linhas, "12345", "INF420") == 1);
    CHECK(ProximaVez(linhas, "99999", "INF213") == 1);

    const std::string resumo = Resumo(linhas);
    const auto r = Campos(resumo.substr(0, resumo.find('\n')));
    // Uma linha por aluno (2) e mais o cabecalho.
    size_t quebras = 0;
    for (const char c : resumo) quebras += c == '\n';
    CHECK(quebras == 3u);
    const std::string primeira = resumo.substr(resumo.find('\n') + 1,
                                               resumo.find('\n', resumo.find('\n') + 1) - resumo.find('\n') - 1);
    const std::string cab = resumo.substr(0, resumo.find('\n'));
    CHECK(Coluna(cab, primeira, "matricula") == "12345");
    CHECK(Coluna(cab, primeira, "tentativas") == "4");
    CHECK(Coluna(cab, primeira, "aprovacoes") == "2");
    CHECK(Coluna(cab, primeira, "reprovacoes") == "1");
    CHECK(Coluna(cab, primeira, "abandonos") == "1");
    CHECK(Coluna(cab, primeira, "tentativas_ate_aprovar") == "3");
    CHECK(Coluna(cab, primeira, "primeira_aprovacao") == "2026-10-28 09:00:00");
    CHECK(Coluna(cab, primeira, "dias_distintos") == "2");
    CHECK(Coluna(cab, primeira, "melhor_nota") == "80.00");
    CHECK(Coluna(cab, primeira, "nota_media") == "51.25");
    CHECK(r.size() == Campos(primeira).size());
}

TEST_CASE("Tentativas: visitante nao conta vez nem entra no resumo, e lixo nao se le") {
    Tentativa t = UmaDupla();
    t.jogadores.resize(1);
    t.jogadores[0].matricula = "visitante";
    t.jogadores[0].visitante = true;
    std::string csv = CabecalhoDasTentativas() + "\n";
    for (const auto& l : LinhasDaTentativa(t, {0})) csv += l + "\n";
    const auto linhas = Ler(csv);
    REQUIRE(linhas.size() == 1u);
    CHECK(linhas[0].visitante);
    CHECK(ProximaVez(linhas, "visitante", "INF213") == 1);
    const std::string resumo = Resumo(linhas);
    CHECK(resumo.find("visitante") == std::string::npos);

    CHECK(Ler("").empty());
    CHECK(Ler("uma,coisa,qualquer\n1,2,3\n").empty());
    // UMA LINHA CORTADA no fim do arquivo (o jogo fechou no meio) nao derruba
    // a leitura: ela e pulada.
    CHECK(Ler(CabecalhoDasTentativas() + "\n1,abc\n").empty());
}
