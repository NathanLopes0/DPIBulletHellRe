//
// A ponte entre as formas de trajetoria e o disco.
//
// Separado de PathShapes.cpp de proposito: aquele arquivo e PURO - so geometria
// e leitura de texto - e por isso entra no alvo de testes sem arrastar o SDL
// junto. Tudo que abre arquivo e escreve log mora aqui.
//

#include "PathShapes.h"

#include <fstream>
#include <map>
#include <string>
#include <SDL_log.h>

#include "../CaminhosArquivo.h"

namespace {
    /// Formas lidas de arquivo, por nome. Separado do cache por parametros
    /// porque a chave aqui e textual.
    std::map<std::string, PathShapes::Path> gFormasDeArquivo;
    bool gArquivoLido = false;
}

namespace PathShapes {

Path DoArquivo(const std::string& nome) {

    static const std::string kCaminho = Caminhos::Asset("Paths/formas.json");

    if (!gArquivoLido) {
        gArquivoLido = true;   // uma tentativa so, mesmo que falhe

        std::ifstream arquivo(kCaminho);
        if (!arquivo.is_open()) {
            SDL_Log("FORMAS: nao foi possivel abrir %s. As formas de arquivo virarao retas.",
                    kCaminho.c_str());
        }
        else {
            const std::string texto((std::istreambuf_iterator<char>(arquivo)),
                                     std::istreambuf_iterator<char>());
            const FormasLidas lidas = LerFormas(texto);

            for (const auto& p : lidas.problemas) {
                SDL_Log("FORMAS em %s: %s", kCaminho.c_str(), p.c_str());
            }
            for (auto& f : lidas.formas) {
                gFormasDeArquivo.emplace(f.first,
                    std::make_shared<const std::vector<Vector2>>(std::move(f.second)));
            }
            SDL_Log("FORMAS: %zu forma(s) carregada(s) de %s", gFormasDeArquivo.size(), kCaminho.c_str());
        }
    }

    if (const auto it = gFormasDeArquivo.find(nome); it != gFormasDeArquivo.end()) {
        return it->second;
    }

    // Avisa uma vez por nome, para nao encher o log a cada projetil.
    static std::map<std::string, bool> jaAvisados;
    if (jaAvisados.emplace(nome, true).second) {
        SDL_Log("FORMAS: nao existe forma chamada \"%s\" em %s. Usando uma reta.",
                nome.c_str(), kCaminho.c_str());
    }
    return Reta();
}

}
