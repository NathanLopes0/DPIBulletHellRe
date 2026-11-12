//
// A ponte entre as materias lidas de arquivo e o jogo.
//

#include "MateriasArquivo.h"

#include <SDL_log.h>

#include "../Motor/CaminhosArquivo.h"

namespace Materias {

const Lista& Carregadas() {

    // Construida na primeira chamada, e nao na inicializacao estatica.
    static const Lista lista = [] {

        const std::string caminho = Caminhos::Asset("materias.json");

        const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
        if (!texto) {
            SDL_Log("MATERIAS: nao consegui abrir %s. O jogo ficara sem materias para "
                    "mostrar. Confira se a pasta Assets foi copiada junto.", caminho.c_str());
            return Lista{};
        }

        Lista lida = LerMaterias(*texto);

        for (const auto& p : lida.problemas) {
            SDL_Log("MATERIAS: %s", p.c_str());
        }
        SDL_Log("MATERIAS: %d materia(s) carregadas de %s", lida.Quantas(), caminho.c_str());

        return lida;
    }();

    return lista;
}

}
