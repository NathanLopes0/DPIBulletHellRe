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
#include "PastaDoChefe.h"
#include "RegrasDeAtaqueArquivo.h"
#include "AttackParameters/AttackParams.h"
#include "AttackParameters/BaloonAttackParams.h"
#include "AttackParameters/ConsultaAttackParams.h"
#include "BaseStrategies/AngledAttack.h"
#include "BaseStrategies/BaloonAttack.h"
#include "BaseStrategies/ConsultaAttack.h"
#include "BaseStrategies/CaixaAttack.h"
#include "AttackParameters/CaixaAttackParams.h"
#include "BaseStrategies/FlexAttack.h"
#include "AttackParameters/FlexAttackParams.h"
#include "BaseStrategies/CapsulaAttack.h"
#include "AttackParameters/CapsulaAttackParams.h"
#include "BaseStrategies/SerpenteAttack.h"
#include "AttackParameters/SerpenteAttackParams.h"
#include "BaseStrategies/SprintAttack.h"
#include "AttackParameters/SprintAttackParams.h"
#include "BaseStrategies/PontosAttack.h"
#include "AttackParameters/PontosAttackParams.h"
#include "BaseStrategies/TokenAttack.h"
#include "AttackParameters/TokenAttackParams.h"
#include "BaseStrategies/PalavraAttack.h"
#include "AttackParameters/PalavraAttackParams.h"
#include "BaseStrategies/CircleSpreadAttack.h"
#include "BaseStrategies/LaserAttack.h"
#include "BaseStrategies/WaveAttack.h"
#include "../Actors/Teachers/Boss.h"
#include "../Actors/Teachers/BossAttackState.h"
#include "../Components/AIComponents/FSMComponent.h"
#include "../Movements/MovementStrategies.h"

#include "../CaminhosArquivo.h"

namespace {

    /// O caminho e montado na HORA, e nao guardado num const de namespace: um
    /// const std::string em escopo de namespace e construido ANTES do main,
    /// portanto antes de Caminhos::Inicializar() descobrir a pasta base, e
    /// capturaria o palpite de reserva ("..") - o arquivo passaria a ser
    /// procurado relativo ao diretorio de onde o jogo foi lancado, que e
    /// exatamente o problema que a resolucao de caminhos veio resolver.
    std::string Caminho(const std::string& chefe) {
        return Caminhos::Asset(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kFases));
    }

    /// UM CHEFE DE CADA VEZ, lido na primeira vez que alguem o pede. Montar o
    /// Thiago nao abre mais os arquivos dos outros quatro.
    std::map<std::string, FasesLidas> gPorChefe;

    const FasesLidas& Lidas(const std::string& chefe) {

        const auto ja = gPorChefe.find(chefe);
        if (ja != gPorChefe.end()) return ja->second;

        FasesLidas lidas;
        const std::string caminho = Caminho(chefe);

        std::ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            SDL_Log("FASES: nao foi possivel abrir %s. O chefe \"%s\" nao tera ataque "
                    "nenhum: as fases dele vivem nesse arquivo, e nao ha mais configuracao "
                    "de reserva em C++. Confira se a pasta Assets foi copiada junto com o "
                    "executavel, e se o jogo esta rodando de um diretorio abaixo da raiz "
                    "do projeto.", caminho.c_str(), chefe.c_str());
        } else {
            const std::string texto((std::istreambuf_iterator<char>(arquivo)),
                                     std::istreambuf_iterator<char>());
            lidas = LerFases(texto);

            for (const auto& p : lidas.problemas) {
                SDL_Log("FASES em %s: %s", caminho.c_str(), p.c_str());
            }
        }

        return gPorChefe.emplace(chefe, std::move(lidas)).first->second;
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
        if (tipo == "ConsultaAttack")     return std::make_unique<ConsultaAttack>(spawner, boss);
        if (tipo == "CaixaAttack")        return std::make_unique<CaixaAttack>(spawner, boss);
        if (tipo == "FlexAttack")         return std::make_unique<FlexAttack>(spawner, boss);
        if (tipo == "CapsulaAttack")      return std::make_unique<CapsulaAttack>(spawner, boss);
        if (tipo == "SerpenteAttack")     return std::make_unique<SerpenteAttack>(spawner, boss);
        if (tipo == "SprintAttack")       return std::make_unique<SprintAttack>(spawner, boss);
        if (tipo == "PontosAttack")       return std::make_unique<PontosAttack>(spawner, boss);
        if (tipo == "TokenAttack")        return std::make_unique<TokenAttack>(spawner, boss);
        if (tipo == "PalavraAttack")      return std::make_unique<PalavraAttack>(spawner, boss);
        return nullptr;
    }

    /**
     * Instancia o movimento do chefe. 'a' e 'b' da descricao entram na ordem em
     * que cada construtor os recebe:
     *   RandomWander     a = intervalo entre destinos (s), b = velocidade (px/s)
     *   HoverAbovePlayer a = velocidade (px/s),          b = altura acima do jogador
     *   GoToCenter       nao usa nenhum dos dois
     *   Orbitar          a = raio (px),                  b = voltas por segundo
     *   Varredura        a = velocidade (px/s),          b = margem das bordas (px)
     *   Mergulho         a = velocidade (px/s),          b = profundidade (px)
     *   Escada           a = segundos por degrau,         b = quantos degraus
     *   Deslocamento     a = segundos por passo,          b = quantas colunas
     */
    std::unique_ptr<IMovementStrategy> CriarMovimento(const DescricaoDeMovimento& m) {
        if (m.tipo == "RandomWander")     return std::make_unique<RandomWanderStrategy>(m.a, m.b);
        if (m.tipo == "HoverAbovePlayer") return std::make_unique<HoverAbovePlayerStrategy>(m.a, m.b);
        if (m.tipo == "Orbitar")          return std::make_unique<OrbitarStrategy>(m.a, m.b);
        if (m.tipo == "Varredura")        return std::make_unique<VarreduraStrategy>(m.a, m.b);
        if (m.tipo == "Mergulho")         return std::make_unique<MergulhoStrategy>(m.a, m.b);
        if (m.tipo == "Escada")           return std::make_unique<EscadaStrategy>(m.a, m.b);
        if (m.tipo == "Deslocamento")     return std::make_unique<DeslocamentoStrategy>(m.a, m.b);
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
        else if (a.consulta) {
            auto consulta = std::make_unique<ConsultaAttackParams>();
            consulta->eixo = (a.consulta->eixo == "Coluna")
                                 ? ConsultaAttackParams::Eixo::Coluna
                                 : ConsultaAttackParams::Eixo::Linha;
            if (a.consulta->linhas)    consulta->forma.linhas  = *a.consulta->linhas;
            if (a.consulta->colunas)   consulta->forma.colunas = *a.consulta->colunas;
            if (a.consulta->indice)    consulta->indice        = *a.consulta->indice;
            if (a.consulta->aviso)     consulta->aviso         = *a.consulta->aviso;
            if (a.consulta->invertido) consulta->invertido     = *a.consulta->invertido;
            params = std::move(consulta);
        }
        else if (a.estrategia == "CaixaAttack") {
            // PARA TODO ataque CaixaAttack, com bloco ou sem: a estrategia faz
            // dynamic_cast e nao dispara sem esta struct, e o bloco e opcional.
            auto caixa = std::make_unique<CaixaAttackParams>();
            if (a.caixa && a.caixa->proporcao)         caixa->proporcao         = *a.caixa->proporcao;
            if (a.caixa && a.caixa->meiaAlturaInicial) caixa->meiaAlturaInicial = *a.caixa->meiaAlturaInicial;
            params = std::move(caixa);
        }
        else if (a.estrategia == "CapsulaAttack") {
            // Como a caixa: para todo ataque CapsulaAttack, com bloco ou sem.
            auto capsula = std::make_unique<CapsulaAttackParams>();
            if (a.capsula && a.capsula->abre)       capsula->abre                 = *a.capsula->abre;
            if (a.capsula && a.capsula->velocidade) capsula->velocidadeDaAbertura = *a.capsula->velocidade;
            if (a.capsula)                          capsula->escreve              = a.capsula->escreve;
            params = std::move(capsula);
        }
        else if (a.estrategia == "SerpenteAttack") {
            auto serpente = std::make_unique<SerpenteAttackParams>();
            if (a.serpente && a.serpente->espaco)  serpente->espaco  = *a.serpente->espaco;
            if (a.serpente && a.serpente->giro)    serpente->giro    = *a.serpente->giro;
            if (a.serpente && a.serpente->duracao) serpente->duracao = *a.serpente->duracao;
            if (a.serpente) { serpente->cabeca = a.serpente->cabeca; serpente->corpo = a.serpente->corpo; }
            params = std::move(serpente);
        }
        else if (a.estrategia == "SprintAttack") {
            auto sprint = std::make_unique<SprintAttackParams>();
            if (a.sprint && a.sprint->revisao) sprint->revisao = *a.sprint->revisao;
            if (a.sprint) {
                sprint->historia = a.sprint->historia;
                sprint->cursor   = a.sprint->cursor;
                sprint->clique   = a.sprint->clique;
            }
            params = std::move(sprint);
        }
        else if (a.estrategia == "TokenAttack") {
            auto tokens = std::make_unique<TokenAttackParams>();
            if (a.tokens && a.tokens->temperatura) tokens->temperatura = *a.tokens->temperatura;
            if (a.tokens && a.tokens->candidatos)  tokens->candidatos  = *a.tokens->candidatos;
            if (a.tokens && a.tokens->abertura)    tokens->abertura    = *a.tokens->abertura;
            if (a.tokens) tokens->frase = a.tokens->frase;
            params = std::move(tokens);
        }
        else if (a.estrategia == "PalavraAttack") {
            auto palavra = std::make_unique<PalavraAttackParams>();
            if (a.palavra) {
                if (a.palavra->valor)  palavra->palavra = *a.palavra->valor;
                if (a.palavra->altura) palavra->altura  = *a.palavra->altura;
                if (!a.palavra->zero.empty()) palavra->zero = a.palavra->zero;
                if (!a.palavra->um.empty())   palavra->um   = a.palavra->um;
                palavra->umAtravessa = a.palavra->todosAtravessam;
            }
            params = std::move(palavra);
        }
        else if (a.estrategia == "PontosAttack") {
            auto pontos = std::make_unique<PontosAttackParams>();
            if (a.pontos) {
                pontos->posto = a.pontos->posto;
                pontos->mirar = a.pontos->mirar;
                pontos->prever = a.pontos->prever;
            }
            params = std::move(pontos);
        }
        else if (a.flex) {
            auto flex = std::make_unique<FlexAttackParams>();
            Flex::PeloNome(a.flex->de, flex->de);       // LerFases ja recusou nome errado
            Flex::PeloNome(a.flex->para, flex->para);
            if (a.flex->aos)     flex->aos     = *a.flex->aos;
            if (a.flex->em)      flex->em      = *a.flex->em;
            if (a.flex->item)    flex->item    = *a.flex->item;
            if (a.flex->largura) flex->largura = *a.flex->largura;
            params = std::move(flex);
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

    const FasesLidas& lidas = Lidas(nomeDoConjunto);

    if (lidas.fases.empty()) {
        SDL_Log("FASES: %s nao trouxe nenhuma fase.", Caminho(nomeDoConjunto).c_str());
        return false;
    }

    // As transicoes sao conferidas ANTES de registrar qualquer coisa: um
    // conjunto pela metade e pior que nenhum, porque deixa o chefe atacando
    // numa fase e congelado na seguinte.
    const auto problemas = ValidarTransicoes(lidas.fases, nomeDoConjunto);
    if (!problemas.empty()) {
        for (const auto& p : problemas) {
            SDL_Log("FASES em %s: %s", Caminho(nomeDoConjunto).c_str(), p.c_str());
        }
        SDL_Log("FASES: as fases de \"%s\" foram recusadas pela validacao. "
                "Nada foi registrado.", nomeDoConjunto.c_str());
        return false;
    }

    // "ataquesDe" e resolvido AQUI, e nao na camada pura: la a fase que
    // empresta fica com a lista vazia de proposito, para haver UMA fonte de
    // verdade. Quem monta precisa da lista de verdade, e e aqui que ela e
    // procurada - uma vez por fase, e so no momento de registrar.
    auto ataquesDaFase = [&lidas](const DescricaoDeFase& f) -> const std::vector<DescricaoDeAtaque>& {
        if (f.ataquesDe.empty()) return f.ataques;
        for (const auto& origem : lidas.fases) {
            if (origem.nome == f.ataquesDe) return origem.ataques;
        }
        // ValidarTransicoes ja recusou o conjunto se o nome nao existisse, entao
        // chegar aqui seria bug de codigo e nao de arquivo. A lista vazia deixa
        // a fase sem ataque em vez de dereferenciar nada.
        return f.ataques;
    };

    for (const DescricaoDeFase& fase : lidas.fases) {

        for (const DescricaoDeAtaque& a : ataquesDaFase(fase)) {

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
                configurator = ConfiguratorDeArquivo(nomeDoConjunto, a.regrasNome);
            }

            // A JANELA DA ETAPA, se o ataque pediu uma. LerFases ja recusou a
            // etapa que a fase nao tem; um ataque emprestado ("ataquesDe") que
            // pedisse a etapa de outra fase cai aqui na fase inteira.
            float janelaDe = 0.0f, janelaAte = 1.0e9f;
            if (!a.etapa.empty() && !JanelaDaEtapa(fase.etapas, a.etapa, janelaDe, janelaAte)) {
                SDL_Log("FASES: a fase \"%s\" usa um ataque da etapa \"%s\", que ela nao tem. "
                        "O ataque dispara na fase inteira.", fase.nome.c_str(), a.etapa.c_str());
                janelaDe = 0.0f; janelaAte = 1.0e9f;
            }

            boss->AddAttackPattern(fase.nome, std::move(estrategia), CriarParams(a),
                                   a.cooldown, configurator, janelaDe, janelaAte, a.gatilho);
        }

        auto estado = std::make_unique<BossAttackState>(fsm, fase.nome, fase.duracao, fase.proximo);
        fsm->RegisterState(std::move(estado));

        boss->RegisterMovementStrategy(fase.nome, CriarMovimento(fase.movimento));

        // O CLONE E SO UMA ANOTACAO AQUI. Quem o cria e a batalha, no momento em
        // que a fase comeca - antes disso nao ha onde pôr um segundo corpo, e
        // criar um que fica escondido por duas fases seria pagar por ele a luta
        // inteira.
        if (fase.clone.existe) {
            boss->RegistrarClone(fase.nome, fase.clone.ataquesDe);
        }

        // O ANUNCIO TAMBEM. Quem o mostra e a batalha, que pergunta ao chefe em
        // que fase ele esta - ver Battle::AtualizarPlacar.
        if (!fase.anuncio.empty()) {
            boss->RegistrarAnuncio(fase.nome, fase.anuncio);
        }

        // E AS ETAPAS: onde cada uma acaba, e o que o placar diz durante ela.
        if (!fase.etapas.empty()) {
            std::vector<std::pair<float, std::string>> fimETexto;
            float fim = 0.0f;
            for (const auto& e : fase.etapas) {
                fim += e.duracao;
                fimETexto.emplace_back(fim, e.anuncio.empty() ? e.nome : e.anuncio);
            }
            boss->RegistrarEtapas(fase.nome, std::move(fimETexto));
        }

        // E O INTERCAMBIO, pelo mesmo caminho: quem move os alunos e a batalha.
        if (fase.intercambio) {
            boss->RegistrarIntercambio(fase.nome, fase.intercambio->aCada,
                                       fase.intercambio->aviso);
        }
    }

    SDL_Log("FASES: o chefe \"%s\" foi montado com %zu fase(s).",
            nomeDoConjunto.c_str(), lidas.fases.size());
    return true;
}
