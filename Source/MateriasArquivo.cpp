//
// A ponte entre as materias lidas de arquivo e o jogo.
//

#include "MateriasArquivo.h"

#include <fstream>
#include <sstream>
#include <SDL_log.h>

#include "CaminhosArquivo.h"

namespace Materias {

const Lista& Carregadas() {

    // Construida na PRIMEIRA chamada, e nao na inicializacao estatica: um objeto
    // de escopo de namespace nasceria antes do main, portanto antes de
    // Caminhos::Inicializar descobrir a pasta base - o mesmo cuidado dos outros
    // leitores de dados.
    static const Lista lista = [] {

        const std::string caminho = Caminhos::Asset("materias.json");

        std::ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            SDL_Log("MATERIAS: nao consegui abrir %s. O jogo ficara sem materias para "
                    "mostrar. Confira se a pasta Assets foi copiada junto.", caminho.c_str());
            return Lista{};
        }

        std::ostringstream buffer;
        buffer << arquivo.rdbuf();
        Lista lida = LerMaterias(buffer.str());

        for (const auto& p : lida.problemas) {
            SDL_Log("MATERIAS: %s", p.c_str());
        }
        SDL_Log("MATERIAS: %d materia(s) carregadas de %s", lida.Quantas(), caminho.c_str());

        return lida;
    }();

    return lista;
}

}
