//
// As horas complementares.
//

#include "doctest.h"

#include <algorithm>
#include <set>
#include <string>

#include "../Source/Aluno/Horas.h"
#include "../Source/Aluno/Progresso.h"

using namespace Horas;

namespace {
    Materias::Lista Curso() {
        return Materias::LerMaterias(R"({ "materias": [
            { "codigo": "A1", "nome": "INF 1", "professor": "Ana", "chefe": "ana", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "V1", "nome": "INF 3", "professor": "Vera", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "T1", "nome": "TCC", "professor": "Tom", "chefe": "tom", "coluna": 1, "orientacao": true,
              "desbloqueio": { "tipo": "sempre" } }
        ] })");
    }

    bool Tem(const std::vector<std::string>& v, const std::string& chave) {
        return std::find(v.begin(), v.end(), chave) != v.end();
    }

    /// Uma prova aprovada sem nada de especial: muitos acertos, nenhuma fase limpa.
    Feito Comum() {
        Feito f;
        f.materia = "A1";
        f.aprovado = true;
        f.acertosSofridos = 30;
        f.aprovacoesNaMateria = 1;
        f.menorNota = 38.0f;
        return f;
    }
}

TEST_CASE("Horas: cada atividade tem a sua chave, as suas horas e um nome sem professor") {
    std::set<std::string> chaves;
    for (const Dados& d : Todas()) {
        CAPTURE(d.chave);
        CHECK(d.horas > 0);
        CHECK(std::string(d.titulo).size() > 2u);
        CHECK(std::string(d.descricao).size() > 5u);
        CHECK(std::string(d.chave).find('@') == std::string::npos);
        chaves.insert(d.chave);
        // A por materia leva o codigo na chave e o nome da materia na tela - nunca o do professor.
        const std::string chave = Chave(d.atividade, "A1");
        CHECK(chave == (d.porMateria ? std::string(d.chave) + "@A1" : std::string(d.chave)));
        const std::string nome = Nome(chave, Curso());
        CHECK((nome.find("INF 1") != std::string::npos) == d.porMateria);
        CHECK(nome.find("Ana") == std::string::npos);
    }
    CHECK(chaves.size() == Todas().size());
    CHECK(Nome("nao-existe", Curso()).empty());
    CHECK(Nome("virada@A1", Curso()).empty());       // a do curso nao tem materia
    CHECK(Nome("fase-limpa", Curso()).empty());      // e a por materia precisa de uma
    // A meta cabe no curso de verdade (onze materias com batalha) com folga, e nao sobra de graca.
    int porMateria = 0, doCurso = 0;
    for (const Dados& d : Todas()) (d.porMateria ? porMateria : doCurso) += d.horas;
    CHECK(porMateria * 11 + doCurso >= kMeta + 20);
    CHECK(doCurso < kMeta);
    CHECK(Possiveis(Curso()) == porMateria * 2 + doCurso);
}

TEST_CASE("Horas: so quem passou cumpre, e cada feito da a sua atividade") {
    Feito reprovado = Comum();
    reprovado.aprovado = false;
    reprovado.fasesLimpas = 3;
    reprovado.acertosSofridos = 0;
    CHECK(Cumpridas(reprovado).empty());
    CHECK(Cumpridas(Comum()).empty());

    Feito f = Comum();
    f.fasesLimpas = 1;
    CHECK(Cumpridas(f) == std::vector<std::string>{"fase-limpa@A1"});

    f = Comum();
    f.acertosSofridos = kPoucosAcertos;
    CHECK(Tem(Cumpridas(f), "poucos-acertos@A1"));
    f.acertosSofridos = kPoucosAcertos + 1;
    CHECK(Cumpridas(f).empty());

    f = Comum();
    f.aprovacoesNaMateria = kAprovacoesDoVeterano;
    CHECK(Tem(Cumpridas(f), "veterano@A1"));

    f = Comum();
    f.menorNota = kNotaDaVirada - 0.1f;
    CHECK(Cumpridas(f) == std::vector<std::string>{"virada"});

    f = Comum();
    f.raspoes = kRaspoes;
    f.pontosExtras = kPontosExtras;
    f.poderes = kPoderes;
    CHECK(Cumpridas(f) == std::vector<std::string>{"de-raspao", "ponto-extra", "poder"});

    // A cobertura e da dupla.
    f = Comum();
    f.coberturas = kCoberturas;
    CHECK(Cumpridas(f).empty());
    f.dupla = true;
    CHECK(Cumpridas(f) == std::vector<std::string>{"cobertura"});

    // A prova da semana, e o podio dela.
    f = Comum();
    f.provaDaSemana = true;
    CHECK(Cumpridas(f) == std::vector<std::string>{"prova-da-semana"});
    f.posicaoNaSemana = 3;
    CHECK(Tem(Cumpridas(f), "podio"));
    f.posicaoNaSemana = 4;
    CHECK_FALSE(Tem(Cumpridas(f), "podio"));
    f.provaDaSemana = false;
    f.posicaoNaSemana = 1;
    CHECK(Cumpridas(f).empty());
}

TEST_CASE("Horas: a ficha guarda cada chave uma vez, e as horas e o quadro saem dela") {
    const Materias::Lista curso = Curso();
    Progresso p;
    CHECK(p.RegistrarAtividade("fase-limpa@A1"));
    CHECK_FALSE(p.RegistrarAtividade("fase-limpa@A1"));     // de novo nao conta
    CHECK(p.RegistrarAtividade("fase-limpa@T1"));
    CHECK(p.RegistrarAtividade("virada"));
    CHECK_FALSE(p.RegistrarAtividade(""));
    p.RegistrarAtividade("atividade-que-saiu-do-jogo");
    CHECK(De(p.Atividades()) == 2 + 2 + 6);

    const auto quadro = Quadro(p.Atividades(), curso);
    REQUIRE(quadro.size() == Todas().size());
    CHECK(quadro[0].dados.atividade == Atividade::FaseLimpa);
    CHECK(quadro[0].cumpridas == 2);
    CHECK(quadro[0].possiveis == 2);                        // as duas materias com batalha
    for (const Linha& l : quadro) {
        CHECK(l.cumpridas <= l.possiveis);
        if (l.dados.atividade == Atividade::Virada) CHECK(l.cumpridas == 1);
        if (l.dados.atividade == Atividade::Podio) CHECK(l.cumpridas == 0);
    }

    CHECK(FraseDasNovas({"fase-limpa@A1", "virada"}, curso) ==
          "Horas complementares: +8 h (Fase limpa em INF 1, Virada)");
    CHECK(FraseDasNovas({}, curso).empty());
    CHECK(FraseDasNovas({"nao-existe"}, curso).empty());
}
