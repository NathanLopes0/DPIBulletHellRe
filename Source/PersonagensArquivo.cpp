//
// A ponte entre o catalogo de personagens e o disco.
//

#include "PersonagensArquivo.h"

#include <fstream>
#include <sstream>
#include <SDL_log.h>

#include "CaminhosArquivo.h"

namespace Personagens {

const Catalogo& Carregado() {

    // Construido na PRIMEIRA chamada, e nao na inicializacao estatica: um objeto
    // de escopo de namespace nasceria antes do main, portanto antes de
    // Caminhos::Inicializar descobrir a pasta base - o mesmo cuidado dos outros
    // leitores de dados.
    static const Catalogo catalogo = [] {

        const std::string caminho = Caminhos::Asset("personagens.json");

        std::ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            SDL_Log("PERSONAGENS: nao consegui abrir %s. Ninguem vai conseguir montar uma "
                    "personagem. Confira se a pasta Assets foi copiada junto.", caminho.c_str());
            return Catalogo{};
        }

        std::ostringstream buffer;
        buffer << arquivo.rdbuf();
        Catalogo lido = LerCatalogo(buffer.str());

        for (const auto& p : lido.problemas) {
            SDL_Log("PERSONAGENS: %s", p.c_str());
        }
        SDL_Log("PERSONAGENS: %d categoria(s) e %d combinacao(oes) de %s",
                static_cast<int>(lido.categorias.size()),
                static_cast<int>(lido.predefinidas.size()), caminho.c_str());

        return lido;
    }();

    return catalogo;
}

}
