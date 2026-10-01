//
// A ponte entre a montagem de caminhos e o disco.
//

#include "CaminhosArquivo.h"

#include <filesystem>
#include <SDL.h>
#include <SDL_log.h>

#include "Caminhos.h"

namespace {

    /// Quantos niveis subir procurando a base. Cinco cobre com folga as pastas de
    /// build que aparecem na pratica (cmake-build-debug, build/Debug, out/build/
    /// x64-Debug do Visual Studio).
    constexpr int kMaxNiveis = 5;

    /// A pasta cuja presenca identifica a base. Assets e o unico diretorio que o
    /// jogo PRECISA achar, entao serve de marco sem inventar um arquivo novo.
    const char* kMarcador = "Assets";

    std::string gBase;
    bool gInicializado = false;

    /// Esta e a unica funcao que olha o disco. E ela que a camada pura recebe
    /// como predicado - e e por isso que a busca da base pode ser testada sem
    /// criar diretorio nenhum.
    bool TemAssets(const std::string& diretorio) {
        std::error_code ec;
        return std::filesystem::is_directory(Caminhos::Juntar(diretorio, kMarcador), ec);
    }

}

namespace Caminhos {

bool Inicializar() {

    gInicializado = true;

    // 1. A pasta do executavel. Independe de onde o jogo foi lancado, e e isso
    //    que faz a maquina de Arcade funcionar.
    if (char* doExe = SDL_GetBasePath()) {
        const std::string partida(doExe);
        SDL_free(doExe);
        gBase = ProcurarSubindo(partida, kMaxNiveis, TemAssets);
        if (!gBase.empty()) {
            SDL_Log("CAMINHOS: base em %s (achada a partir do executavel)", gBase.c_str());
            return true;
        }
    }

    // 2. O diretorio de trabalho, para o caso de o executavel estar fora da
    //    arvore do projeto.
    std::error_code ec;
    const std::string trabalho = std::filesystem::current_path(ec).string();
    if (!ec) {
        gBase = ProcurarSubindo(trabalho, kMaxNiveis, TemAssets);
        if (!gBase.empty()) {
            SDL_Log("CAMINHOS: base em %s (achada a partir do diretorio de trabalho)",
                    gBase.c_str());
            return true;
        }
    }

    // 3. Pior caso: o comportamento historico. Assim nada fica PIOR do que era,
    //    e o log diz exatamente o que procurar.
    gBase = "..";
    SDL_Log("CAMINHOS: nao achei uma pasta com \"%s\" dentro, nem subindo %d niveis "
            "a partir do executavel, nem a partir do diretorio de trabalho (%s). "
            "Usando \"..\", que e o comportamento antigo: o jogo so vai achar os "
            "arquivos se for lancado de um nivel abaixo da pasta que contem %s.",
            kMarcador, kMaxNiveis, trabalho.c_str(), kMarcador);
    return false;
}

const std::string& Base() {
    return gBase;
}

std::string Asset(const std::string& relativo) {
    if (!gInicializado) {
        // Nao e fatal, mas e sinal de ordem de inicializacao errada: alguem pediu
        // um arquivo antes de a base ser descoberta. Avisa UMA vez.
        static bool avisado = false;
        if (!avisado) {
            avisado = true;
            SDL_Log("CAMINHOS: Asset(\"%s\") foi chamado antes de Inicializar(). "
                    "Usando \"..\" por enquanto.", relativo.c_str());
        }
        gBase = "..";
    }
    return Juntar(Juntar(gBase, kMarcador), relativo);
}

std::string Save(const std::string& relativo) {

    const std::string pasta = Juntar(gBase.empty() ? ".." : gBase, "Saves");

    std::error_code ec;
    std::filesystem::create_directories(pasta, ec);
    if (ec) {
        // Nao lanca: quem chama trata o arquivo que nao abre. Mas registra, porque
        // save que nao grava e a falha mais silenciosa que este jogo pode ter.
        SDL_Log("CAMINHOS: nao consegui criar a pasta de saves %s (%s). "
                "O progresso dos alunos nao vai ser gravado.",
                pasta.c_str(), ec.message().c_str());
    }
    return Juntar(pasta, relativo);
}

}
