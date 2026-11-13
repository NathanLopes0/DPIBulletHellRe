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

// ---- As horas como moeda: as optativas se compram com elas ----

namespace {
    /// A1 abre sempre; O1 e uma optativa de 10 h que pede A1; O2, uma de 8 h sem regra; e P1 uma
    /// obrigatoria que tentou ter preco.
    Materias::Lista ComOptativas() {
        return Materias::LerMaterias(R"({ "materias": [
            { "codigo": "A1", "nome": "INF 1", "chefe": "ana", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "O1", "nome": "INF 9", "chefe": "oto", "coluna": 1, "optativa": true,
              "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"], "preco": 10 } },
            { "codigo": "O2", "nome": "INF 8", "chefe": "ivo", "coluna": 1, "optativa": true,
              "desbloqueio": { "tipo": "sempre", "preco": 8 } },
            { "codigo": "P1", "nome": "INF 2", "chefe": "pia", "coluna": 0,
              "desbloqueio": { "tipo": "sempre", "preco": 5 } }
        ] })");
    }
}

TEST_CASE("Horas: so optativa tem preco, e ele e lido do desbloqueio") {
    const Materias::Lista c = ComOptativas();
    CHECK(c.problemas.size() == 1u);   // a obrigatoria com preco: avisada, e abre so com a regra
    CHECK(c.PrecoDe(c.IndiceDe("O1")) == 10);
    CHECK(c.PrecoDe(c.IndiceDe("O2")) == 8);
    CHECK(c.PrecoDe(c.IndiceDe("P1")) == 0);
    CHECK(c.PrecoDe(c.IndiceDe("A1")) == 0);
    CHECK(Materias::LerMaterias(R"({ "materias": [ { "codigo": "X", "coluna": 0, "optativa": true,
        "desbloqueio": { "tipo": "sempre", "preco": -3 } } ] })").problemas.size() == 1u);
}

TEST_CASE("Horas: a optativa abre com a regra e a compra; o saldo desconta o preco, a carga nao") {
    const Materias::Lista c = ComOptativas();
    const int o1 = c.IndiceDe("O1"), o2 = c.IndiceDe("O2");
    Progresso p;
    CHECK(PodeComprar(c, o1, p) == Compra::FaltaARegra);   // falta passar em A1
    CHECK(PodeComprar(c, o2, p) == Compra::FaltaHora);     // sem regra, mas sem horas
    CHECK(PodeComprar(c, c.IndiceDe("A1"), p) == Compra::NaoSeVende);

    p.RegistrarNota(c.IndiceDe("A1"), 80.0f, Dificuldade::Nivel::Normal);
    CHECK(c.RegraCumprida(o1, p));
    CHECK_FALSE(c.Desbloqueada(o1, p));                    // a regra sozinha nao abre
    CHECK(PodeComprar(c, o1, p) == Compra::FaltaHora);

    p.RegistrarAtividade("virada");                         // 6 h
    p.RegistrarAtividade("podio");                          // + 8 h
    CHECK(Saldo(p.Atividades(), c) == 14);
    CHECK(PodeComprar(c, o1, p) == Compra::Pode);

    p.RegistrarAtividade(Materias::ChaveDaCompra("O1"));
    CHECK(c.Desbloqueada(o1, p));
    CHECK(PodeComprar(c, o1, p) == Compra::JaComprou);
    CHECK(Gastas(p.Atividades(), c) == 10);
    CHECK(Saldo(p.Atividades(), c) == 4);
    CHECK(De(p.Atividades()) == 14);                        // a carga conta o que foi ganho
    CHECK(PodeComprar(c, o2, p) == Compra::FaltaHora);      // 4 h nao pagam 8

    // A compra nao e uma atividade: nao vale hora, nem aparece no quadro.
    for (const Linha& l : Quadro(p.Atividades(), c)) CHECK(l.cumpridas <= l.possiveis);
    CHECK(Nome(Materias::ChaveDaCompra("O1"), c).empty());
}

TEST_CASE("Horas: o saldo nunca fica negativo, nem se o preco subir depois da compra") {
    const Materias::Lista c = ComOptativas();
    Progresso p;
    p.RegistrarAtividade(Materias::ChaveDaCompra("O1"));   // comprada sem horas (um save editado)
    CHECK(Saldo(p.Atividades(), c) == 0);
}

TEST_CASE("Horas: o requisito da optativa vale junto com a regra - a mencao de ouro, as horas, as aprovacoes") {
    const Materias::Lista c = Materias::LerMaterias(R"({ "materias": [
        { "codigo": "A1", "nome": "INF 1", "professor": "Ana", "chefe": "ana", "coluna": 0,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B1", "nome": "INF 2", "professor": "Bia", "chefe": "bia", "coluna": 0,
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "O1", "nome": "INF 9", "professor": "Oto", "chefe": "oto", "coluna": 1, "optativa": true,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"], "preco": 5,
                           "requisitos": [ { "tipo": "mencao", "mencao": "ouro" }, { "tipo": "horas", "quantas": 6 } ] } },
        { "codigo": "O2", "nome": "INF 8", "professor": "Rui", "chefe": "rui", "coluna": 1, "optativa": true,
          "desbloqueio": { "tipo": "sempre",
                           "requisitos": [ { "tipo": "aprovadas", "quantas": 2 }, { "tipo": "mencao", "mencao": "dificil" },
                                           { "tipo": "mencao", "mencao": "platina" }, { "tipo": "sorte" } ] } }
    ] })");
    // A mencao que nao existe e o tipo que nao existe ficam de fora, avisados.
    CHECK(c.problemas.size() == 2u);
    const int a1 = c.IndiceDe("A1"), b1 = c.IndiceDe("B1"), o1 = c.IndiceDe("O1"), o2 = c.IndiceDe("O2");
    CHECK(c.Por(o2)->desbloqueio.requisitos.size() == 2u);

    // A frase diz tudo o que falta: a regra e os requisitos.
    const std::string frase = c.ExigenciaDe(o1);
    CHECK(frase.find("INF 1") != std::string::npos);
    CHECK(frase.find("pelo menos 1 men\xc3\xa7\xc3\xa3o de ouro") != std::string::npos);
    CHECK(frase.find("6 h") != std::string::npos);
    CHECK(c.ExigenciaDe(o2).rfind("Precisa de 2 aprova", 0) == 0);

    Progresso p;
    p.RegistrarNota(a1, 80.0f, Dificuldade::Nivel::Normal);
    CHECK_FALSE(c.RegraCumprida(o1, p));                  // passou, mas sem ouro e sem horas
    CHECK(PodeComprar(c, o1, p) == Compra::FaltaARegra);
    p.RegistrarNota(a1, 92.0f, Dificuldade::Nivel::Normal);
    CHECK_FALSE(c.RegraCumprida(o1, p));                  // o ouro veio; as horas, nao
    p.RegistrarAtividade("virada");                        // 6 h
    CHECK(c.RegraCumprida(o1, p));
    CHECK(PodeComprar(c, o1, p) == Compra::Pode);
    // Gastar as horas nao fecha o requisito: ele conta a carga, e nao o saldo.
    p.RegistrarAtividade(Materias::ChaveDaCompra("O1"));
    CHECK(Saldo(p.Atividades(), c) == 1);
    CHECK(c.Desbloqueada(o1, p));

    // A outra pede duas aprovacoes (a optativa nao conta) e uma no Dificil.
    CHECK_FALSE(c.RegraCumprida(o2, p));
    p.RegistrarNota(b1, 70.0f, Dificuldade::Nivel::Dificil);
    CHECK(c.RegraCumprida(o2, p));
    CHECK(c.Desbloqueada(o2, p));                          // sem preco: abre so com a regra
}
