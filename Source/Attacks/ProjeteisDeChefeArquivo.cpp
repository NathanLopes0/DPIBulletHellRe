//
// A ponte entre os tipos de projetil lidos de arquivo e o motor.
//
// Separado de ProjeteisDeChefe.cpp pelo mesmo motivo de FasesDeAtaqueArquivo.cpp
// e RegrasDeAtaqueArquivo.cpp: aquele e puro e entra no alvo de testes; este abre
// arquivo, escreve log e conhece Boss, entao fica de fora.
//

#include "ProjeteisDeChefeArquivo.h"

#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <SDL_log.h>

#include "ProjeteisDeChefe.h"
#include "PastaDoChefe.h"
#include "../Actors/Teachers/Boss.h"
#include "../Actors/Teachers/BossFactory/BossProjectileFactory/FabricaDeProjetil.h"

#include "../CaminhosArquivo.h"

namespace {

    /// O caminho e montado na HORA, e nao guardado num const de namespace: um
    /// const std::string em escopo de namespace e construido ANTES do main,
    /// portanto antes de Caminhos::Inicializar() descobrir a pasta base, e
    /// capturaria o palpite de reserva.
    std::string Caminho(const std::string& chefe) {
        return Caminhos::Asset(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kProjeteis));
    }

    /// UM CHEFE DE CADA VEZ, lido na primeira vez que alguem o pede.
    ///
    /// Era um arquivo so com todos dentro, lido inteiro na primeira montagem de
    /// qualquer chefe. Com uma pasta por chefe, montar o Thiago nao abre mais os
    /// arquivos dos outros quatro - e um erro no arquivo do Julio deixa de
    /// aparecer no log de uma batalha do Salles.
    std::map<std::string, ProjeteisLidos> gPorChefe;

    const ProjeteisLidos& Lidos(const std::string& chefe) {

        const auto ja = gPorChefe.find(chefe);
        if (ja != gPorChefe.end()) return ja->second;

        ProjeteisLidos lidos;
        const std::string caminho = Caminho(chefe);

        std::ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            SDL_Log("PROJETEIS: nao foi possivel abrir %s. O chefe \"%s\" ficara sem "
                    "projetil: os tipos dele vivem nesse arquivo, e nao ha mais fabrica "
                    "em C++. Confira se a pasta Assets foi copiada junto com o executavel.",
                    caminho.c_str(), chefe.c_str());
        } else {
            std::ostringstream buffer;
            buffer << arquivo.rdbuf();
            lidos = LerProjeteis(buffer.str());

            for (const auto& p : lidos.problemas) {
                SDL_Log("PROJETEIS em %s: %s", caminho.c_str(), p.c_str());
            }
        }

        return gPorChefe.emplace(chefe, std::move(lidos)).first->second;
    }
}

bool RegistrarProjeteisDeArquivo(Boss* boss, const std::string& nomeDoConjunto) {

    if (!boss) {
        SDL_Log("PROJETEIS: RegistrarProjeteisDeArquivo recebeu boss nulo.");
        return false;
    }

    const ProjeteisLidos& lidos = Lidos(nomeDoConjunto);

    if (lidos.projeteis.empty()) {
        SDL_Log("PROJETEIS: %s nao trouxe nenhum projetil. Este chefe ficara sem projetil, "
                "e cada ataque dele vai falhar ao procurar a fabrica que pede.",
                Caminho(nomeDoConjunto).c_str());
        return false;
    }

    int registrados = 0;

    for (const auto& entrada : lidos.projeteis) {

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
