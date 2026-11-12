// A ponte entre as regras lidas de arquivo e o motor.

#include "RegrasDeAtaqueArquivo.h"

#include <memory>
#include <SDL_log.h>

#include "Behaviors.h"
#include "PastaDoChefe.h"
#include "PathShapes.h"
#include "../Motor/Random.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Actors/Player/Player.h"
#include "../Scenes/Battle/Battle.h"

#include "../Motor/CaminhosArquivo.h"

namespace {

    /// O caminho e montado na hora, e nao guardado num const de namespace.
    std::string Caminho(const std::string& chefe) {
        return Caminhos::Asset(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kRegras));
    }

    /// Um chefe de cada vez, lido na primeira vez que alguem o pede.
    std::map<std::string, RegrasLidas> gPorChefe;

    const RegrasLidas& Lidas(const std::string& chefe) {

        const auto ja = gPorChefe.find(chefe);
        if (ja != gPorChefe.end()) return ja->second;

        RegrasLidas lidas;
        const std::string caminho = Caminho(chefe);

        const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
        if (!texto) {
            SDL_Log("REGRAS: nao foi possivel abrir %s. Os ataques do chefe \"%s\" que "
                    "dependem dele ficarao sem configuracao de projetil.",
                    caminho.c_str(), chefe.c_str());
        } else {
            lidas = LerRegras(*texto);

            for (const auto& p : lidas.problemas) {
                SDL_Log("REGRAS em %s: %s", caminho.c_str(), p.c_str());
            }
        }

        return gPorChefe.emplace(chefe, std::move(lidas)).first->second;
    }

    /// Traduz a forma nomeada: primeiro as que existem em codigo, depois o
    /// arquivo de formas do incremento 1.
    PathShapes::Path FormaPeloNome(const std::string& nome) {
        if (nome == "Reta" || nome.empty()) return PathShapes::Reta();
        return PathShapes::DoArquivo(nome);
    }

    Mira MiraPeloNome(const std::string& nome, const float parametro) {
        if (nome == "MirarNoJogador") return Mira(Mira::MirarNoJogador);
        if (nome == "MirarPrevendo")  return Mira(Mira::MirarPrevendo, parametro);
        if (nome == "AnguloFixo")     return Mira(Mira::AnguloFixo, parametro);
        return Mira();   // AlinharComVelocidade
    }

    /// A unica parte que precisa saber os tipos concretos. Cresce uma linha por behavior novo - e o
    /// preco de descrever comportamento em dados.
    PathShapes::Path FormaDaMotion(const DescricaoDeBehavior& d, const int indice) {
        if (!d.formaPorIndice.empty()) return PathShapes::DaFamilia(d.formaPorIndice, indice);
        return FormaPeloNome(d.forma);
    }

    void Inserir(Projectile* p, const DescricaoDeBehavior& d, const int indice) {
        if (d.tipo == "Path")
            p->insertMotion<PathBehavior>(FormaDaMotion(d, indice), d.velocidade, d.atraso,
                                          MiraPeloNome(d.mira, d.parametroDaMira),
                                          d.pararNoPonto, d.pararPor);
        else if (d.tipo == "Tracking")
            p->insertMotion<TrackingBehavior>(d.atraso, d.forca, d.duracao);
        else if (d.tipo == "Wobble")
            p->insertMotion<WobbleBehavior>(d.atraso, d.amplitude, d.frequencia, d.duracao);
        else if (d.tipo == "MiraPeriodica")
            p->insertMotion<MiraPeriodicaBehavior>(d.ritmo);
        else if (d.tipo == "Amarrar") {
            Amarra::Corda corda;
            corda.comprimento = d.comprimento;
            corda.duracao = d.duracao;
            Amarra::SentidoPeloNome(d.sentido, corda.sentido);   // a leitura ja o validou
            p->insertMotion<AmarrarBehavior>(corda, d.escreve);
        }
        else if (d.tipo == "Ancorar")
            p->insertMotion<AncorarBehavior>(Vector2(d.dx, d.dy), d.duracao, d.velocidade);
        else if (d.tipo == "Accelerate")
            p->insertModifier<AccelerateBehavior>(d.atraso, d.fator);
        else if (d.tipo == "SlowDown")
            p->insertModifier<SlowDownBehavior>(d.atraso, d.fator);
        else if (d.tipo == "Activate")
            p->insertModifier<ActivateBehavior>(d.atraso, d.velocidadeInicial);
        else if (d.tipo == "Deactivate")
            p->insertModifier<DeactivateBehavior>(d.atraso);
        else if (d.tipo == "PulsoDeVelocidade")
            p->insertModifier<PulsoDeVelocidadeBehavior>(d.ritmo, d.moduloInvestida, d.moduloPausa);
        else if (d.tipo == "Promessa")
            p->insertModifier<PromessaBehavior>(d.atraso, d.resolve, d.velocidade, d.escreve);
    }

}

std::function<void(Projectile*, int)> ConfiguratorDeArquivo(const std::string& chefe,
                                                            const std::string& nomeDoConjunto) {

    const RegrasLidas& lidas = Lidas(chefe);

    const auto it = lidas.conjuntos.find(nomeDoConjunto);
    if (it == lidas.conjuntos.end()) {
        SDL_Log("REGRAS: nao existe conjunto chamado \"%s\" em %s. Os projeteis "
                "deste ataque sairao sem configuracao.",
                nomeDoConjunto.c_str(), Caminho(chefe).c_str());
        return [](Projectile*, int) {};
    }

    return ConfiguratorDeRegras(it->second);
}

namespace {

    /// Algum aluno em campo esta preso por uma corrente?
    bool AlgumJogadorAcorrentado(Projectile* p) {
        if (!p) return false;
        const auto batalha = dynamic_cast<Battle*>(p->GetScene());
        if (!batalha) return false;

        for (const Player* jogador : batalha->Jogadores()) {
            if (jogador && jogador->EstaAcorrentado()) return true;
        }
        return false;
    }

}

std::function<void(Projectile*, int)> ConfiguratorDeRegras(const std::vector<Regra>& regrasLidas) {

    // As regras sao copiadas para um shared_ptr e capturadas por valor.
    auto regras = std::make_shared<const std::vector<Regra>>(regrasLidas);

    return [regras](Projectile* p, const int indice) {
        if (!p) return;

        // Quem pergunta ao mundo e esta camada, e nao RegrasDeAtaque.cpp.
        const bool alvoAcorrentado = AlgumJogadorAcorrentado(p);

        for (const Regra& r : *regras) {
            const float sorteio = Random::GetFloatRange(0.0f, 1.0f);
            if (!RegraSeAplica(r, indice, sorteio, alvoAcorrentado)) continue;

            // A escala vem primeiro, na mesma ordem em que os configurators escritos a mao faziam.
            if (r.escala) p->SetScale(*r.escala);

            // A rotacao e da rajada, e nao do tipo.
            if (r.rotacionar) {
                if (const auto b = dynamic_cast<BossProjectile*>(p)) {
                    b->DefinirRotacaoPelaVelocidade(*r.rotacionar);
                }
            }

            if (!r.posto.empty()) {
                if (const auto b = dynamic_cast<BossProjectile*>(p)) b->DefinirPosto(r.posto);
            }

            if (r.temMotion) Inserir(p, r.motion, indice);
            for (const auto& m : r.modifiers) Inserir(p, m, indice);

            if (!r.animacao.empty()) {
                if (auto anim = p->GetComponent<DrawAnimatedComponent>()) {
                    anim->SetAnimation(r.animacao);
                }
            }
        }
    };
}
