// O painel de controle do gabinete. Camada pura.
//
// O que estes testes protegem nao e a aparencia do rodape: e o ACORDO entre o
// que a tela anuncia e o que o jogo escuta. A selecao de fases escrevia "T
// trocar usuario" e escutava o T - num gabinete que nao tem tecla T. A acao era
// impossivel, e o rodape ensinava a fazer o impossivel.
//
// Por isso os testes insistem em tres coisas: o rodape sai do BOTAO, os botoes
// nao compartilham tecla, e o nome da tecla que aparece escrito no computador e
// o nome da tecla que o jogo le.

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

TEST_CASE("Painel: o nome da tecla nao e vazio nem repetido") {

    // Ele aparece escrito na tela no computador. Dois botoes com o mesmo nome
    // de tecla mandariam quem esta testando apertar a tecla errada.
    for (const Painel::Botao a : kTodos) {
        CHECK(std::string(Painel::NomeDaTecla(a)).empty() == false);
        for (const Painel::Botao b : kTodos) {
            if (a == b) continue;
            CHECK(std::string(Painel::NomeDaTecla(a)) != std::string(Painel::NomeDaTecla(b)));
        }
    }
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
        const std::string linha = Painel::Rodape({{b, "fazer alguma coisa"}},
                                                 Painel::Jeito::SoOPainel);

        CHECK(linha.find(Painel::Nome(b)) != std::string::npos);
        CHECK(linha.find("fazer alguma coisa") != std::string::npos);

        // E nao anuncia os outros.
        for (const Painel::Botao outro : kTodos) {
            if (outro == b) continue;
            CHECK(linha.find(Painel::Nome(outro)) == std::string::npos);
        }
    }
}

TEST_CASE("Painel: no computador o rodape diz TAMBEM a tecla, e e a tecla certa") {

    // Quem esta testando no teclado nao tem como saber qual e o botao 2. E a
    // tecla escrita tem de ser a MESMA que Apertado le - se fossem duas fontes,
    // o rodape mandaria apertar uma tecla que nao faz nada.
    for (const Painel::Botao b : kTodos) {
        const std::string linha = Painel::Rodape({{b, "jogar"}}, Painel::Jeito::ComATecla);

        CHECK(linha.find(Painel::Nome(b)) != std::string::npos);
        CHECK(linha.find(std::string("(") + Painel::NomeDaTecla(b) + ")") != std::string::npos);

        const std::vector<Uint8> teclado = TecladoCom(Painel::Tecla(b));
        CHECK(Painel::Apertado(teclado.data(), b));
    }
}

TEST_CASE("Painel: no gabinete a tecla NAO aparece") {

    // La nao ha teclado. Dizer "ESPACO" para quem tem um botao na mao e ruido.
    const std::string linha = Painel::Rodape({{Painel::Botao::Um, "jogar"}},
                                             Painel::Jeito::SoOPainel);

    CHECK(linha.find("ESPACO") == std::string::npos);
    CHECK(linha.find("(") == std::string::npos);
    CHECK(linha == std::string("BOTAO 1") + Painel::kEntreNomeEAcao + "jogar");
}

TEST_CASE("Painel: o rodape junta os itens com o mesmo espacamento") {

    const std::string linha = Painel::Rodape({{Painel::Botao::Um, "jogar"},
                                              {Painel::Botao::Dois, "voltar"}},
                                             Painel::Jeito::SoOPainel);

    CHECK(linha == std::string("BOTAO 1") + Painel::kEntreNomeEAcao + "jogar"
                 + Painel::kEntreAcoes
                 + "BOTAO 2" + Painel::kEntreNomeEAcao + "voltar");
}

TEST_CASE("Painel: o manche entra no rodape sem ser botao, e tem tecla propria") {

    const std::string semTecla = Painel::Rodape({Painel::Manche("escolher"),
                                                 {Painel::Botao::Um, "digitar"}},
                                                Painel::Jeito::SoOPainel);
    CHECK(semTecla.find("MANCHE") == 0);
    CHECK(semTecla.find("BOTAO 1") != std::string::npos);
    CHECK(semTecla.find("SETAS") == std::string::npos);

    const std::string comTecla = Painel::Rodape({Painel::Manche("escolher")},
                                                Painel::Jeito::ComATecla);
    CHECK(comTecla.find("(SETAS)") != std::string::npos);
}

TEST_CASE("Painel: rodape vazio e linha vazia, e um item so nao leva separador") {

    CHECK(Painel::Rodape({}, Painel::Jeito::SoOPainel).empty());
    CHECK(Painel::Rodape({}, Painel::Jeito::ComATecla).empty());

    const std::string um = Painel::Rodape({{Painel::Botao::Tres, "ranking"}},
                                          Painel::Jeito::SoOPainel);
    CHECK(um == std::string("BOTAO 3") + Painel::kEntreNomeEAcao + "ranking");
    CHECK(um.find(Painel::kEntreAcoes) == std::string::npos);
}

TEST_CASE("Painel: tecla de letra se chama pela propria letra") {

    // AMARRA AS DUAS COLUNAS DA TABELA. O nome da tecla e escrito na tela e o
    // scancode e o que o jogo le; sem esta conferencia, trocar o botao 2 de B
    // para C e esquecer o nome faria a tela mandar apertar a tecla errada - e
    // nada reclamaria, porque os dois campos continuariam preenchidos.
    for (const Painel::Botao b : kTodos) {
        const SDL_Scancode tecla = Painel::Tecla(b);
        if (tecla < SDL_SCANCODE_A || tecla > SDL_SCANCODE_Z) continue;

        const std::string esperado(1, static_cast<char>('A' + (tecla - SDL_SCANCODE_A)));
        CHECK(std::string(Painel::NomeDaTecla(b)) == esperado);
    }
}
