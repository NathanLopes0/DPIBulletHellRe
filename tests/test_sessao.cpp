// Quem esta jogando, de cada lado do painel. Camada pura.
//
// O que estes testes protegem sao duas regras que so aparecem quando ha dois
// jogadores, e que nenhuma tela consegue conferir sozinha:
//
//   1. A MESMA MATRICULA NAS DUAS POSICOES. As duas sessoes apontam para o
//      mesmo arquivo em Saves/, e a ultima a fechar sobrescreve a outra. O
//      aluno perderia a batalha inteira sem nenhum aviso.
//
//   2. QUEM ENTRA DEPOIS DE QUEM. A matricula e a criacao de personagem
//      terminavam as duas mandando para a selecao de fases, cada uma com a sua
//      linha. Com dois jogadores as duas precisam da mesma condicao nova, e
//      bastaria uma esquecer para o jogador 2 nunca ser chamado.

#include "doctest.h"

#include "../Source/Sessao.h"
#include "../Source/Matricula.h"

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

    // Voltar e redigitar o proprio numero tem de continuar valendo. Sem a
    // excecao da propria posicao, a segunda tentativa do jogador 1 esbarraria
    // nele mesmo - e a tela diria que ele ja esta jogando.
    const auto sessoes = Dupla("89384", "90112");

    CHECK(Sessao::JaEstaEmUso(sessoes, 0, "89384") == false);
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "90112") == false);

    // Mas trocar para a do outro continua sendo recusado.
    CHECK(Sessao::JaEstaEmUso(sessoes, 0, "90112"));
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, "89384"));
}

TEST_CASE("Sessao: dois visitantes podem jogar juntos") {

    // Visitante nao grava arquivo nenhum, entao nao ha o que um sobrescreva do
    // outro (requisitos-coop, D7). Tratar "visitante" como matricula repetida
    // impediria dois amigos de jogarem sem se identificar - que e justamente o
    // caso mais comum num gabinete de corredor.
    const auto sessoes = Dupla(Matricula::kVisitante, Matricula::kVisitante);

    CHECK(Sessao::JaEstaEmUso(sessoes, 0, Matricula::kVisitante) == false);
    CHECK(Sessao::JaEstaEmUso(sessoes, 1, Matricula::kVisitante) == false);
}

TEST_CASE("Sessao: posicao vazia nao bloqueia ninguem") {

    // Antes de o jogador 2 digitar, a sessao dele existe com matricula vazia.
    // Se vazio colidisse com vazio, a tela recusaria a primeira tentativa dele.
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

    // Nao deveria acontecer, e se acontecer o caminho seguro e JOGAR - e nao
    // pedir uma terceira matricula que nao existe, deixando o aluno preso numa
    // tela que nunca termina.
    CHECK(Sessao::Proximo(Sessao::Modo::Um, 1) == Sessao::Passo::Jogar);
    CHECK(Sessao::Proximo(Sessao::Modo::Dois, 2) == Sessao::Passo::Jogar);
    CHECK(Sessao::Proximo(Sessao::Modo::Dois, 99) == Sessao::Passo::Jogar);
}
