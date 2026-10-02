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
#include "PathShapes.h"
#include "../Random.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

#include "../CaminhosArquivo.h"

namespace {

    /// O caminho e montado na PRIMEIRA chamada, e nao na inicializacao estatica.
    ///
    /// A diferenca importa: um "const std::string" em escopo de namespace e
    /// construido ANTES do main, portanto antes de Caminhos::Inicializar()
    /// descobrir a pasta base. Ele capturava o palpite de reserva ("..") e o
    /// arquivo passava a ser procurado relativo ao diretorio de onde o jogo foi
    /// lancado - exatamente o problema que a resolucao de caminhos veio resolver.
    /// O static DENTRO da funcao e construido na primeira chamada, que acontece
    /// quando um chefe e montado, bem depois da inicializacao.
    const std::string& Caminho() {
        static const std::string c = Caminhos::Asset("Attacks/regras.json");
        return c;
    }
    std::map<std::string, std::vector<Regra>> gConjuntos;
    bool gArquivoLido = false;

    void GarantirArquivoLido() {
        if (gArquivoLido) return;
        gArquivoLido = true;

        std::ifstream arquivo(Caminho());
        if (!arquivo.is_open()) {
            SDL_Log("REGRAS: nao foi possivel abrir %s. Os ataques que dependem "
                    "dele ficarao sem configuracao de projetil.", Caminho().c_str());
            return;
        }

        const std::string texto((std::istreambuf_iterator<char>(arquivo)),
                                 std::istreambuf_iterator<char>());
        RegrasLidas lidas = LerRegras(texto);

        for (const auto& p : lidas.problemas) {
            SDL_Log("REGRAS em %s: %s", Caminho().c_str(), p.c_str());
        }
        gConjuntos = std::move(lidas.conjuntos);
        SDL_Log("REGRAS: %zu conjunto(s) carregado(s) de %s", gConjuntos.size(), Caminho().c_str());
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
    void Inserir(Projectile* p, const DescricaoDeBehavior& d) {
        if (d.tipo == "Path")
            p->insertMotion<PathBehavior>(FormaPeloNome(d.forma), d.velocidade, d.atraso,
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
    }

}

std::function<void(Projectile*, int)> ConfiguratorDeArquivo(const std::string& nomeDoConjunto) {

    GarantirArquivoLido();

    const auto it = gConjuntos.find(nomeDoConjunto);
    if (it == gConjuntos.end()) {
        SDL_Log("REGRAS: nao existe conjunto chamado \"%s\" em %s. Os projeteis "
                "deste ataque sairao sem configuracao.", nomeDoConjunto.c_str(), Caminho().c_str());
        return [](Projectile*, int) {};
    }

    return ConfiguratorDeRegras(it->second);
}

std::function<void(Projectile*, int)> ConfiguratorDeRegras(const std::vector<Regra>& regrasLidas) {

    // As regras sao copiadas para um shared_ptr e capturadas por valor: o
    // configurator vive enquanto o chefe viver, e nao pode depender de um mapa
    // global - nem de uma lista de outro modulo - continuar existindo.
    auto regras = std::make_shared<const std::vector<Regra>>(regrasLidas);

    return [regras](Projectile* p, const int indice) {
        if (!p) return;
        for (const Regra& r : *regras) {
            const float sorteio = Random::GetFloatRange(0.0f, 1.0f);
            if (!RegraSeAplica(r, indice, sorteio)) continue;

            // A escala vem primeiro, na mesma ordem em que os configurators
            // escritos a mao faziam: SetScale antes dos behaviors. As duas coisas
            // sao independentes (uma mexe no sprite e no colisor, a outra na
            // velocidade), mas manter a ordem deixa o comportamento identico ao
            // do codigo que este arquivo substitui, inclusive sob comparacao.
            if (r.escala) p->SetScale(*r.escala);

            if (r.temMotion) Inserir(p, r.motion);
            for (const auto& m : r.modifiers) Inserir(p, m);

            if (!r.animacao.empty()) {
                if (auto anim = p->GetComponent<DrawAnimatedComponent>()) {
                    anim->SetAnimation(r.animacao);
                }
            }
        }
    };
}
