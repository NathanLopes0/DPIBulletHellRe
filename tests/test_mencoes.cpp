//
// As mencoes de cada materia.
//

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Aluno/Mencoes.h"
#include "../Source/Aluno/Progresso.h"
#include "../Source/Curso/Materias.h"

using namespace Mencoes;

namespace {
    /// Um curso de tres materias: duas com batalha (a segunda pede orientador) e uma sem.
    Materias::Lista Curso() {
        Materias::Lista lista = Materias::LerMaterias(R"({ "materias": [
            { "codigo": "A1", "nome": "INF 1", "professor": "Ana", "chefe": "ana", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "T1", "nome": "TCC", "professor": "Tom", "chefe": "tom", "coluna": 1, "orientacao": true,
              "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"] } },
            { "codigo": "V1", "nome": "INF 3", "professor": "Vera", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } }
        ] })");
        REQUIRE(lista.Quantas() == 3);
        REQUIRE(lista.Por(1)->orientacao);
        REQUIRE(lista.Por(2)->chefe.empty());
        return lista;
    }
    constexpr int kA = 0, kT = 1, kV = 2;
}

TEST_CASE("Mencoes: quem nunca jogou nao tem nenhuma, e cada materia tem as que existem nela") {
    const Materias::Lista curso = Curso();
    const Progresso p;
    const DaMateria a = Da(p, curso, kA);
    CHECK(a.Ganhas() == 0);
    CHECK(a.Possiveis() == kQuantas);
    // O TCC nao orienta ninguem, e a materia sem batalha nao tem mencao.
    const DaMateria t = Da(p, curso, kT);
    CHECK(t.Possiveis() == kQuantas - 1);
    CHECK_FALSE(t.Existe(Mencao::Orientou));
    CHECK(Da(p, curso, kV).Possiveis() == 0);
    CHECK(Da(p, curso, 99).Possiveis() == 0);
    const Total total = DoCurso(p, curso);
    CHECK(total.ganhas == 0);
    CHECK(total.possiveis == 2 * kQuantas - 1);
}

TEST_CASE("Mencoes: a nota da o bronze, a prata, o ouro e a nota cheia, um degrau de cada vez") {
    const Materias::Lista curso = Curso();
    Progresso p;
    p.RegistrarNota(kA, 55.0f, Dificuldade::Nivel::Normal);
    CHECK(Da(p, curso, kA).Ganhas() == 0);                    // reprovado: nenhuma
    p.RegistrarNota(kA, 60.0f, Dificuldade::Nivel::Normal);
    DaMateria d = Da(p, curso, kA);
    CHECK(d.Tem(Mencao::Bronze));
    CHECK_FALSE(d.Tem(Mencao::Prata));
    p.RegistrarNota(kA, kNotaDaPrata, Dificuldade::Nivel::Normal);
    d = Da(p, curso, kA);
    CHECK(d.Tem(Mencao::Prata));
    CHECK_FALSE(d.Tem(Mencao::Ouro));
    p.RegistrarNota(kA, kNotaDoOuro, Dificuldade::Nivel::Normal);
    d = Da(p, curso, kA);
    CHECK(d.Tem(Mencao::Ouro));
    CHECK_FALSE(d.Tem(Mencao::NotaCheia));
    p.RegistrarNota(kA, 100.0f, Dificuldade::Nivel::Normal);
    d = Da(p, curso, kA);
    CHECK(d.Tem(Mencao::NotaCheia));
    CHECK(d.Ganhas() == 4);
    // A nota pior depois nao tira nada: vale o recorde.
    p.RegistrarNota(kA, 10.0f, Dificuldade::Nivel::Normal);
    CHECK(Da(p, curso, kA).Ganhas() == 4);
}

TEST_CASE("Mencoes: o Dificil, a dupla e o professor que orientou") {
    const Materias::Lista curso = Curso();
    Progresso p;
    p.RegistrarNota(kA, 70.0f, Dificuldade::Nivel::Normal);
    DaMateria antes = Da(p, curso, kA);
    CHECK_FALSE(antes.Tem(Mencao::Dificil));
    CHECK_FALSE(antes.Tem(Mencao::Dupla));

    // Reprovar no Dificil nao da a mencao; passar, da.
    p.RegistrarNota(kA, 40.0f, Dificuldade::Nivel::Dificil);
    CHECK_FALSE(Da(p, curso, kA).Tem(Mencao::Dificil));
    p.RegistrarNota(kA, 61.0f, Dificuldade::Nivel::Dificil);
    CHECK(Da(p, curso, kA).Tem(Mencao::Dificil));

    // A dupla, em qualquer dificuldade - e a dupla no Dificil da as duas.
    Progresso q;
    q.RegistrarNotaDeDupla(kA, 65.0f, Dificuldade::Nivel::Dificil, "123");
    const DaMateria dupla = Da(q, curso, kA);
    CHECK(dupla.Tem(Mencao::Dupla));
    CHECK(dupla.Tem(Mencao::Dificil));
    CHECK(dupla.Tem(Mencao::Bronze));

    // O curriculo de orientacoes: so de materia aprovada, e conta as vezes.
    Progresso r;
    r.RegistrarOrientacao(kA);
    CHECK(r.Orientacoes(kA) == 0);                           // nunca jogou: nao ha quem oriente
    r.RegistrarNota(kA, 50.0f, Dificuldade::Nivel::Normal);
    r.RegistrarOrientacao(kA);
    CHECK(r.Orientacoes(kA) == 0);                           // reprovado tambem nao
    r.RegistrarNota(kA, 75.0f, Dificuldade::Nivel::Normal);
    const DaMateria semOrientar = Da(r, curso, kA);
    r.RegistrarOrientacao(kA);
    r.RegistrarOrientacao(kA);
    CHECK(r.Orientacoes(kA) == 2);
    const DaMateria orientou = Da(r, curso, kA);
    CHECK(orientou.Tem(Mencao::Orientou));
    CHECK(Novas(semOrientar, orientou) == std::vector<Mencao>{Mencao::Orientou});
    // Sobrevive a ficha: Entradas e Restaurar levam a contagem.
    Progresso lido;
    lido.Restaurar(r.Entradas());
    CHECK(lido.Orientacoes(kA) == 2);
}

TEST_CASE("Mencoes: as novas de uma batalha, a frase delas, e os selos") {
    const Materias::Lista curso = Curso();
    Progresso p;
    const DaMateria antes = Da(p, curso, kA);
    p.RegistrarNotaDeDupla(kA, 85.0f, Dificuldade::Nivel::Normal, "9");
    const DaMateria depois = Da(p, curso, kA);
    const std::vector<Mencao> novas = Novas(antes, depois);
    CHECK(novas == std::vector<Mencao>{Mencao::Bronze, Mencao::Prata, Mencao::Dupla});
    CHECK(FraseDasNovas(novas) == "Novas men\xc3\xa7\xc3\xb5" "es: bronze, prata, em dupla");
    CHECK(FraseDasNovas({Mencao::Ouro}) == "Nova men\xc3\xa7\xc3\xa3o: ouro");
    CHECK(FraseDasNovas({}).empty());
    CHECK(Novas(depois, depois).empty());
    const Total total = DoCurso(p, curso);
    CHECK(total.ganhas == 3);

    // Cada selo com o seu texto e a sua cor, e todos na legenda.
    std::set<std::string> selos;
    std::set<int> cores;
    const std::string legenda = Legenda();
    for (const Mencao m : kTodas) {
        selos.insert(Selo(m));
        CHECK(std::string(Nome(m)).size() > 1u);
        const Cor c = CorDa(m);
        cores.insert(c.r * 65536 + c.g * 256 + c.b);
        CHECK(legenda.find(std::string(Selo(m)) + " ") != std::string::npos);
    }
    CHECK(selos.size() == static_cast<size_t>(kQuantas));
    CHECK(cores.size() == static_cast<size_t>(kQuantas));
    CHECK(kNotaDaPrata > Progresso::kNotaDeAprovacao);
    CHECK(kNotaDoOuro > kNotaDaPrata);
}
