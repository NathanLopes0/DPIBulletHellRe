//
// O orientador do TCC: quem pode orientar, e quem a dupla escolhe.
//

#include "doctest.h"

#include <string>

#include "../Source/Materias.h"
#include "../Source/Orientacao.h"
#include "../Source/Progresso.h"

namespace {

    /// Um curso pequeno: cinco materias com professor e chefe, uma sem chefe,
    /// uma do mesmo professor de outra, e o TCC, que pede orientacao.
    const char* kCurso = R"({
      "materias": [
        { "codigo": "P1",  "nome": "P1",  "professor": "Andre",   "chefe": "andre",   "coluna": 0,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "ED",  "nome": "ED",  "professor": "Salles",  "chefe": "salles",  "coluna": 1,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "OC",  "nome": "OC",  "professor": "Ricardo", "chefe": "ricardo", "coluna": 1,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "BD",  "nome": "BD",  "professor": "Thiago",  "chefe": "thiago",  "coluna": 1,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "IA",  "nome": "IA",  "professor": "Julio",   "chefe": "julio",   "coluna": 2,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "SEM", "nome": "SEM", "professor": "Fulano",                       "coluna": 2,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "OC2", "nome": "OC2", "professor": "Ricardo", "chefe": "ricardo", "coluna": 2,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "TCC", "nome": "TCC", "professor": "Hugo",    "chefe": "hugo",    "coluna": 3,
          "orientacao": true, "desbloqueio": { "tipo": "sempre" } }
      ]
    })";

    Progresso Notas(const Materias::Lista& l, const std::vector<std::pair<const char*, float>>& notas) {
        Progresso p;
        for (const auto& [codigo, nota] : notas) p.RegistrarNota(l.IndiceDe(codigo), nota);
        return p;
    }

    std::vector<std::string> Professores(const std::vector<Orientacao::Candidato>& cs) {
        std::vector<std::string> nomes;
        for (const auto& c : cs) nomes.push_back(c.professor);
        return nomes;
    }
}

TEST_CASE("Materias: o TCC pede orientacao, e um valor que nao e booleano vira aviso") {
    const auto l = Materias::LerMaterias(kCurso);
    REQUIRE(l.problemas.empty());
    CHECK(l.Por(l.IndiceDe("TCC"))->orientacao);
    CHECK_FALSE(l.Por(l.IndiceDe("P1"))->orientacao);

    const auto errado = Materias::LerMaterias(R"({ "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "orientacao": "sim", "desbloqueio": { "tipo": "sempre" } } ] })");
    REQUIRE(errado.materias.size() == 1);
    CHECK_FALSE(errado.materias[0].orientacao);
    bool avisou = false;
    for (const auto& p : errado.problemas) if (p.find("orientacao") != std::string::npos) avisou = true;
    CHECK(avisou);
}

TEST_CASE("Orientacao: os tres professores das maiores notas aprovadas, a maior primeiro") {
    const auto l = Materias::LerMaterias(kCurso);
    const auto p = Notas(l, {{"P1", 90.0f}, {"ED", 70.0f}, {"OC", 85.0f}, {"BD", 61.0f}, {"IA", 59.0f}});
    const auto cs = Orientacao::Candidatos(l, {&p});
    REQUIRE(cs.size() == 3);
    CHECK(Professores(cs) == std::vector<std::string>{"Andre", "Ricardo", "Salles"});
    CHECK(cs[0].nota == doctest::Approx(90.0f));
    CHECK(cs[0].chefe == "andre");
    CHECK(cs[1].materia == l.IndiceDe("OC"));
}

TEST_CASE("Orientacao: so aprovada, so com batalha, e nunca a materia que pede orientacao") {
    const auto l = Materias::LerMaterias(kCurso);
    // IA reprovada, SEM sem chefe, TCC pede orientacao: sobram duas.
    const auto p = Notas(l, {{"P1", 80.0f}, {"IA", 59.9f}, {"SEM", 99.0f}, {"TCC", 100.0f}, {"BD", 60.0f}});
    const auto cs = Orientacao::Candidatos(l, {&p});
    CHECK(Professores(cs) == std::vector<std::string>{"Andre", "Thiago"});   // 60 e aprovacao

    Progresso nada;
    CHECK(Orientacao::Candidatos(l, {&nada}).empty());
    CHECK(Orientacao::Candidatos(l, {nullptr}).empty());
}

TEST_CASE("Orientacao: no empate vem a que esta antes no curso, e um professor aparece uma vez so") {
    const auto l = Materias::LerMaterias(kCurso);
    // OC2 e do Ricardo tambem, com nota maior que OC: ele aparece uma vez, pela
    // OC2 - e a OC, que seria a segunda, da o lugar ao Andre.
    const auto p = Notas(l, {{"BD", 80.0f}, {"ED", 80.0f}, {"OC", 85.0f}, {"OC2", 95.0f}, {"P1", 80.0f}});
    const auto cs = Orientacao::Candidatos(l, {&p});
    REQUIRE(cs.size() == 3);
    CHECK(Professores(cs) == std::vector<std::string>{"Ricardo", "Andre", "Salles"});
    CHECK(cs[0].materia == l.IndiceDe("OC2"));
}

TEST_CASE("Orientacao: em dupla a nota de cada materia e a maior dos dois, e se sabe de quem") {
    const auto l = Materias::LerMaterias(kCurso);
    const auto a = Notas(l, {{"P1", 90.0f}, {"ED", 62.0f}});
    const auto b = Notas(l, {{"ED", 88.0f}, {"IA", 75.0f}, {"P1", 90.0f}});
    const auto cs = Orientacao::Candidatos(l, {&a, &b});
    REQUIRE(cs.size() == 3);
    CHECK(Professores(cs) == std::vector<std::string>{"Andre", "Salles", "Julio"});
    CHECK(cs[0].deQuem == 0);                 // empate em 90: o primeiro
    CHECK(cs[1].deQuem == 1);
    CHECK(cs[1].nota == doctest::Approx(88.0f));

    // E um visitante, que nunca jogou, nao tira nada da dupla.
    Progresso visitante;
    CHECK(Professores(Orientacao::Candidatos(l, {&a, &visitante})) == Professores(Orientacao::Candidatos(l, {&a})));
}

TEST_CASE("Orientacao: a votacao - concordando e esse, discordando vence o de nota mais alta") {
    std::vector<Orientacao::Candidato> cs(3);
    CHECK(Orientacao::Resultado(cs, {2}) == 2);          // sozinho, o voto dele
    CHECK(Orientacao::Resultado(cs, {1, 1}) == 1);
    CHECK(Orientacao::Resultado(cs, {2, 0}) == 0);       // o mais acima na lista
    CHECK(Orientacao::Resultado(cs, {1, 2}) == 1);
    CHECK(Orientacao::Resultado(cs, {1, -1}) == -1);     // o segundo ainda nao votou
    CHECK(Orientacao::Resultado(cs, {3}) == -1);         // voto fora da lista
    CHECK(Orientacao::Resultado(cs, {}) == -1);
    CHECK(Orientacao::Resultado({}, {0}) == -1);
}
