//
// A ponte entre a prova da semana e o disco.
//

#include "ProvaDaSemanaArquivo.h"

#include <fstream>

#include <SDL_log.h>

#include "../Motor/CaminhosArquivo.h"

namespace ProvaDaSemana {

    namespace {
        constexpr const char* kArquivo = "prova_da_semana.csv";
    }

    std::vector<Entrada> Carregar() {
        const auto texto = Caminhos::LerTexto(Caminhos::Save(kArquivo));
        return texto ? Ler(*texto) : std::vector<Entrada>{};
    }

    bool Acrescentar(const Entrada& e) {
        const std::string caminho = Caminhos::Save(kArquivo);
        const bool novo = !Caminhos::LerTexto(caminho).has_value();
        std::ofstream saida(caminho, std::ios::app);
        if (!saida.is_open()) {
            SDL_Log("PROVA DA SEMANA: nao consegui escrever %s. A nota desta prova nao entrou no ranking da semana.",
                    caminho.c_str());
            return false;
        }
        if (novo) saida << Cabecalho() << "\n";
        saida << Linha(e) << "\n";
        return saida.good();
    }
}
