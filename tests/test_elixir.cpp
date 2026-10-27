//
// A fase 3 do Lucas: processos do Elixir trocando mensagens.
//

#include "doctest.h"

#include <set>

#include "../Source/Attacks/Elixir.h"

using namespace Elixir;

TEST_CASE("Elixir: as mensagens saem em rodizio, uma de cada processo vivo") {
    std::set<int> quem;
    for (int vez = 0; vez < 4; ++vez) quem.insert(Remetente(vez, 4));
    CHECK(quem == std::set<int>{0, 1, 2, 3});
    CHECK(Remetente(5, 4) == 1);
    CHECK(Remetente(3, 0) == -1);   // sem processo, ninguem manda
    CHECK(Placar(7) == "send(aluno, {:duvida, 7})");
    CHECK(DegrauDo(Dificuldade::Nivel::Normal).entreMensagens == kEntreMensagens);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).entreMensagens < kEntreMensagens);
}
