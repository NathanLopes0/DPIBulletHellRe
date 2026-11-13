//
// O Arroyo (INF 332 - Projeto e Analise de Algoritmos): a arvore de recursao, os paradigmas, a rede
// de ordenacao, o caixeiro e a tempera simulada.
//

#include "doctest.h"

#include <algorithm>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "../Source/Attacks/Intratavel.h"
#include "../Source/Attacks/Paradigmas.h"
#include "../Source/Attacks/Recorrencia.h"
#include "../Source/Attacks/Tempera.h"

// ---- Fase 1: a arvore de recursao ----

namespace {
    Recorrencia::Caso Achar(const std::string& algoritmo) {
        for (const auto& c : Recorrencia::Casos()) if (algoritmo == c.algoritmo) return c;
        FAIL("recorrencia que nao existe: " << algoritmo);
        return {};
    }
}

TEST_CASE("Recorrencia: o Teorema Mestre nos tres casos") {
    using namespace Recorrencia;
    CHECK(Solucao(Achar("merge sort")) == "\xce\x98(n log n)");                       // caso 2
    CHECK(Solucao(Achar("busca bin\xc3\xa1ria")) == "\xce\x98(log n)");              // caso 2, k = 0
    CHECK(Solucao(Achar("Karatsuba")) == "\xce\x98(n^1,58)");                        // caso 1
    CHECK(Solucao(Achar("percurso em \xc3\xa1rvore")) == "\xce\x98(n)");             // caso 1
    CHECK(Solucao(Achar("sele\xc3\xa7\xc3\xa3o linear")) == "\xce\x98(n)");          // caso 3
    CHECK(Solucao(Achar("multiplica\xc3\xa7\xc3\xa3o ing\xc3\xaanua")) == "\xce\x98(n\xc2\xb2)");
    CHECK(Solucao(Achar("pares mais distantes")) == "\xce\x98(n\xc2\xb2)");          // caso 3
    CHECK(Solucao(Achar("merge sort tern\xc3\xa1rio")) == "\xce\x98(n log n)");
}

TEST_CASE("Recorrencia: a formula, o rotulo e a cara de cada no") {
    using namespace Recorrencia;
    CHECK(Formula(Achar("merge sort")) == "T(n) = 2T(n/2) + n");
    CHECK(Formula(Achar("busca bin\xc3\xa1ria")) == "T(n) = T(n/2) + 1");
    CHECK(Placar(Achar("merge sort"), true) == "T(n) = 2T(n/2) + n = \xce\x98(n log n)");
    const Caso ms = Achar("merge sort");
    CHECK(Rotulo(ms, 0) == "n");
    CHECK(Rotulo(ms, 2) == "n/4");
    CHECK(Rotulo(ms, Niveis(ms)) == "1");
    CHECK(Cara(ms, 1) == "N2");
    CHECK(Cara(ms, Niveis(ms)) == "Um");
    CHECK(Cara(Achar("merge sort tern\xc3\xa1rio"), 1) == "N3");
    CHECK(Resistencia(0) > Resistencia(1));
    CHECK(Resistencia(2) == 1);
}

TEST_CASE("Recorrencia: o trabalho de cada nivel conta a historia do Teorema Mestre") {
    using namespace Recorrencia;
    auto doNivel = [](const Caso& c, const int nivel) {
        int nos = 1;
        for (int i = 0; i < nivel; ++i) nos *= c.a;
        return nos * Trabalho(c, nivel);
    };
    // Merge sort: todo nivel custa o mesmo.
    const Caso ms = Achar("merge sort");
    for (int d = 0; d < Niveis(ms); ++d) CHECK(doNivel(ms, d) == kN);
    // Karatsuba: o custo cresce para as folhas.
    const Caso ka = Achar("Karatsuba");
    CHECK(doNivel(ka, 1) > doNivel(ka, 0));
    // Selecao linear: o custo cai, e a raiz domina.
    const Caso se = Achar("sele\xc3\xa7\xc3\xa3o linear");
    CHECK(doNivel(se, 1) < doNivel(se, 0));
    // Busca binaria: um tiro por nivel.
    const Caso bb = Achar("busca bin\xc3\xa1ria");
    for (int d = 0; d < Niveis(bb); ++d) CHECK(doNivel(bb, d) == 1);
    // Nenhuma arvore passa de 16 folhas.
    for (const Caso& c : Casos()) {
        int folhas = 1;
        for (int i = 0; i < Niveis(c); ++i) folhas *= c.a;
        CHECK(folhas <= 16);
        CHECK(Desvios(c.a).size() == static_cast<size_t>(c.a));
    }
    CHECK(Desvios(1) == std::vector<float>{0.0f});
}

TEST_CASE("Recorrencia: a ordem da luta tem o merge sort primeiro e nao repete") {
    Variacao::Sorteio s(17u);
    const auto ordem = Recorrencia::Sortear(s);
    REQUIRE(ordem.size() == static_cast<size_t>(Recorrencia::kRodadas));
    CHECK(ordem.front() == 0);
    CHECK(std::set<int>(ordem.begin(), ordem.end()).size() == ordem.size());
    const auto difi = Recorrencia::DegrauDo(Dificuldade::Nivel::Dificil);
    const auto normal = Recorrencia::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(difi.porNivel < normal.porNivel);
}

// ---- Fase 2: os paradigmas ----

TEST_CASE("Paradigmas: o troco guloso acerta no real e erra nos sistemas ruins") {
    using namespace Paradigmas;
    const auto& t = Trocos();
    REQUIRE(t.size() == 3u);
    CHECK(Guloso(t[0]) == std::vector<int>{100, 100, 100, 50, 25, 10});
    CHECK(Otimo(t[0]) == 6);
    CHECK(Guloso(t[1]) == std::vector<int>{25, 1, 1, 1, 1, 1, 1});
    CHECK(Otimo(t[1]) == 4);
    CHECK(Guloso(t[2]) == std::vector<int>{9, 1, 1, 1});
    CHECK(Otimo(t[2]) == 2);
    CHECK(PlacarDoTroco(t[1]) == "Troco de 31: 25+1+1+1+1+1+1  (\xc3\xb3timo: 4 moedas)");
    // As moedas de um troco cabem no tempo dele.
    for (const auto& troco : t) CHECK(0.25f + kEntreMoedas * static_cast<float>(Guloso(troco).size()) < kDuracaoDoTroco);
    CHECK(PlacarDoTroco(t[0]).find("\xc3\xb3timo!") != std::string::npos);
    CHECK(EscalaDaMoeda(100) > EscalaDaMoeda(1));
    CHECK(CaraDaMoeda(25) == "M25");
    // Os tres trocos cabem na etapa.
    CHECK(kDuracaoDoTroco * 3.0f <= kDuracao[0] + 1e-4f);
}

TEST_CASE("Paradigmas: a tabela da subsequencia comum e as ondas") {
    using namespace Paradigmas;
    const std::string a = kPalavraA, b = kPalavraB;
    const auto t = Tabela(a, b);
    CHECK(t[a.size()][b.size()] == 7);   // LGRITMO
    CHECK(Casou(a, b, 1, 4));            // o A de ALGORITMO e o A de LOGARITMO
    CHECK_FALSE(Casou(a, b, 1, 1));
    const auto ondas = Ondas(9, 9);
    CHECK(ondas.size() == 17u);
    size_t casas = 0;
    for (size_t d = 0; d < ondas.size(); ++d) {
        for (const auto& [i, j] : ondas[d]) {
            CHECK(i + j == static_cast<int>(d) + 2);
            ++casas;
        }
    }
    CHECK(casas == 81u);
    CHECK(CaraDaCasa(3, false) == "V3");
    CHECK(CaraDaCasa(7, true) == "C7");
    // A onda inteira cabe na etapa, com folga.
    const auto d = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(d.porOnda * static_cast<float>(ondas.size()) < kDuracao[1]);
}

TEST_CASE("Paradigmas: a busca das seis rainhas acha a primeira solucao em 56 passos") {
    using namespace Paradigmas;
    const auto busca = Busca(kRainhas);
    CHECK(busca.size() == 56u);
    CHECK(std::count_if(busca.begin(), busca.end(), [](const Passo& p) { return p.coloca; }) == 31);
    const auto fim = NoTabuleiro(busca, static_cast<int>(busca.size()), kRainhas);
    CHECK(fim == std::vector<int>{1, 3, 5, 0, 2, 4});
    // Nenhuma rainha ataca outra no fim.
    for (int i = 0; i < kRainhas; ++i)
        for (int j = i + 1; j < kRainhas; ++j) {
            CHECK(fim[static_cast<size_t>(i)] != fim[static_cast<size_t>(j)]);
            CHECK(std::abs(fim[static_cast<size_t>(i)] - fim[static_cast<size_t>(j)]) != j - i);
        }
    // A busca inteira cabe na etapa.
    const auto d = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(d.porPasso * static_cast<float>(busca.size()) < kDuracao[2] - 0.3f);
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(7.0f) == 1);
    CHECK(EtapaNoTempo(19.0f) == 2);
}

// ---- Fase 3: os limites inferiores e a NP-completude ----

TEST_CASE("Intratavel: a rede ordena todas as permutacoes de 6, com 12 comparadores") {
    using namespace Intratavel;
    CHECK(Comparadores() == 12);
    CHECK(LimiteInferior(6) == 10);
    CHECK(LimiteInferior(6) <= Comparadores());
    std::vector<int> v = {0, 1, 2, 3, 4, 5};
    int todas = 0;
    do {
        CHECK(Ordenar(v) == std::vector<int>{0, 1, 2, 3, 4, 5});
        ++todas;
    } while (std::next_permutation(v.begin(), v.end()));
    CHECK(todas == 720);
    for (const auto& camada : Camadas()) {
        std::set<int> fios;
        for (const auto& [i, j] : camada) {
            CHECK(i < j);
            CHECK(fios.insert(i).second);   // um fio so entra uma vez por camada
            CHECK(fios.insert(j).second);
        }
    }
    CHECK(XDoFio(0, 0.0f, 600.0f) < XDoFio(5, 0.0f, 600.0f));
    // log2 720 = 9,49...: arredonda para 10. Sem o sinal de teto, que a fonte do quadro nao tem.
    CHECK(PlacarDaRede() == "\xce\xa9(n log n): log\xe2\x82\x82 6! \xe2\x89\x88 9,5 \xe2\x86\x92 10  \xc2\xb7  a rede usa 12");
    CHECK(PlacarDaRede().find("\xe2\x8c\x88") == std::string::npos);
}

TEST_CASE("Intratavel: o caixeiro conta as rotas e anda na rota") {
    using namespace Intratavel;
    CHECK(Rotas(5) == 12);
    CHECK(Rotas(6) == 60);
    CHECK(Rotas(7) == 360);
    Variacao::Sorteio s(5u);
    const auto c = Cidades(s, 6, 100.0f, 100.0f, 800.0f, 400.0f);
    REQUIRE(c.size() == 6u);
    for (const auto& p : c) {
        CHECK(p.x >= 100.0f);
        CHECK(p.x <= 900.0f);
    }
    const auto rota = Tentativa(s, 6);
    CHECK(rota.front() == 0);
    std::vector<int> ordenada = rota;
    std::sort(ordenada.begin(), ordenada.end());
    CHECK(ordenada == std::vector<int>{0, 1, 2, 3, 4, 5});
    // O caixeiro comeca e termina na cidade 0, e no meio esta em algum trecho.
    CHECK((NaRota(c, rota, 0.0f) - c[0]).Length() < 1e-3f);
    CHECK((NaRota(c, rota, 1.0f) - c[0]).Length() < 1e-2f);
    // Um quadrado de lado 1: a volta mede 4.
    const std::vector<Vector2> q = {Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1)};
    CHECK(Comprimento(q, {0, 1, 2, 3}) == doctest::Approx(4.0f));
    CHECK(Comprimento(q, {0, 2, 1, 3}) > 4.0f);
    CHECK(PlacarDoCaixeiro(6, 3, 1840.4f) == "n = 6  \xc2\xb7  60 rotas  \xc2\xb7  tentativa 3  \xc2\xb7  melhor 1840");
    CHECK(kFimDaRede + kDuracaoDaRodada * 3.0f == doctest::Approx(Fase::kDuracao));
}

// ---- A repescagem: a tempera simulada ----

TEST_CASE("Tempera: esfria, reaquece, e o Metropolis aceita pior so quando quente") {
    using namespace Tempera;
    CHECK(Temperatura(0.0f) == doctest::Approx(kQuente));
    CHECK(Temperatura(kReaquecimento - 0.01f) < 10.0f);
    CHECK(Temperatura(kReaquecimento) == doctest::Approx(kQuente));   // o reaquecimento
    CHECK(Temperatura(3.0f) < Temperatura(1.0f));
    CHECK(Aceita(-5.0f, 1.0f, 0.99f));                 // melhorar sempre passa
    CHECK(Aceita(40.0f, kQuente, 0.5f));               // quente, aceita pior
    CHECK_FALSE(Aceita(40.0f, kFrio, 0.01f));          // frio, nao
    CHECK(ChanceDePior(40.0f, kQuente) > 0.6f);
    CHECK(ChanceDePior(40.0f, kFrio) < 0.01f);
    CHECK(Placar(kQuente).rfind("T = 1000", 0) == 0);
}

TEST_CASE("Tempera: o vale do aluno e o mais fundo, e o passo encolhe no frio") {
    using namespace Tempera;
    const Vector2 aluno(600.0f, 600.0f);
    const std::vector<Vector2> vales = {Vector2(200.0f, 300.0f), Vector2(1000.0f, 300.0f)};
    CHECK(Energia(aluno, aluno, vales) < Energia(vales[0], aluno, vales));
    CHECK(Energia(vales[0], aluno, vales) == doctest::Approx(0.0f));
    Variacao::Sorteio s(3u);
    float quente = 0.0f, frio = 0.0f;
    for (int i = 0; i < 200; ++i) {
        quente += (Proposta(aluno, kQuente, s) - aluno).Length();
        frio += (Proposta(aluno, kFrio, s) - aluno).Length();
    }
    CHECK(quente > 3.0f * frio);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).solucoes > DegrauDo(Dificuldade::Nivel::Normal).solucoes);
}
