// O painel de controle do gabinete. Camada pura.
//
// O que estes testes protegem nao e a aparencia do rodape: e o ACORDO entre o
// que a tela anuncia e o que o jogo escuta. A selecao de fases escrevia "T
// trocar usuario" e escutava o T - num gabinete que nao tem tecla T. A acao era
// impossivel, e o rodape ensinava a fazer o impossivel.
//
// Por isso os testes insistem em duas coisas: o rodape sai do BOTAO, e os
// botoes nao compartilham tecla.

#include "doctest.h"

#include "../Source/Painel.h"

#include <string>
#include <vector>

namespace {

/// Um teclado zerado do tamanho que a SDL entrega, com uma tecla apertada.
std::vector<Uint8> TecladoCom(const SDL_Scancode tecla) {
    std::vector<Uint8> teclado(SDL_NUM_SCANCODES, 0);
    teclado[tecla] = 1;
    return teclado;
}

const Painel::Botao kTodos[] = {Painel::Botao::Um, Painel::Botao::Dois, Painel::Botao::Tres};

} // namespace

TEST_CASE("Painel: cada botao tem uma tecla so dele") {

    // DUAS ACOES NA MESMA TECLA disparariam juntas - e o rodape anunciaria duas
    // coisas diferentes para o mesmo aperto.
    for (const Painel::Botao a : kTodos) {
        CHECK(Painel::Tecla(a) != SDL_SCANCODE_UNKNOWN);
        for (const Painel::Botao b : kTodos) {
            if (a == b) continue;
            CHECK(Painel::Tecla(a) != Painel::Tecla(b));
        }
    }
}

TEST_CASE("Painel: cada botao tem um nome so dele, e o nome diz o numero") {

    CHECK(std::string(Painel::Nome(Painel::Botao::Um))   == "BOTAO 1");
    CHECK(std::string(Painel::Nome(Painel::Botao::Dois)) == "BOTAO 2");
    CHECK(std::string(Painel::Nome(Painel::Botao::Tres)) == "BOTAO 3");
}

TEST_CASE("Painel: Apertado responde pela tecla do botao, e so por ela") {

    for (const Painel::Botao a : kTodos) {
        const std::vector<Uint8> teclado = TecladoCom(Painel::Tecla(a));

        for (const Painel::Botao b : kTodos) {
            CHECK(Painel::Apertado(teclado.data(), b) == (a == b));
        }
    }
}

TEST_CASE("Painel: teclado ausente nao e botao apertado") {

    // A cena recebe o ponteiro da SDL e o repassa. Se ele chegar nulo, o jogo
    // nao pode cair - e muito menos agir como se tudo estivesse apertado.
    CHECK(Painel::Apertado(nullptr, Painel::Botao::Um) == false);
}

TEST_CASE("Painel: o rodape anuncia o NOME do botao que a cena escuta") {

    // ESTE E O TESTE QUE IMPORTA. O item do rodape e construido a partir do
    // botao, entao nao existe caminho para escrever um nome e escutar outro.
    for (const Painel::Botao b : kTodos) {
        const std::string linha = Painel::Rodape({{b, "fazer alguma coisa"}});

        CHECK(linha.find(Painel::Nome(b)) != std::string::npos);
        CHECK(linha.find("fazer alguma coisa") != std::string::npos);

        // E nao anuncia os outros.
        for (const Painel::Botao outro : kTodos) {
            if (outro == b) continue;
            CHECK(linha.find(Painel::Nome(outro)) == std::string::npos);
        }
    }
}

TEST_CASE("Painel: o rodape junta os itens com o mesmo espacamento") {

    const std::string linha = Painel::Rodape({{Painel::Botao::Um, "jogar"},
                                              {Painel::Botao::Dois, "voltar"}});

    CHECK(linha == std::string("BOTAO 1") + Painel::kEntreNomeEAcao + "jogar"
                 + Painel::kEntreAcoes
                 + "BOTAO 2" + Painel::kEntreNomeEAcao + "voltar");
}

TEST_CASE("Painel: o manche entra no rodape sem ser botao") {

    const std::string linha = Painel::Rodape({Painel::Manche("escolher"),
                                              {Painel::Botao::Um, "digitar"}});

    CHECK(linha.find("MANCHE") == 0);
    CHECK(linha.find("BOTAO 1") != std::string::npos);
}

TEST_CASE("Painel: rodape vazio e linha vazia, e um item so nao leva separador") {

    CHECK(Painel::Rodape({}).empty());

    const std::string um = Painel::Rodape({{Painel::Botao::Tres, "ranking"}});
    CHECK(um == std::string("BOTAO 3") + Painel::kEntreNomeEAcao + "ranking");
    CHECK(um.find(Painel::kEntreAcoes) == std::string::npos);
}
