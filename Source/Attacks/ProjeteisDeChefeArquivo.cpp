//
// A ponte entre os tipos de projetil lidos de arquivo e o motor.
//
// Separado de ProjeteisDeChefe.cpp pelo mesmo motivo de FasesDeAtaqueArquivo.cpp
// e RegrasDeAtaqueArquivo.cpp: aquele e puro e entra no alvo de testes; este abre
// arquivo, escreve log e conhece Boss, entao fica de fora.
//

#include "ProjeteisDeChefeArquivo.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <SDL_log.h>

#include "ProjeteisDeChefe.h"
#include "../Actors/Teachers/Boss.h"
#include "../Actors/Teachers/BossFactory/BossProjectileFactory/FabricaDeProjetil.h"

#include "../CaminhosArquivo.h"

namespace {

    /// O caminho e montado na PRIMEIRA chamada, e nao na inicializacao estatica.
    /// Um const std::string em escopo de namespace e construido ANTES do main,
    /// portanto antes de Caminhos::Inicializar() descobrir a pasta base, e
    /// capturaria o palpite de reserva.
    const std::string& Caminho() {
        static const std::string c = Caminhos::Asset("Attacks/projeteis.json");
        return c;
    }

    ProjeteisLidos gLidos;
    bool gArquivoLido = false;

    void GarantirArquivoLido() {
        if (gArquivoLido) return;
        gArquivoLido = true;

        std::ifstream arquivo(Caminho());
        if (!arquivo.is_open()) {
            SDL_Log("PROJETEIS: nao foi possivel abrir %s. NENHUM chefe tera projetil: os "
                    "tipos dos quatro vivem neste arquivo, e nao ha mais fabrica em C++. "
                    "Confira se a pasta Assets foi copiada junto com o executavel.",
                    Caminho().c_str());
            return;
        }

        std::ostringstream buffer;
        buffer << arquivo.rdbuf();

        gLidos = LerProjeteis(buffer.str());

        for (const auto& p : gLidos.problemas) {
            SDL_Log("PROJETEIS: %s", p.c_str());
        }
    }
}

bool RegistrarProjeteisDeArquivo(Boss* boss, const std::string& nomeDoConjunto) {

    if (!boss) {
        SDL_Log("PROJETEIS: RegistrarProjeteisDeArquivo recebeu boss nulo.");
        return false;
    }

    GarantirArquivoLido();

    const auto conjunto = gLidos.conjuntos.find(nomeDoConjunto);
    if (conjunto == gLidos.conjuntos.end()) {
        SDL_Log("PROJETEIS: nao existe conjunto \"%s\" em %s. Este chefe ficara sem projetil, "
                "e cada ataque dele vai falhar ao procurar a fabrica que pede.",
                nomeDoConjunto.c_str(), Caminho().c_str());
        return false;
    }

    int registrados = 0;

    for (const auto& entrada : conjunto->second) {

        const std::string& nome = entrada.first;
        DescricaoDeProjetil d = entrada.second;

        // Os caminhos sao resolvidos AQUI, e nao no leitor: o leitor e puro e nao
        // conhece a pasta base. A fabrica recebe a descricao com os caminhos ja
        // absolutos, entao ela tambem nao precisa conhecer o sistema de arquivos.
        d.sprite = Caminhos::Asset(d.sprite);
        d.dados  = Caminhos::Asset(d.dados);

        boss->AddProjectileFactory(nome, std::make_unique<FabricaDeProjetil>(d, nome));
        ++registrados;
    }

    if (registrados == 0) {
        SDL_Log("PROJETEIS: o conjunto \"%s\" nao tem nenhum projetil utilizavel.",
                nomeDoConjunto.c_str());
        return false;
    }

    return true;
}
