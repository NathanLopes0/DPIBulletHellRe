//
// A fase 3 do Lucas: processos do Elixir trocando mensagens.
//

#include "Elixir.h"

namespace Elixir {

    int Remetente(const int vez, const int processos) {
        if (processos <= 0 || vez < 0) return -1;
        return vez % processos;
    }

    std::string Placar(const int n) {
        return "send(aluno, {:duvida, " + std::to_string(n) + "})";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreMensagens = Dificuldade::Escolher(n, kEntreMensagens, 0.35f);
        return d;
    }
}
