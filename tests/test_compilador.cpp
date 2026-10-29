//
// A fase 2 do Vladimir: o compilador.
//

#include "doctest.h"

#include <cstdint>
#include <set>
#include <string>

#include "../Source/Attacks/Vladimir/Compilador.h"
#include "Ajudantes.h"

using namespace Compilador;

TEST_CASE("Compilador: o lexico quebra a linha em tokens, e o espaco nao conta") {
    const auto t = Lexico("x = a + b * 2;");
    REQUIRE(t.size() == 8u);
    CHECK(t[0].tipo == Tipo::Id);
    CHECK(t[1].tipo == Tipo::Igual);
    CHECK(t[3].tipo == Tipo::Op);
    CHECK(t[6].tipo == Tipo::Num);
    CHECK(t[6].texto == "2");
    CHECK(t[7].tipo == Tipo::Fim);
}

TEST_CASE("Compilador: o parser respeita a precedencia, e o codigo sai em pos-ordem") {
    // O * ANTES DO +: a + b * 2 e a + (b * 2).
    const Arvore a = Sintatico(Lexico("x = a + b * 2;"));
    REQUIRE(a.raiz >= 0);
    const auto codigo = Ajudantes::Codigo(a);
    CHECK(codigo == std::vector<std::string>{"LOAD a", "LOAD b", "PUSH 2", "MUL", "ADD", "STORE x"});
    // DA ESQUERDA PARA A DIREITA entre iguais: a - b - 1 e (a - b) - 1.
    const Arvore s = Sintatico(Lexico("x = a - b - 1;"));
    CHECK(Ajudantes::Codigo(s) == std::vector<std::string>{"LOAD a", "LOAD b", "SUB", "PUSH 1", "SUB", "STORE x"});
    // O * DO COMECO TAMBEM: a * b + 3.
    CHECK(Ajudantes::Codigo(Sintatico(Lexico("x = a * b + 3;"))) ==
          std::vector<std::string>{"LOAD a", "LOAD b", "MUL", "PUSH 3", "ADD", "STORE x"});
}

TEST_CASE("Compilador: a pos-ordem poe as folhas antes de quem as junta, e o = por ultimo") {
    const Arvore a = Sintatico(Lexico("x = a * 2 + b;"));
    const auto ordem = PosOrdem(a);
    REQUIRE(ordem.size() == a.nos.size());
    CHECK(ordem.back() == a.raiz);
    std::set<int> vistos;
    for (const int n : ordem) {
        const No& no = a.nos[static_cast<size_t>(n)];
        if (no.esq >= 0) CHECK(vistos.count(no.esq) == 1u);
        if (no.dir >= 0) CHECK(vistos.count(no.dir) == 1u);
        vistos.insert(n);
    }
}

TEST_CASE("Compilador: cada rodada gera outra linha, com a e b e de 2 a 3 operadores") {
    std::set<std::string> linhas;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoCompilador);
        const std::string linha = Gerar(s);
        CAPTURE(linha);
        CHECK(linha.rfind("x = ", 0) == 0u);
        CHECK(linha.back() == ';');
        const auto t = Lexico(linha);
        int ops = 0;
        bool temA = false, temB = false;
        for (const auto& k : t) {
            if (k.tipo == Tipo::Op) ++ops;
            if (k.texto == "a") temA = true;
            if (k.texto == "b") temB = true;
        }
        CHECK(ops >= 2);
        CHECK(ops <= 3);
        CHECK(temA);
        CHECK(temB);
        // A LINHA COMPILA: uma instrucao por no, menos o alvo.
        const Arvore a = Sintatico(t);
        REQUIRE(a.raiz >= 0);
        CHECK(Ajudantes::Codigo(a).size() == a.nos.size() - 1u);
        // E A RODADA CABE NO RELOGIO, mesmo a maior, mesmo no Normal.
        CHECK(MarcosDa(static_cast<int>(t.size()), static_cast<int>(a.nos.size())).pronto < kRodada);
        linhas.insert(linha);
    }
    CHECK(linhas.size() >= 100u);
}

TEST_CASE("Compilador: a arvore cabe no campo, um nivel abaixo do outro, e longe do aluno") {
    const Arvore a = Sintatico(Lexico("x = a + b * 2 - 7;"));
    const auto p = PosicoesDaArvore(a, 1200.0f);
    for (size_t n = 0; n < a.nos.size(); ++n) {
        CHECK(p[n].x > 40.0f);
        CHECK(p[n].x < 1160.0f);
        CHECK(p[n].y < 560.0f);                         // o aluno vive abaixo de 600
        const No& no = a.nos[n];
        for (const int f : {no.esq, no.dir}) if (f >= 0) CHECK(p[static_cast<size_t>(f)].y > p[n].y);
    }
}

TEST_CASE("Compilador: as etapas vem em ordem, e o placar e as caras dizem cada uma") {
    const Marcos m = MarcosDa(8, 7);
    CHECK(EtapaEm(0.0f, m) == Etapa::Lexico);
    CHECK(EtapaEm(m.sintatico, m) == Etapa::Sintatico);
    CHECK(EtapaEm(m.codigo, m) == Etapa::Codigo);
    CHECK(EtapaEm(m.pronto, m) == Etapa::Pronto);
    CHECK(Placar("x = a;", Etapa::Codigo, "LOAD a") == "x = a;\nLOAD a");
    CHECK(CaraDaInstrucao("STORE x") == "STORE");
    CHECK(CaraDaInstrucao("MUL") == "MUL");
    CHECK(CaraDoLexema("*") == "Vezes");
    CHECK(CaraDoLexema("7") == "N7");
    CHECK(DegrauDo(Dificuldade::Nivel::Normal).porInstrucao == kPorInstrucao);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).porInstrucao < kPorInstrucao);
}

TEST_CASE("Compilador: cada luta sorteia tres linhas, e as folhas a e b tem ficha propria") {
    Variacao::Sorteio s1 = Variacao::SorteioDa(7u, kFaseDoCompilador);
    Variacao::Sorteio s2 = Variacao::SorteioDa(7u, kFaseDoCompilador);
    CHECK(Sortear(s1).linhas == Sortear(s2).linhas);   // a mesma luta, as mesmas linhas
    std::set<std::string> planos;
    for (std::uint32_t semente = 0; semente < 20; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoCompilador);
        const Plano p = Sortear(s);
        planos.insert(p.linhas[0] + p.linhas[1] + p.linhas[2]);
    }
    CHECK(planos.size() == 20u);

    const Arvore a = Sintatico(Lexico("x = a + b * 2;"));
    std::set<std::string> caras;
    for (int n = 0; n < static_cast<int>(a.nos.size()); ++n) caras.insert(CaraDoNo(a, n));
    CHECK(caras == std::set<std::string>{"X", "Igual", "IdA", "Mais", "IdB", "Vezes", "N2"});
}

TEST_CASE("Compilador: cada no da arvore sabe de que token saiu, e todo token menos o ; vira um no") {
    const auto tokens = Lexico("x = a + b * 2;");
    const Arvore a = Sintatico(tokens);
    std::set<int> usados;
    for (const No& no : a.nos) {
        REQUIRE(no.token >= 0);
        REQUIRE(no.token < static_cast<int>(tokens.size()));
        CHECK(tokens[static_cast<size_t>(no.token)].texto == no.texto);
        usados.insert(no.token);
    }
    CHECK(usados.size() == tokens.size() - 1u);
    CHECK(usados.count(static_cast<int>(tokens.size()) - 1) == 0u);   // o ; nao vira no
}
