//
// A ponte entre as opcoes do jogo e o disco.
//

#include "ConfiguracaoArquivo.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <SDL_log.h>

#include "../Core/CaminhosArquivo.h"

namespace {
    std::string Caminho() { return Caminhos::Save("opcoes.cfg"); }
}

namespace ConfiguracaoArquivo {

Configuracao::Dados Carregar() {
    const std::string caminho = Caminho();

    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        // Nao e erro: e a primeira vez que o jogo abre nesta maquina.
        return {};
    }

    std::ostringstream buffer;
    buffer << arquivo.rdbuf();

    const Configuracao::Lida lida = Configuracao::Ler(buffer.str());
    for (const auto& p : lida.problemas) {
        SDL_Log("OPCOES: %s: %s", caminho.c_str(), p.c_str());
    }
    return lida.dados;
}

bool Gravar(const Configuracao::Dados& dados) {
    const std::string caminho = Caminho();
    const std::string temporario = caminho + ".tmp";

    {
        std::ofstream saida(temporario, std::ios::trunc);
        if (!saida.is_open()) {
            SDL_Log("OPCOES: nao consegui abrir %s para escrever. A opcao vale ate o jogo "
                    "fechar, mas NAO foi gravada.", temporario.c_str());
            return false;
        }
        saida << Configuracao::Serializar(dados);
        if (!saida.good()) {
            SDL_Log("OPCOES: erro ao escrever %s. A opcao NAO foi gravada.", temporario.c_str());
            return false;
        }
    }   // fecha antes de renomear

    std::error_code ec;
    std::filesystem::rename(temporario, caminho, ec);
    if (ec) {
        SDL_Log("OPCOES: nao consegui trocar %s por %s (%s). O arquivo anterior foi "
                "preservado.", temporario.c_str(), caminho.c_str(), ec.message().c_str());
        std::filesystem::remove(temporario, ec);
        return false;
    }
    return true;
}

}
