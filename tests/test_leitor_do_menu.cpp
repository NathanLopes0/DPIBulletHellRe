//
// As batidas dos menus da batalha: a pausa e o resultado.
//

#include "doctest.h"

#include "../Source/Entrada/Painel.h"
#include "../Source/Scenes/Battle/LeitorDoMenu.h"

#include <vector>

namespace {

/// Um teclado zerado do tamanho que a SDL entrega.
std::vector<Uint8> Solto() { return std::vector<Uint8>(SDL_NUM_SCANCODES, 0); }

std::vector<Uint8> Com(const SDL_Scancode tecla) {
    std::vector<Uint8> teclado = Solto();
    teclado[tecla] = 1;
    return teclado;
}

} // namespace

TEST_CASE("LeitorDoMenu: o que ja vinha apertado quando o menu abriu nao conta") {
    LeitorDoMenu leitor;
    const auto cima = Com(Painel::Tecla(Painel::Jogador::Um, Painel::Direcao::Cima));
    // Comeca segurado: a tecla precisa ser solta antes.
    CHECK_FALSE(leitor.Ler(cima.data()).cima);
    CHECK_FALSE(leitor.Ler(Solto().data()).cima);
    CHECK(leitor.Ler(cima.data()).cima);
    // Segurada, conta uma vez so.
    CHECK_FALSE(leitor.Ler(cima.data()).cima);

    // Segurar de novo (o menu abriu outra vez): a tecla apertada agora tambem nao conta.
    leitor.Ler(Solto().data());
    const auto confirmar = Com(Painel::Tecla(Painel::Jogador::Um, Painel::Botao::Um));
    leitor.Segurar();
    CHECK_FALSE(leitor.Ler(confirmar.data()).confirmar);
}

TEST_CASE("LeitorDoMenu: qualquer um dos dois paineis mexe no menu, e o Enter confirma") {
    for (const Painel::Jogador quem : Painel::kJogadores) {
        LeitorDoMenu leitor;
        leitor.Ler(Solto().data());
        CHECK(leitor.Ler(Com(Painel::Tecla(quem, Painel::Direcao::Baixo)).data()).baixo);
        leitor.Ler(Solto().data());
        CHECK(leitor.Ler(Com(Painel::Tecla(quem, Painel::Botao::Um)).data()).confirmar);
    }
    for (const SDL_Scancode enter : {SDL_SCANCODE_RETURN, SDL_SCANCODE_KP_ENTER}) {
        LeitorDoMenu leitor;
        leitor.Ler(Solto().data());
        const auto batidas = leitor.Ler(Com(enter).data());
        CHECK(batidas.confirmar);
        CHECK_FALSE(batidas.cima);
        CHECK_FALSE(batidas.baixo);
    }
}
