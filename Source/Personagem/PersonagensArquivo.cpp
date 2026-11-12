//
// A ponte entre o catalogo de personagens e o disco.
//

#include "PersonagensArquivo.h"

#include <SDL_log.h>

#include "../Motor/CaminhosArquivo.h"

namespace Personagens {

const Catalogo& Carregado() {

    // Construido na primeira chamada, e nao na inicializacao estatica.
    static const Catalogo catalogo = [] {

        const std::string caminho = Caminhos::Asset("personagens.json");

        const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
        if (!texto) {
            SDL_Log("PERSONAGENS: nao consegui abrir %s. Ninguem vai conseguir montar uma "
                    "personagem. Confira se a pasta Assets foi copiada junto.", caminho.c_str());
            return Catalogo{};
        }

        Catalogo lido = LerCatalogo(*texto);

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
