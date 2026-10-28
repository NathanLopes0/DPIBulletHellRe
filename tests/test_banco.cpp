//
// A tabela "alunos" do Thiago: o que esta escrito no campo, e as consultas.
//

#include "doctest.h"

#include <algorithm>
#include <cstdlib>
#include <set>

#include "../Source/Attacks/Banco.h"
#include "../Source/Variacao.h"
#include "../Source/Dificuldade.h"

namespace {
    Banco::Registros Exemplo() {
        //            mat  nome curso nota ano
        return { {12, 0, 1, 72, 1},     // Ana  INF 72 1
                 {45, 3, 2, 38, 2},     // Davi MAT 38 2
                 {30, 1, 1, 55, 2},     // Bia  INF 55 2
                 {81, 6, 0, 91, 4} };   // Iara FIS 91 4
    }
}

TEST_CASE("Banco: o sorteio da quatro registros com matricula, nome e nota diferentes") {
    for (std::uint32_t s = 0; s < 500; ++s) {
        const auto regs = Banco::Sortear(s * 2654435761u);
        CAPTURE(s);
        REQUIRE(static_cast<int>(regs.size()) == Banco::kLinhas);
        std::set<int> mats, nomes, notas;
        for (const auto& r : regs) {
            CHECK(r.mat >= 10);
            CHECK(r.mat <= 99);
            CHECK(r.nota >= 0);
            CHECK(r.nota <= 99);
            CHECK(r.ano >= 1);
            CHECK(r.ano <= Banco::kMaiorAno);
            CHECK(r.curso >= 0);
            CHECK(r.curso < static_cast<int>(Banco::kCursos.size()));
            mats.insert(r.mat);
            nomes.insert(r.nome);
            notas.insert(r.nota);
        }
        CHECK(mats.size() == regs.size());
        CHECK(nomes.size() == regs.size());
        CHECK(notas.size() == regs.size());
    }
    const auto a = Banco::Sortear(7u);
    const auto b = Banco::Sortear(7u);
    for (size_t i = 0; i < a.size(); ++i) CHECK(a[i].mat == b[i].mat);
}

TEST_CASE("Banco: o que esta escrito em cada casa, e a cara dela na folha") {
    const auto regs = Exemplo();
    using C = Banco::Coluna;
    CHECK(Banco::Texto(regs[0], C::Mat) == "12");
    CHECK(Banco::Texto(regs[0], C::Nome) == "Ana");
    CHECK(Banco::Texto(regs[0], C::Curso) == "INF");
    CHECK(Banco::Texto(regs[1], C::Nota) == "38");
    CHECK(Banco::Texto(regs[3], C::Ano) == "4");
    CHECK(Banco::Cara(regs[1], C::Nota) == "V38");
    CHECK(Banco::Cara(regs[3], C::Nome) == "Iara");
    CHECK(Banco::Cara(regs[3], C::Curso) == "FIS");
    CHECK(Banco::CaraDoCabecalho(C::Nota) == "Cnota");
}

TEST_CASE("Banco: a condicao le a casa, e o texto e o do WHERE") {
    const auto regs = Exemplo();
    using C = Banco::Coluna;
    using O = Banco::Op;
    CHECK(Banco::Linhas(regs, {C::Nota, O::Menor, 60}) == std::vector<int>{1, 2});
    CHECK(Banco::Linhas(regs, {C::Nota, O::MaiorOuIgual, 72}) == std::vector<int>{0, 3});
    CHECK(Banco::Linhas(regs, {C::Curso, O::Igual, 1}) == std::vector<int>{0, 2});
    CHECK(Banco::Linhas(regs, {C::Ano, O::Igual, 2}) == std::vector<int>{1, 2});
    CHECK(Banco::Linhas(regs, {C::Nome, O::Igual, 6}) == std::vector<int>{3});
    CHECK(Banco::TextoDaCondicao({C::Nota, O::Menor, 60}) == "nota < 60");
    CHECK(Banco::TextoDaCondicao({C::Nota, O::MaiorOuIgual, 70}) == "nota >= 70");
    CHECK(Banco::TextoDaCondicao({C::Curso, O::Igual, 2}) == "curso = 'MAT'");
    CHECK(Banco::TextoDaCondicao({C::Nome, O::Igual, 0}) == "nome = 'Ana'");
    CHECK(Banco::TextoDaCondicao({C::Ano, O::Igual, 3}) == "ano = 3");
}

TEST_CASE("Banco: as casas de cada forma de consulta") {
    const auto regs = Exemplo();
    using C = Banco::Coluna;
    using O = Banco::Op;
    using T = Banco::Tipo;

    const auto selecao = Banco::Casas(regs, {T::Selecao, C::Mat, {C::Nota, O::Menor, 60}});
    CHECK(selecao.size() == 2 * Banco::kColunas);
    for (const auto& c : selecao) CHECK((c.linha == 1 || c.linha == 2));

    const auto projecao = Banco::Casas(regs, {T::Projecao, C::Curso, {}});
    REQUIRE(projecao.size() == regs.size());
    for (const auto& c : projecao) CHECK(c.coluna == static_cast<int>(C::Curso));

    const auto ambas = Banco::Casas(regs, {T::Ambas, C::Nome, {C::Ano, O::Igual, 2}});
    CHECK(ambas == std::vector<Banco::Casa>{{1, 1}, {2, 1}});

    CHECK(Banco::Casas(regs, {T::Ordenacao, C::Nota, {}}).empty());
}

TEST_CASE("Banco: o SQL que o placar escreve") {
    using C = Banco::Coluna;
    using O = Banco::Op;
    using T = Banco::Tipo;
    CHECK(Banco::Sql({T::Selecao, C::Mat, {C::Nota, O::Menor, 50}}) == "SELECT * WHERE nota < 50");
    CHECK(Banco::Sql({T::Projecao, C::Nota, {}}) == "SELECT nota");
    CHECK(Banco::Sql({T::Ambas, C::Nome, {C::Curso, O::Igual, 2}}) == "SELECT nome WHERE curso = 'MAT'");
    CHECK(Banco::Sql({T::Ordenacao, C::Ano, {}}) == "ORDER BY ano");
}

TEST_CASE("Banco: as consultas da fase 1 seguem o ciclo, e sempre sobra onde ficar") {
    using T = Banco::Tipo;
    const std::vector<T> ciclo = {T::Selecao, T::Projecao, T::Ambas, T::Selecao, T::Projecao, T::Ordenacao};
    for (std::uint32_t s = 0; s < 400; ++s) {
        const auto regs = Banco::Sortear(s * 7919u + 3u);
        for (int vez = 0; vez < 12; ++vez) {
            CAPTURE(s);
            CAPTURE(vez);
            const auto c = Banco::Proxima(regs, s * 31u + static_cast<std::uint32_t>(vez), vez);
            CHECK(c.tipo == ciclo[static_cast<size_t>(vez % 6)]);
            const auto linhas = Banco::Linhas(regs, c.onde);
            if (c.tipo == T::Selecao) {
                // UMA LINHA PELO MENOS, e nunca todas: sempre ha onde ficar.
                CHECK(!linhas.empty());
                CHECK(linhas.size() < regs.size());
            }
            if (c.tipo == T::Ambas) {
                CHECK(!linhas.empty());
                CHECK(linhas.size() <= 2);
            }
            if (c.tipo == T::Ordenacao) {
                const auto col = c.coluna;
                const bool emOrdem = std::is_sorted(regs.begin(), regs.end(), [col](const auto& a, const auto& b) {
                    return Banco::Valor(a, col) < Banco::Valor(b, col);
                });
                CHECK_FALSE(emOrdem);
            }
            CHECK(static_cast<int>(c.coluna) >= 0);
            CHECK(static_cast<int>(c.coluna) < Banco::kColunas);
        }
    }
}

TEST_CASE("Banco: o ORDER BY ordena, e diz de onde veio cada registro") {
    auto regs = Exemplo();
    const auto antes = Banco::Ordenar(regs, Banco::Coluna::Nota);
    CHECK(antes == std::vector<int>{1, 2, 0, 3});
    CHECK(regs[0].nota == 38);
    CHECK(regs[3].nota == 91);

    // ESTAVEL: dois do mesmo ano ficam na ordem em que estavam.
    auto outros = Exemplo();
    Banco::Ordenar(outros, Banco::Coluna::Ano);
    CHECK(outros[1].mat == 45);
    CHECK(outros[2].mat == 30);

    // Pelo nome e pela ordem alfabetica, porque kNomes esta em ordem.
    auto porNome = Exemplo();
    Banco::Ordenar(porNome, Banco::Coluna::Nome);
    CHECK(Banco::Texto(porNome[0], Banco::Coluna::Nome) == "Ana");
    CHECK(Banco::Texto(porNome[1], Banco::Coluna::Nome) == "Bia");
    CHECK(Banco::Texto(porNome[2], Banco::Coluna::Nome) == "Davi");
    CHECK(Banco::Texto(porNome[3], Banco::Coluna::Nome) == "Iara");
    CHECK(std::is_sorted(Banco::kNomes.begin(), Banco::kNomes.end(), [](const char* a, const char* b) {
        return std::string(a) < std::string(b);
    }));
    CHECK(std::is_sorted(Banco::kCursos.begin(), Banco::kCursos.end(), [](const char* a, const char* b) {
        return std::string(a) < std::string(b);
    }));
}

TEST_CASE("Banco: todo plano do produto cartesiano passa por toda casa, uma vez") {
    for (int k = 0; k < Banco::kPlanos; ++k) {
        const auto plano = static_cast<Banco::Plano>(k);
        CAPTURE(k);
        const auto casas = Banco::Produto(plano);
        REQUIRE(static_cast<int>(casas.size()) == Banco::kLinhas * Banco::kColunas);
        std::set<std::pair<int, int>> vistas;
        for (const auto& c : casas) {
            CHECK(c.linha >= 0);
            CHECK(c.linha < Banco::kLinhas);
            CHECK(c.coluna >= 0);
            CHECK(c.coluna < Banco::kColunas);
            vistas.insert({c.linha, c.coluna});
        }
        CHECK(static_cast<int>(vistas.size()) == Banco::kLinhas * Banco::kColunas);
        // O LACO ANDA DE VIZINHA EM VIZINHA, menos na virada do laco de fora:
        // e o que deixa o plano ser seguido com os olhos.
        int saltos = 0;
        for (size_t i = 1; i < casas.size(); ++i) {
            const int d = std::abs(casas[i].linha - casas[i - 1].linha) + std::abs(casas[i].coluna - casas[i - 1].coluna);
            if (d != 1) ++saltos;
        }
        CHECK(saltos <= std::max(Banco::kLinhas, Banco::kColunas));
    }
    CHECK(Banco::Produto(Banco::Plano::PorLinha).front() == Banco::Casa{0, 0});
    CHECK(Banco::Produto(Banco::Plano::PorColuna)[1] == Banco::Casa{1, 0});
    CHECK(Banco::Produto(Banco::Plano::DeTrasParaFrente).front() == Banco::Casa{Banco::kLinhas - 1, Banco::kColunas - 1});
    CHECK(Banco::Produto(Banco::Plano::ZigueZague)[Banco::kColunas] == Banco::Casa{1, Banco::kColunas - 1});
}

TEST_CASE("Banco: as voltas do produto seguem os quatro planos, cada uma mais depressa") {
    for (int vez = 0; vez < 12; ++vez) {
        CHECK(static_cast<int>(Banco::PlanoDaVolta(vez)) == vez % Banco::kPlanos);
        CHECK(Banco::PassoDoProduto(vez + 1) <= Banco::PassoDoProduto(vez));
        CHECK(Banco::PassoDoProduto(vez) >= Banco::kPassoMinimoDoProduto);
    }
    CHECK(Banco::PassoDoProduto(0) == doctest::Approx(Banco::kPassoDoProduto));
    CHECK(Banco::PassoDoProduto(100) == doctest::Approx(Banco::kPassoMinimoDoProduto));
    CHECK(Banco::PlacarDoProduto(Banco::Plano::PorLinha) == "JOIN sem WHERE: por linha");
    CHECK(Banco::PlacarDoProduto(Banco::Plano::PorColuna) == "JOIN sem WHERE: por coluna");
    CHECK(Banco::PlacarDoProduto(Banco::Plano::ZigueZague) == "JOIN sem WHERE: em zigue-zague");
}

TEST_CASE("Banco: os comandos da fase 2 nunca esvaziam nem estouram a tabela") {
    using C = Banco::Comando;
    for (std::uint32_t s = 0; s < 200; ++s) {
        auto regs = Banco::Sortear(s * 131u + 7u);
        for (int vez = 0; vez < 40; ++vez) {
            CAPTURE(s);
            CAPTURE(vez);
            const int n = static_cast<int>(regs.size());
            const auto e = Banco::ProximaEscrita(regs, s * 1000u + static_cast<std::uint32_t>(vez));
            if (n <= 2) CHECK(e.comando == C::Insert);
            if (n >= Banco::kLinhas) CHECK(e.comando != C::Insert);
            const auto linhas = Banco::Linhas(regs, e.onde);
            if (e.comando == C::Delete) {
                CHECK(!linhas.empty());
                CHECK(static_cast<int>(linhas.size()) < n);
            }
            if (e.comando == C::Update) {
                CHECK(!linhas.empty());
                CHECK(linhas.size() <= 2);
            }
            if (e.comando == C::Insert) {
                // O REGISTRO NOVO NAO REPETE NINGUEM.
                for (const auto& r : regs) {
                    CHECK(r.mat != e.novo.mat);
                    CHECK(r.nome != e.novo.nome);
                    CHECK(r.nota != e.novo.nota);
                }
            }
            const auto mexidas = Banco::Executar(regs, e);
            CHECK(!mexidas.empty());
            CHECK(!regs.empty());
            CHECK(static_cast<int>(regs.size()) <= Banco::kLinhas);
        }
    }
}

TEST_CASE("Banco: o que cada comando faz com a tabela, e o SQL dele") {
    using C = Banco::Coluna;
    using O = Banco::Op;
    auto regs = Exemplo();

    Banco::Escrita apaga;
    apaga.comando = Banco::Comando::Delete;
    apaga.onde = {C::Nota, O::Menor, 60};
    CHECK(Banco::Sql(apaga) == "DELETE WHERE nota < 60");
    CHECK(Banco::Executar(regs, apaga) == std::vector<int>{1, 2});
    REQUIRE(regs.size() == 2);
    CHECK(regs[0].mat == 12);
    CHECK(regs[1].mat == 81);   // a de baixo subiu

    Banco::Escrita muda;
    muda.comando = Banco::Comando::Update;
    muda.onde = {C::Curso, O::Igual, 0};
    muda.nota = 5;
    CHECK(Banco::Sql(muda) == "UPDATE SET nota = 5 WHERE curso = 'FIS'");
    CHECK(Banco::Executar(regs, muda) == std::vector<int>{1});
    CHECK(regs[1].nota == 5);

    Banco::Escrita poe;
    poe.comando = Banco::Comando::Insert;
    poe.novo = {50, 4, 2, 66, 3};
    CHECK(Banco::Sql(poe) == "INSERT INTO alunos: Eva");
    CHECK(Banco::Executar(regs, poe) == std::vector<int>{2});
    CHECK(regs.back().mat == 50);
}

TEST_CASE("Banco: o registro novo nunca repete ninguem da tabela") {
    for (std::uint32_t s = 0; s < 300; ++s) {
        const auto regs = Banco::Sortear(s);
        const auto r = Banco::NovoRegistro(Banco::Registros(regs.begin(), regs.begin() + 3), s * 17u + 1u);
        for (size_t i = 0; i < 3; ++i) {
            CHECK(regs[i].mat != r.mat);
            CHECK(regs[i].nome != r.nome);
            CHECK(regs[i].nota != r.nota);
        }
    }
}

TEST_CASE("Banco: o ciclo de consultas muda de ordem a cada luta, com as mesmas consultas e a selecao abrindo") {
    using T = Banco::Tipo;
    const auto regs = Exemplo();
    std::set<std::vector<int>> tipos;
    for (std::uint32_t semente = 0; semente < 1000; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, Banco::kFaseDaSelecao);
        const auto ordem = Variacao::Ordem(s, Banco::kConsultasNoCiclo);
        std::vector<int> estaLuta;
        for (int volta = 0; volta < 2; ++volta) {
            int selecoes = 0, projecoes = 0, ambas = 0, ordenacoes = 0;
            for (int k = 0; k < Banco::kConsultasNoCiclo; ++k) {
                const int vez = volta * Banco::kConsultasNoCiclo + k;
                const auto c = Banco::Proxima(regs, semente + static_cast<std::uint32_t>(vez), Variacao::NaOrdem(ordem, vez));
                if (k == 0) CHECK(c.tipo == T::Selecao);   // o SELECT ... WHERE abre cada volta
                selecoes += c.tipo == T::Selecao;
                projecoes += c.tipo == T::Projecao;
                ambas += c.tipo == T::Ambas;
                ordenacoes += c.tipo == T::Ordenacao;
                if (volta == 0) estaLuta.push_back(static_cast<int>(c.tipo));
            }
            // AS MESMAS SEIS de sempre, em outra ordem.
            CHECK(selecoes == 2);
            CHECK(projecoes == 2);
            CHECK(ambas == 1);
            CHECK(ordenacoes == 1);
        }
        tipos.insert(estaLuta);
    }
    // DEPOIS DA SELECAO QUE ABRE vem uma selecao, duas projecoes, as duas e o
    // ORDER BY: 5! / 2! = 60 sequencias, e em 1000 sementes a chance de faltar
    // alguma e de 60 * (59/60)^1000, menos de uma em cem mil.
    CHECK(tipos.size() == 60u);
}

TEST_CASE("Banco: os planos do produto mudam de ordem a cada luta, o por linha abrindo") {
    std::set<std::vector<int>> vistas;
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, Banco::kFaseDoProduto);
        const auto ordem = Variacao::Ordem(s, Banco::kPlanos);
        std::vector<int> planos;
        for (int vez = 0; vez < 2 * Banco::kPlanos; ++vez) {
            planos.push_back(static_cast<int>(Banco::PlanoDaVolta(Variacao::NaOrdem(ordem, vez))));
        }
        CHECK(planos[0] == static_cast<int>(Banco::Plano::PorLinha));
        std::vector<int> primeira(planos.begin(), planos.begin() + Banco::kPlanos);
        std::vector<int> ordenada = primeira;
        std::sort(ordenada.begin(), ordenada.end());
        CHECK(ordenada == std::vector<int>{0, 1, 2, 3});   // os quatro, uma vez cada
        CHECK(std::vector<int>(planos.begin() + Banco::kPlanos, planos.end()) == primeira);
        vistas.insert(primeira);
    }
    CHECK(vistas.size() == 6u);   // 3! depois do por linha
}

TEST_CASE("Banco: no Dificil o WHERE da selecao pega pelo menos duas linhas, e ainda sobra onde ficar") {
    using T = Banco::Tipo;
    int duas = 0, tres = 0;
    for (std::uint32_t s = 0; s < 400; ++s) {
        const auto regs = Banco::Sortear(s * 7919u + 3u);
        for (int vez = 0; vez < 12; ++vez) {
            CAPTURE(s);
            CAPTURE(vez);
            const std::uint32_t sorteio = s * 31u + static_cast<std::uint32_t>(vez);
            // O NORMAL E O DE SEMPRE: o minimo de uma linha e a consulta antiga.
            const auto sempre = Banco::Proxima(regs, sorteio, vez);
            const auto normal = Banco::Proxima(regs, sorteio, vez, 1);
            CHECK(Banco::Sql(normal) == Banco::Sql(sempre));
            const auto c = Banco::Proxima(regs, sorteio, vez, 2);
            CHECK(c.tipo == sempre.tipo);
            if (c.tipo != T::Selecao) continue;
            const auto linhas = Banco::Linhas(regs, c.onde);
            CHECK(linhas.size() >= 2u);
            CHECK(linhas.size() < regs.size());
            duas += linhas.size() == 2u;
            tres += linhas.size() == 3u;
        }
    }
    CHECK(duas > 0);
    CHECK(tres > 0);
    // COM POUCAS LINHAS o minimo cede: nunca pede todas. (As duas notas abaixo
    // de 50: a condicao de reserva, "nota < 50", pegaria as duas.)
    const Banco::Registros duasLinhas = {{12, 0, 1, 30, 1}, {45, 3, 2, 38, 2}};
    for (std::uint32_t s = 0; s < 50; ++s) {
        const auto c = Banco::Proxima(duasLinhas, s, 0, 2);
        CHECK(Banco::Linhas(duasLinhas, c.onde).size() == 1u);
    }
}

TEST_CASE("Banco: o degrau do Normal e o de sempre, e o do Dificil aperta cada relogio") {
    const Banco::Degrau n = Banco::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.passo == Banco::kPasso);
    CHECK(n.avisoDaLinha == Banco::kAvisoDaLinha);
    CHECK(n.avisoDaColuna == Banco::kAvisoDaColuna);
    CHECK(n.avisoDaCasa == Banco::kAvisoDaCasa);
    CHECK(n.velocidadeDaLinha == Banco::kVelocidadeDaLinha);
    CHECK(n.velocidadeDaColuna == Banco::kVelocidadeDaColuna);
    CHECK(n.passoDaEscrita == Banco::kPassoDaEscrita);
    CHECK(n.velocidadeDoApagado == Banco::kVelocidadeDoApagado);
    CHECK(n.avisoDoProduto == Banco::kAvisoDoProduto);
    CHECK(n.passoDoProduto == 1.0f);
    CHECK(n.linhasDaSelecao == 1);

    const Banco::Degrau d = Banco::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.passo < n.passo);
    CHECK(d.avisoDaLinha < n.avisoDaLinha);
    CHECK(d.avisoDaColuna < n.avisoDaColuna);
    CHECK(d.avisoDaCasa < n.avisoDaCasa);
    CHECK(d.velocidadeDaLinha > n.velocidadeDaLinha);
    CHECK(d.velocidadeDaColuna > n.velocidadeDaColuna);
    CHECK(d.passoDaEscrita < n.passoDaEscrita);
    CHECK(d.velocidadeDoApagado > n.velocidadeDoApagado);
    CHECK(d.avisoDoProduto < n.avisoDoProduto);
    CHECK(d.passoDoProduto < 1.0f);
    CHECK(d.linhasDaSelecao == 2);
    // O AVISO AINDA VEM ANTES DA PROXIMA CONSULTA: as casas de uma apagam
    // antes de a seguinte acender - a mesma leitura, so que mais depressa.
    CHECK(d.avisoDaLinha < d.passo);
    CHECK(d.avisoDaColuna < d.passo);
    CHECK(d.avisoDaCasa < d.passo);
    // E NENHUMA FAIXA FICA INESCAPAVEL (o cabecalho de thiago/fases.json): no
    // campo de 1200 x 746 e a 300 px/s, atravessar uma linha inteira custa
    // 0,62 s e uma coluna 0,8 s - o aviso do Dificil ainda e maior que isso.
    constexpr float kAndar = 300.0f;
    CHECK(d.avisoDaLinha > 746.0f / Banco::kLinhas / kAndar);
    CHECK(d.avisoDaColuna > 1200.0f / Banco::kColunas / kAndar);
}
