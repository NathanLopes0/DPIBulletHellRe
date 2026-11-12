// O teclado digital da tela de matricula.

#include "doctest.h"

#include <algorithm>
#include <set>
#include <string>

#include "../Source/Entrada/Navegacao.h"
#include "../Source/Aluno/Matricula.h"
#include "../Source/Entrada/Teclado.h"

namespace {
    size_t IndiceDe(const std::string& rotulo) {
        const auto& t = Teclado::Teclas();
        for (size_t i = 0; i < t.size(); ++i) if (t[i].rotulo == rotulo) return i;
        return t.size();
    }
}

TEST_CASE("Teclado: tem os dez digitos e as quatro acoes") {

    const auto& teclas = Teclado::Teclas();

    std::set<char> digitos;
    int apagar = 0, entrar = 0, visitante = 0, voltar = 0;
    for (const auto& t : teclas) {
        switch (t.acao) {
            case Teclado::Acao::Digito:    digitos.insert(t.digito); break;
            case Teclado::Acao::Apagar:    ++apagar; break;
            case Teclado::Acao::Entrar:    ++entrar; break;
            case Teclado::Acao::Visitante: ++visitante; break;
            case Teclado::Acao::Voltar:    ++voltar; break;
        }
    }

    CHECK(digitos.size() == 10);
    for (char d = '0'; d <= '9'; ++d) { CAPTURE(d); CHECK(digitos.count(d) == 1); }

    CHECK(apagar == 1);
    CHECK(entrar == 1);
    CHECK(voltar == 1);

    // O modo visitante tem de continuar existindo.
    CHECK(visitante == 1);
}

TEST_CASE("Teclado: duas teclas nunca ocupam a mesma casa") {

    std::set<std::pair<int,int>> ocupadas;
    for (const auto& t : Teclado::Teclas()) {
        CAPTURE(t.rotulo);
        CHECK(ocupadas.insert({t.coluna, t.linha}).second);
    }
}

TEST_CASE("Teclado: a grade cobre todas as teclas, e so elas") {

    const auto grade = Teclado::Grade();
    const auto& teclas = Teclado::Teclas();

    std::multiset<size_t> naGrade;
    for (const auto& coluna : grade) for (const size_t i : coluna) naGrade.insert(i);

    CHECK(naGrade.size() == teclas.size());
    for (size_t i = 0; i < teclas.size(); ++i) {
        CAPTURE(teclas[i].rotulo);
        CHECK(naGrade.count(i) == 1);
    }
}

TEST_CASE("Teclado: dentro de uma coluna, a grade vai de cima para baixo") {

    // A seta para baixo anda por esta ordem.
    const auto grade = Teclado::Grade();
    const auto& teclas = Teclado::Teclas();

    for (const auto& coluna : grade) {
        for (size_t k = 1; k < coluna.size(); ++k) {
            CAPTURE(teclas[coluna[k]].rotulo);
            CHECK(teclas[coluna[k-1]].linha < teclas[coluna[k]].linha);
        }
    }
}

TEST_CASE("Teclado: toda tecla e alcancavel pelas setas") {

    // Sem teclado fisico, uma tecla inalcancavel e uma funcao que o aluno nao tem como usar.
    const auto grade = Teclado::Grade();
    const auto& teclas = Teclado::Teclas();

    std::set<size_t> vistos{Teclado::Inicial()};
    std::vector<size_t> fila{Teclado::Inicial()};
    while (!fila.empty()) {
        const size_t onde = fila.back();
        fila.pop_back();
        for (const auto mover : {Navegacao::Cima, Navegacao::Baixo,
                                 Navegacao::Esquerda, Navegacao::Direita}) {
            if (const size_t destino = mover(grade, onde); vistos.insert(destino).second) {
                fila.push_back(destino);
            }
        }
    }

    for (size_t i = 0; i < teclas.size(); ++i) {
        CAPTURE(teclas[i].rotulo);
        CHECK(vistos.count(i) == 1);
    }
}

TEST_CASE("Teclado: nenhuma seta sai da grade") {

    const auto grade = Teclado::Grade();
    const size_t quantas = Teclado::Teclas().size();

    for (size_t i = 0; i < quantas; ++i) {
        for (const auto mover : {Navegacao::Cima, Navegacao::Baixo,
                                 Navegacao::Esquerda, Navegacao::Direita}) {
            CAPTURE(i);
            CHECK(mover(grade, i) < quantas);
        }
    }
}

TEST_CASE("Teclado: a tecla inicial existe e e um digito") {

    // Abrir a tela com a selecao numa acao seria ruim.
    const auto& teclas = Teclado::Teclas();
    REQUIRE(Teclado::Inicial() < teclas.size());
    CHECK(teclas[Teclado::Inicial()].acao == Teclado::Acao::Digito);
}

TEST_CASE("Teclado: dá para digitar uma matricula inteira so com as setas") {

    // A travessia completa, que e o que o aluno faz de verdade.
    const auto grade = Teclado::Grade();
    const auto& teclas = Teclado::Teclas();

    std::string digitado;
    for (const char alvo : std::string("89384")) {
        // Caminha ate a tecla do digito alvo.
        size_t onde = Teclado::Inicial();
        std::set<size_t> vistos{onde};
        std::vector<size_t> fila{onde};
        bool achou = teclas[onde].acao == Teclado::Acao::Digito && teclas[onde].digito == alvo;
        while (!achou && !fila.empty()) {
            const size_t atual = fila.back(); fila.pop_back();
            for (const auto mover : {Navegacao::Cima, Navegacao::Baixo,
                                     Navegacao::Esquerda, Navegacao::Direita}) {
                const size_t d = mover(grade, atual);
                if (!vistos.insert(d).second) continue;
                if (teclas[d].acao == Teclado::Acao::Digito && teclas[d].digito == alvo) {
                    onde = d; achou = true; break;
                }
                fila.push_back(d);
            }
        }
        CAPTURE(alvo);
        REQUIRE(achou);
        digitado = Matricula::Digitar(digitado, teclas[onde].digito);
    }

    CHECK(digitado == "89384");
    CHECK(Matricula::Validar(digitado).valida);
}
