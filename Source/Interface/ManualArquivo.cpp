//
// A ponte entre o manual e o disco.
//

#include "ManualArquivo.h"

#include <SDL_log.h>

#include "../Motor/CaminhosArquivo.h"

namespace Manual {

const Lido& Carregado() {

    // Construido na PRIMEIRA chamada, e nao na inicializacao estatica: um objeto
    // de escopo de namespace nasceria antes do main, portanto antes de
    // Caminhos::Inicializar descobrir a pasta base - o mesmo cuidado dos outros
    // leitores de dados.
    static const Lido manual = [] {

        const std::string caminho = Caminhos::Asset("como-jogar.json");

        const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
        if (!texto) {
            SDL_Log("MANUAL: nao consegui abrir %s. A tela Como Jogar vai ficar vazia. "
                    "Confira se a pasta Assets foi copiada junto.", caminho.c_str());
            return Lido{};
        }

        Lido lido = LerManual(*texto);

        for (const auto& p : lido.problemas) {
            SDL_Log("MANUAL: %s", p.c_str());
        }
        SDL_Log("MANUAL: %d pagina(s) de %s", static_cast<int>(lido.paginas.size()), caminho.c_str());
        return lido;
    }();

    return manual;
}

}
