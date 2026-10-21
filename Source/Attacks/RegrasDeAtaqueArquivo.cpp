//
// A ponte entre as regras lidas de arquivo e o motor.
//
// Separado de RegrasDeAtaque.cpp pelo mesmo motivo de PathShapesArquivo.cpp:
// aquele e puro e entra no alvo de testes; este abre arquivo, escreve log e
// conhece Projectile, entao fica de fora.
//

#include "RegrasDeAtaqueArquivo.h"

#include <fstream>
#include <memory>
#include <SDL_log.h>

#include "Behaviors.h"
#include "PastaDoChefe.h"
#include "PathShapes.h"
#include "../Random.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Actors/Player/Player.h"
#include "../Scenes/Battle/Battle.h"

#include "../CaminhosArquivo.h"

namespace {

    /// O caminho e montado na HORA, e nao guardado num const de namespace: um
    /// const std::string em escopo de namespace e construido ANTES do main,
    /// portanto antes de Caminhos::Inicializar() descobrir a pasta base, e
    /// capturaria o palpite de reserva.
    std::string Caminho(const std::string& chefe) {
        return Caminhos::Asset(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kRegras));
    }

    /// UM CHEFE DE CADA VEZ, lido na primeira vez que alguem o pede.
    std::map<std::string, RegrasLidas> gPorChefe;

    const RegrasLidas& Lidas(const std::string& chefe) {

        const auto ja = gPorChefe.find(chefe);
        if (ja != gPorChefe.end()) return ja->second;

        RegrasLidas lidas;
        const std::string caminho = Caminho(chefe);

        std::ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            SDL_Log("REGRAS: nao foi possivel abrir %s. Os ataques do chefe \"%s\" que "
                    "dependem dele ficarao sem configuracao de projetil.",
                    caminho.c_str(), chefe.c_str());
        } else {
            const std::string texto((std::istreambuf_iterator<char>(arquivo)),
                                     std::istreambuf_iterator<char>());
            lidas = LerRegras(texto);

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

    /// A unica parte que precisa saber os tipos concretos. Cresce uma linha por
    /// behavior novo - e o preco de descrever comportamento em dados.
    /// A forma que esta motion pede: uma forma so, ou a da familia na posicao
    /// deste projetil.
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
        else if (d.tipo == "Reescrever")
            p->insertModifier<ReescreverBehavior>(d.atraso, d.raio, d.escreve,
                                                  d.velocidade, d.forca, d.duracao);
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

    /// @brief Algum aluno em campo esta preso por uma corrente?
    ///
    /// Vai pelo projetil ate a cena porque o configurator e criado na MONTAGEM
    /// do chefe, quando os jogadores ainda nem existem - guardar um ponteiro
    /// para o Player ali seria guardar um ponteiro para o nada.
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

    // As regras sao copiadas para um shared_ptr e capturadas por valor: o
    // configurator vive enquanto o chefe viver, e nao pode depender de um mapa
    // global - nem de uma lista de outro modulo - continuar existindo.
    auto regras = std::make_shared<const std::vector<Regra>>(regrasLidas);

    return [regras](Projectile* p, const int indice) {
        if (!p) return;

        // QUEM PERGUNTA AO MUNDO E ESTA CAMADA, e nao RegrasDeAtaque.cpp. A
        // condicao "alvoAcorrentado" e a unica que depende do estado do jogo em
        // vez da posicao na rajada; manter a consulta aqui e o que deixa a
        // regra pura continuar pura e testavel com um bool.
        //
        // QUALQUER UM DOS DOIS SERVE, em dupla: se um esta preso, as tarefas
        // perseguem. E a mesma escolha da nota compartilhada - o que acontece
        // com um acontece para a dupla -, e evita o absurdo de metade da rajada
        // perseguir um e metade o outro dentro do mesmo disparo.
        const bool alvoAcorrentado = AlgumJogadorAcorrentado(p);

        for (const Regra& r : *regras) {
            const float sorteio = Random::GetFloatRange(0.0f, 1.0f);
            if (!RegraSeAplica(r, indice, sorteio, alvoAcorrentado)) continue;

            // A escala vem primeiro, na mesma ordem em que os configurators
            // escritos a mao faziam: SetScale antes dos behaviors. As duas coisas
            // sao independentes (uma mexe no sprite e no colisor, a outra na
            // velocidade), mas manter a ordem deixa o comportamento identico ao
            // do codigo que este arquivo substitui, inclusive sob comparacao.
            if (r.escala) p->SetScale(*r.escala);

            // A ROTACAO E DA RAJADA, E NAO DO TIPO. Ver o campo em
            // RegrasDeAtaque.h: o mesmo "Estruturas" do Salles e lista na fase
            // 1 (seta, gira) e arvore na fase 3 (no simetrico, nao gira).
            //
            // O dynamic_cast e porque so projetil de chefe gira - o do jogador
            // nao tem o conceito. Uma regra com "rotacionar" num projetil que
            // nao e de chefe simplesmente nao faz nada, que e melhor do que
            // falhar: a regra pode estar certa para os outros projeteis dela.
            if (r.rotacionar) {
                if (const auto b = dynamic_cast<BossProjectile*>(p)) {
                    b->DefinirRotacaoPelaVelocidade(*r.rotacionar);
                }
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
