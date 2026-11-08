//
// A pausa da batalha, no computador. Camada pura.
//

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Interface/Pausa.h"

using namespace Pausa;

TEST_CASE("Pausa: no gabinete nunca pausa, e no computador so com a luta valendo") {
    CHECK(PodePausar(false, true));
    CHECK_FALSE(PodePausar(true, true));     // o painel nao tem ESC
    CHECK_FALSE(PodePausar(false, false));   // carregando, ou na tela de fim
    CHECK_FALSE(PodePausar(true, false));
}

TEST_CASE("Pausa: o menu abre no Continuar, e da a volta nas pontas") {
    Menu m;
    CHECK_FALSE(m.aberto);
    Abrir(m);
    CHECK(m.aberto);
    CHECK(Escolhida(m) == Opcao::Continuar);

    Mover(m, 1);
    CHECK(Escolhida(m) == Opcao::Recomecar);
    Mover(m, 1);
    CHECK(Escolhida(m) == Opcao::Sair);
    Mover(m, 1);
    CHECK(Escolhida(m) == Opcao::Continuar);
    Mover(m, -1);
    CHECK(Escolhida(m) == Opcao::Sair);

    // Fechar e abrir de novo volta ao Continuar: quem apertou ESC sem querer
    // nao pode cair no Sair da vez anterior.
    Fechar(m);
    CHECK_FALSE(m.aberto);
    Abrir(m);
    CHECK(Escolhida(m) == Opcao::Continuar);
}

TEST_CASE("Pausa: cada opcao tem um texto, e nenhum se repete") {
    std::set<std::string> vistos;
    for (const Opcao o : kOpcoes) {
        const std::string t = Texto(o);
        CHECK_FALSE(t.empty());
        vistos.insert(t);
    }
    CHECK(static_cast<int>(vistos.size()) == kQuantas);
    CHECK(std::string(Texto(Opcao::Recomecar)) == "Recome\xc3\xa7" "ar a fase");
}

TEST_CASE("Pausa: a volta conta 3, 2, 1 e entao a luta segue") {
    CHECK(Contagem(kRetomada) == 3);
    CHECK(Contagem(2.5f) == 3);
    CHECK(Contagem(2.0f) == 2);
    CHECK(Contagem(1.01f) == 2);
    CHECK(Contagem(0.4f) == 1);
    CHECK(Contagem(0.0f) == 0);
    CHECK(Contagem(-0.1f) == 0);
    CHECK(Contagem(9.0f) == 3);   // nunca mostra mais que o comeco
}

TEST_CASE("Pausa: perder o foco pausa nas mesmas condicoes do ESC, e so uma vez") {
    CHECK(PausaAoPerderOFoco(false, true, false));
    CHECK_FALSE(PausaAoPerderOFoco(true, true, false));    // no gabinete, nunca
    CHECK_FALSE(PausaAoPerderOFoco(false, false, false));  // carregando ou no fim
    CHECK_FALSE(PausaAoPerderOFoco(false, true, true));    // ja estava pausada
}
