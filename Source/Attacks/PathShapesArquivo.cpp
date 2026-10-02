//
// A ponte entre as formas lidas de arquivo e o motor.
//
// Separado de PathShapes.cpp pelo mesmo motivo dos outros pares: aquele e puro e
// entra no alvo de testes; este abre arquivo e escreve log, entao fica de fora.
//

#include "PathShapes.h"

#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include <SDL_log.h>

#include "../CaminhosArquivo.h"

namespace {
    /// Formas lidas de arquivo, por nome. Separado do cache por parametros
    /// porque a chave aqui e textual.
    std::map<std::string, PathShapes::Path> gFormasDeArquivo;

    /// Familias lidas de arquivo: um nome -> varias formas, na ordem em que os
    /// projeteis da rajada as recebem.
    std::map<std::string, std::vector<PathShapes::Path>> gFamiliasDeArquivo;

    bool gArquivoLido = false;

    const std::string& Caminho() {
        static const std::string c = Caminhos::Asset("Paths/formas.json");
        return c;
    }

    /// Le o arquivo UMA vez. Extraido de dentro de DoArquivo porque agora ha dois
    /// pontos de entrada (forma simples e familia) e os dois precisam do arquivo
    /// carregado - duplicar a carga daria dois caches e dois conjuntos de avisos.
    void GarantirLido() {

        if (gArquivoLido) return;
        gArquivoLido = true;   // uma tentativa so, mesmo que falhe

        std::ifstream arquivo(Caminho());
        if (!arquivo.is_open()) {
            SDL_Log("FORMAS: nao foi possivel abrir %s. As formas de arquivo virarao retas.",
                    Caminho().c_str());
            return;
        }

        const std::string texto((std::istreambuf_iterator<char>(arquivo)),
                                 std::istreambuf_iterator<char>());
        PathShapes::FormasLidas lidas = PathShapes::LerFormas(texto);

        for (const auto& p : lidas.problemas) {
            SDL_Log("FORMAS em %s: %s", Caminho().c_str(), p.c_str());
        }

        for (auto& f : lidas.formas) {
            gFormasDeArquivo.emplace(f.first,
                std::make_shared<const std::vector<Vector2>>(std::move(f.second)));
        }

        for (auto& fam : lidas.familias) {
            std::vector<PathShapes::Path> formas;
            formas.reserve(fam.second.size());
            for (auto& forma : fam.second) {
                formas.push_back(std::make_shared<const std::vector<Vector2>>(std::move(forma)));
            }
            gFamiliasDeArquivo.emplace(fam.first, std::move(formas));
        }

        SDL_Log("FORMAS: %zu forma(s) e %zu familia(s) carregadas de %s",
                gFormasDeArquivo.size(), gFamiliasDeArquivo.size(), Caminho().c_str());
    }

    /// Avisa UMA vez por chave, para nao encher o log a cada projetil disparado.
    bool PrimeiroAviso(const std::string& chave) {
        static std::map<std::string, bool> jaAvisados;
        return jaAvisados.emplace(chave, true).second;
    }
}

namespace PathShapes {

Path DoArquivo(const std::string& nome) {

    GarantirLido();

    if (const auto it = gFormasDeArquivo.find(nome); it != gFormasDeArquivo.end()) {
        return it->second;
    }

    if (PrimeiroAviso("forma:" + nome)) {
        // Mensagem separada quando o nome existe, mas como FAMILIA: e o erro mais
        // provavel depois que familias passaram a existir, e dizer "nao existe"
        // mandaria a pessoa procurar um erro de digitacao que nao ha.
        if (gFamiliasDeArquivo.count(nome) == 1) {
            SDL_Log("FORMAS: \"%s\" e uma FAMILIA de formas, nao uma forma so. "
                    "Na regra, use \"formaPorIndice\" em vez de \"forma\". Usando uma reta.",
                    nome.c_str());
        }
        else {
            SDL_Log("FORMAS: nao existe forma chamada \"%s\" em %s. Usando uma reta.",
                    nome.c_str(), Caminho().c_str());
        }
    }
    return Reta();
}

Path DaFamilia(const std::string& nome, const int indice) {

    GarantirLido();

    const auto it = gFamiliasDeArquivo.find(nome);
    if (it == gFamiliasDeArquivo.end()) {
        if (PrimeiroAviso("familia:" + nome)) {
            if (gFormasDeArquivo.count(nome) == 1) {
                SDL_Log("FORMAS: \"%s\" e uma forma so, nao uma familia. Na regra, use "
                        "\"forma\" em vez de \"formaPorIndice\". Usando uma reta.", nome.c_str());
            }
            else {
                SDL_Log("FORMAS: nao existe familia chamada \"%s\" em %s. Usando uma reta.",
                        nome.c_str(), Caminho().c_str());
            }
        }
        return Reta();
    }

    const auto& familia = it->second;
    if (familia.empty()) return Reta();

    if (indice < 0 || indice >= static_cast<int>(familia.size())) {
        // Acontece quando o ataque dispara mais projeteis do que a familia tem
        // formas - numa arvore, mais tiros do que folhas. O resto da volta ao
        // inicio em vez de virar reta: um ramo repetido estraga menos a leitura do
        // que um tiro atravessando a arvore em linha reta.
        if (PrimeiroAviso("indice:" + nome)) {
            SDL_Log("FORMAS: a familia \"%s\" tem %zu forma(s) e um projetil pediu a de indice "
                    "%d. O ataque dispara mais projeteis do que a familia cobre - confira "
                    "\"projeteis\" em fases.json. Reaproveitando as formas em ciclo.",
                    nome.c_str(), familia.size(), indice);
        }
        const int n = static_cast<int>(familia.size());
        return familia[static_cast<size_t>(((indice % n) + n) % n)];
    }

    return familia[static_cast<size_t>(indice)];
}

}
