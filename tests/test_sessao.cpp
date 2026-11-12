// Quem esta jogando, de cada lado do painel.

#include "doctest.h"

#include "../Source/Aluno/Sessao.h"
#include "../Source/Aluno/Matricula.h"

#include <string>
#include <vector>

namespace {

/// Uma dupla, com as matriculas que o teste pedir.
std::vector<Sessao::Dados> Dupla(const std::string& um, const std::string& dois) {
    std::vector<Sessao::Dados> sessoes(2);
    sessoes[0].matricula = um;
    sessoes[1].matricula = dois;
    return sessoes;
}

} // namespace

TEST_CASE("Sessao: um jogador e o caso normal, dois e o co-op") {

    CHECK(Sessao::Quantos(Sessao::Modo::Um) == 1);
    CHECK(Sessao::Quantos(Sessao::Modo::Dois) == 2);
}

TEST_CASE("Sessao: a matricula do parceiro e recusada") {

    const auto sessoes = Dupla("89384", "");

    // A posicao 1 esta digitando, e a 0 ja tem o 89384.
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "89384"));
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "90112") == false);
}

TEST_CASE("Sessao: a propria matricula NAO conta contra si mesma") {

    // Voltar e redigitar o proprio numero tem de continuar valendo.
    const auto sessoes = Dupla("89384", "90112");

    CHECK(Sessao::JaEstaEmUso(sessoes, 0, "89384") == false);
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "90112") == false);

    // Mas trocar para a do outro continua sendo recusado.
    CHECK(Sessao::JaEstaEmUso(sessoes, 0, "90112"));
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "89384"));
}

TEST_CASE("Sessao: dois visitantes podem jogar juntos") {

    // Visitante nao grava arquivo nenhum, entao nao ha o que um sobrescreva do outro.
    const auto sessoes = Dupla(Matricula::kVisitante, Matricula::kVisitante);

    CHECK(Sessao::JaEstaEmUso(sessoes, 0, Matricula::kVisitante) == false);
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, Matricula::kVisitante) == false);
}

TEST_CASE("Sessao: posicao vazia nao bloqueia ninguem") {

    // Antes de o jogador 2 digitar, a sessao dele existe com matricula vazia.
    const auto sessoes = Dupla("", "");

    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "") == false);
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "89384") == false);
}

TEST_CASE("Sessao: com um jogador, quem entrou foi o ultimo") {

    CHECK(Sessao::Proximo(Sessao::Modo::Um, 0) == Sessao::Passo::Jogar);
}

TEST_CASE("Sessao: com dois, o primeiro chama o segundo e o segundo joga") {

    CHECK(Sessao::Proximo(Sessao::Modo::Dois, 0) == Sessao::Passo::OutroJogador);
    CHECK(Sessao::Proximo(Sessao::Modo::Dois, 1) == Sessao::Passo::Jogar);
}

TEST_CASE("Sessao: uma posicao alem do fim nao pede mais ninguem") {

    // Nao deveria acontecer, e se acontecer o caminho seguro e jogar.
    CHECK(Sessao::Proximo(Sessao::Modo::Um, 1) == Sessao::Passo::Jogar);
    CHECK(Sessao::Proximo(Sessao::Modo::Dois, 2) == Sessao::Passo::Jogar);
    CHECK(Sessao::Proximo(Sessao::Modo::Dois, 99) == Sessao::Passo::Jogar);
}

// ---- Quem jogou do outro lado ----

TEST_CASE("Sessao: o parceiro de uma posicao e a matricula da outra") {

    const auto sessoes = Dupla("89384", "90112");

    CHECK(Sessao::ParceiroDe(sessoes, 0) == "90112");
    CHECK(Sessao::ParceiroDe(sessoes, 1) == "89384");
}

TEST_CASE("Sessao: quem jogou sozinho nao tem parceiro") {

    // E o caso normal, e tudo tem de valer nele.
    std::vector<Sessao::Dados> so_um(1);
    so_um[0].matricula = "89384";

    CHECK(Sessao::ParceiroDe(so_um, 0).empty());
}

TEST_CASE("Sessao: visitante nao e parceiro, porque nao tem matricula") {

    // A nota continua valendo para o progresso de quem se identificou.
    const auto comVisitante = Dupla("89384", Matricula::kVisitante);

    CHECK(Sessao::ParceiroDe(comVisitante, 0).empty());
    CHECK(Sessao::ParceiroDe(comVisitante, 1) == "89384");
}

TEST_CASE("Sessao: dois visitantes nao tem parceiro nenhum") {

    const auto doisVisitantes = Dupla(Matricula::kVisitante, Matricula::kVisitante);

    CHECK(Sessao::ParceiroDe(doisVisitantes, 0).empty());
    CHECK(Sessao::ParceiroDe(doisVisitantes, 1).empty());
}

TEST_CASE("Sessao: o jogo recem-aberto nao tem parceiro") {

    // A matricula vazia nao e um valor especial a mais para lembrar.
    const std::vector<Sessao::Dados> vazias(2);

    CHECK(Sessao::ParceiroDe(vazias, 0).empty());
    CHECK(Sessao::ParceiroDe(vazias, 1).empty());
}

TEST_CASE("Sessao: posicao fora da faixa nao le memoria alheia") {

    const auto sessoes = Dupla("89384", "90112");

    CHECK(Sessao::ParceiroDe(sessoes, 2).empty());
    CHECK(Sessao::ParceiroDe(sessoes, 99).empty());
    CHECK(Sessao::ParceiroDe({}, 0).empty());
}

TEST_CASE("Sessao: tres posicoes nao tem O parceiro") {

    // A palavra so faz sentido numa dupla: nomear um dos dois seria dizer que o outro nao jogou.
    std::vector<Sessao::Dados> tres(3);
    tres[0].matricula = "1";
    tres[1].matricula = "2";
    tres[2].matricula = "3";

    for (size_t i = 0; i < tres.size(); ++i) {
        CAPTURE(i);
        CHECK(Sessao::ParceiroDe(tres, i).empty());
    }
}
