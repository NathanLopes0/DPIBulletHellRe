//
// A ponte entre as fases lidas de arquivo e o motor.
//
// Separado de FasesDeAtaque.cpp pelo mesmo motivo de PathShapesArquivo.cpp e
// RegrasDeAtaqueArquivo.cpp: aquele e puro e entra no alvo de testes; este abre
// arquivo, escreve log e conhece Boss, entao fica de fora.
//

#include "FasesDeAtaqueArquivo.h"

#include <fstream>
#include <map>
#include <memory>
#include <SDL_log.h>

#include "FasesDeAtaque.h"
#include "RegrasDeAtaqueArquivo.h"
#include "AttackParameters/AttackParams.h"
#include "AttackParameters/BaloonAttackParams.h"
#include "BaseStrategies/AngledAttack.h"
#include "BaseStrategies/BaloonAttack.h"
#include "BaseStrategies/CircleSpreadAttack.h"
#include "BaseStrategies/LaserAttack.h"
#include "BaseStrategies/WaveAttack.h"
#include "../Actors/Teachers/Boss.h"
#include "../Actors/Teachers/BossAttackState.h"
#include "../Components/AIComponents/FSMComponent.h"
#include "../Movements/MovementStrategies.h"

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
        static const std::string c = Caminhos::Asset("Attacks/fases.json");
        return c;
    }
    std::map<std::string, std::vector<DescricaoDeFase>> gConjuntos;
    bool gArquivoLido = false;

    void GarantirArquivoLido() {
        if (gArquivoLido) return;
        gArquivoLido = true;

        std::ifstream arquivo(Caminho());
        if (!arquivo.is_open()) {
            SDL_Log("FASES: nao foi possivel abrir %s. NENHUM chefe tera ataque: as fases "
                    "dos quatro vivem neste arquivo, e nao ha mais configuracao de reserva "
                    "em C++. Confira se a pasta Assets foi copiada junto com o executavel, "
                    "e se o jogo esta rodando de um diretorio abaixo da raiz do projeto.",
                    Caminho().c_str());
            return;
        }

        const std::string texto((std::istreambuf_iterator<char>(arquivo)),
                                 std::istreambuf_iterator<char>());
        FasesLidas lidas = LerFases(texto);

        for (const auto& p : lidas.problemas) {
            SDL_Log("FASES em %s: %s", Caminho().c_str(), p.c_str());
        }
        gConjuntos = std::move(lidas.conjuntos);
        SDL_Log("FASES: %zu conjunto(s) carregado(s) de %s", gConjuntos.size(), Caminho().c_str());
    }

    /// A unica parte que conhece as classes concretas de estrategia. Cresce uma
    /// linha por estrategia nova.
    std::unique_ptr<IAttackStrategy> CriarEstrategia(const std::string& tipo,
                                                     ProjectileFactory* spawner, Boss* boss) {
        if (tipo == "AngledAttack")       return std::make_unique<AngledAttack>(spawner, boss);
        if (tipo == "CircleSpreadAttack") return std::make_unique<CircleSpreadAttack>(spawner, boss);
        if (tipo == "WaveAttack")         return std::make_unique<WaveAttack>(spawner, boss);
        if (tipo == "BaloonAttack")       return std::make_unique<BaloonAttack>(spawner, boss);
        if (tipo == "LaserAttack")        return std::make_unique<LaserAttack>(spawner, boss);
        return nullptr;
    }

    /**
     * Instancia o movimento do chefe. 'a' e 'b' da descricao entram na ordem em
     * que cada construtor os recebe:
     *   RandomWander     a = intervalo entre destinos (s), b = velocidade (px/s)
     *   HoverAbovePlayer a = velocidade (px/s),          b = altura acima do jogador
     *   GoToCenter       nao usa nenhum dos dois
     */
    std::unique_ptr<IMovementStrategy> CriarMovimento(const DescricaoDeMovimento& m) {
        if (m.tipo == "RandomWander")     return std::make_unique<RandomWanderStrategy>(m.a, m.b);
        if (m.tipo == "HoverAbovePlayer") return std::make_unique<HoverAbovePlayerStrategy>(m.a, m.b);
        return std::make_unique<GoToCenterStrategy>();
    }

    /**
     * Monta o AttackParams.
     *
     * SO ESCREVE O QUE O ARQUIVO DISSE. Um campo ausente na descricao fica com o
     * padrao que AttackParams ja tem - e por isso que os campos de
     * DescricaoDeAtaque sao optional em vez de trazerem copias desses padroes.
     * Um ataque migrado que omite "angulo" se comporta exatamente como o codigo
     * C++ que tambem nao mexia em params->angle.
     */
    // O 'enum' na frente do tipo NAO e enfeite: BaloonAttackParams tem um MEMBRO
    // chamado 'side' com o mesmo nome do enum, e o membro ganha na busca de nome.
    // Sem o especificador elaborado, o compilador diz que "'side' nao nomeia um tipo".
    // E o mesmo motivo da assinatura estranha em BaloonAttack::GetDirectionFromSide.
    enum BaloonAttackParams::side LadoPeloNome(const std::string& nome) {
        if (nome == "Down")  return BaloonAttackParams::Down;
        if (nome == "Left")  return BaloonAttackParams::Left;
        if (nome == "Right") return BaloonAttackParams::Right;
        if (nome == "Up")    return BaloonAttackParams::Up;
        return BaloonAttackParams::None;   // LerFases ja recusou; rede de seguranca
    }

    std::unique_ptr<AttackParams> CriarParams(const DescricaoDeAtaque& a) {

        // A BaloonAttack faz dynamic_cast para o struct derivado e nao dispara
        // nada se ele nao vier. E a unica estrategia que exige isso, e o bloco
        // "balao" e o que diz a esta funcao qual dos dois construir.
        std::unique_ptr<AttackParams> params;
        if (a.balao) {
            auto balao = std::make_unique<BaloonAttackParams>();
            balao->side = LadoPeloNome(a.balao->lado);
            if (a.balao->spawnAleatorio)    balao->randomSpawn          = *a.balao->spawnAleatorio;
            if (a.balao->centradoNoJogador) balao->centerOnPlayer       = *a.balao->centradoNoJogador;
            if (a.balao->deslocamento)      balao->centerOnPlayerOffset = *a.balao->deslocamento;
            balao->spawnPoints = a.balao->pontosDeSpawn;
            params = std::move(balao);
        }
        else {
            params = std::make_unique<AttackParams>();
        }

        if (a.projeteis)     params->numProjectiles  = *a.projeteis;
        if (a.velocidade)    params->projectileSpeed = *a.velocidade;
        if (a.angulo)        params->angle           = *a.angulo;
        if (a.anguloCentral) params->centralAngle    = *a.anguloCentral;
        if (a.intervalo)     params->creationSpeed   = *a.intervalo;
        return params;
    }

}

bool ConfigurarFasesDeArquivo(Boss* boss, FSMComponent* fsm, const std::string& nomeDoConjunto) {

    if (!boss || !fsm) {
        SDL_Log("FASES: chamada com boss ou fsm nulo para \"%s\".", nomeDoConjunto.c_str());
        return false;
    }

    GarantirArquivoLido();

    const auto it = gConjuntos.find(nomeDoConjunto);
    if (it == gConjuntos.end()) {
        SDL_Log("FASES: nao existe conjunto chamado \"%s\" em %s.",
                nomeDoConjunto.c_str(), Caminho().c_str());
        return false;
    }

    // As transicoes sao conferidas ANTES de registrar qualquer coisa: um
    // conjunto pela metade e pior que nenhum, porque deixa o chefe atacando
    // numa fase e congelado na seguinte.
    const auto problemas = ValidarTransicoes(it->second, nomeDoConjunto);
    if (!problemas.empty()) {
        for (const auto& p : problemas) {
            SDL_Log("FASES em %s: %s", Caminho().c_str(), p.c_str());
        }
        SDL_Log("FASES: o conjunto \"%s\" foi recusado pela validacao. Nada foi registrado.",
                nomeDoConjunto.c_str());
        return false;
    }

    for (const DescricaoDeFase& fase : it->second) {

        for (const DescricaoDeAtaque& a : fase.ataques) {

            ProjectileFactory* spawner = boss->GetProjectileFactory(a.projetil);
            if (!spawner) {
                SDL_Log("FASES: a fase \"%s\" pede a fabrica de projeteis \"%s\", que este chefe "
                        "nao registrou. O ataque foi ignorado.",
                        fase.nome.c_str(), a.projetil.c_str());
                continue;
            }

            auto estrategia = CriarEstrategia(a.estrategia, spawner, boss);
            if (!estrategia) {
                // Nao deveria acontecer: LerFases ja recusou estrategia
                // desconhecida. Fica como rede para o caso de alguem acrescentar
                // um nome na lista da camada pura e esquecer esta funcao.
                SDL_Log("FASES: a estrategia \"%s\" da fase \"%s\" nao tem construtor em "
                        "CriarEstrategia. O ataque foi ignorado.",
                        a.estrategia.c_str(), fase.nome.c_str());
                continue;
            }

            // As regras vem por nome (de regras.json) ou escritas em linha. Sem
            // regras, o projetil sai sem behavior e voa reto - e legitimo, e o
            // caso da fase 1 do Ricardo.
            ProjectileConfigurator configurator = nullptr;
            if (a.temRegrasEmLinha) {
                configurator = ConfiguratorDeRegras(a.regras);
            }
            else if (!a.regrasNome.empty()) {
                configurator = ConfiguratorDeArquivo(a.regrasNome);
            }

            boss->AddAttackPattern(fase.nome, std::move(estrategia), CriarParams(a),
                                   a.cooldown, configurator);
        }

        auto estado = std::make_unique<BossAttackState>(fsm, fase.nome, fase.duracao, fase.proximo);
        fsm->RegisterState(std::move(estado));

        boss->RegisterMovementStrategy(fase.nome, CriarMovimento(fase.movimento));
    }

    SDL_Log("FASES: conjunto \"%s\" montado com %zu fase(s).",
            nomeDoConjunto.c_str(), it->second.size());
    return true;
}
