// A ponte entre os tipos de projetil lidos de arquivo e o motor.

#include "ProjeteisDeChefeArquivo.h"

#include <map>
#include <memory>
#include <SDL_log.h>

#include "ProjeteisDeChefe.h"
#include "PastaDoChefe.h"
#include "../Actors/Teachers/Boss.h"
#include "../Actors/Teachers/BossFactory/BossProjectileFactory/FabricaDeProjetil.h"

#include "../Motor/CaminhosArquivo.h"

namespace {

    /// O caminho e montado na hora, e nao guardado num const de namespace.
    std::string Caminho(const std::string& chefe) {
        return Caminhos::Asset(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kProjeteis));
    }

    /// Um chefe de cada vez, lido na primeira vez que alguem o pede.
    std::map<std::string, ProjeteisLidos> gPorChefe;

    const ProjeteisLidos& Lidos(const std::string& chefe) {

        const auto ja = gPorChefe.find(chefe);
        if (ja != gPorChefe.end()) return ja->second;

        ProjeteisLidos lidos;
        const std::string caminho = Caminho(chefe);

        const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
        if (!texto) {
            SDL_Log("PROJETEIS: nao foi possivel abrir %s. O chefe \"%s\" ficara sem "
                    "projetil: os tipos dele vivem nesse arquivo, e nao ha mais fabrica "
                    "em C++. Confira se a pasta Assets foi copiada junto com o executavel.",
                    caminho.c_str(), chefe.c_str());
        } else {
            lidos = LerProjeteis(*texto);

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
        DescricaoDeProjetil d = NoNivel(entrada.second, boss->NivelDeDificuldade());

        // Os caminhos sao resolvidos aqui, e nao no leitor: o leitor e puro e nao conhece a pasta base.
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
