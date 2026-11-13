// A ponte entre as fases lidas de arquivo e o motor.

#include "FasesDeAtaqueArquivo.h"

#include <map>
#include <memory>
#include <SDL_log.h>

#include "FasesDeAtaque.h"
#include "PastaDoChefe.h"
#include "RegrasDeAtaqueArquivo.h"
#include "AttackParameters/AttackParams.h"
#include "AttackParameters/ConsultaAttackParams.h"
#include "BaseStrategies/ConsultaAttack.h"
#include "BaseStrategies/CaixaAttack.h"
#include "AttackParameters/CaixaAttackParams.h"
#include "BaseStrategies/PontosAttack.h"
#include "AttackParameters/PontosAttackParams.h"
#include "BaseStrategies/TokenAttack.h"
#include "AttackParameters/TokenAttackParams.h"
#include "BaseStrategies/PalavraAttack.h"
#include "AttackParameters/PalavraAttackParams.h"
#include "BaseStrategies/CircleSpreadAttack.h"
#include "BaseStrategies/WaveAttack.h"
#include "../Actors/Teachers/Boss.h"
#include "../Actors/Teachers/BossAttackState.h"
#include "../Components/AIComponents/FSMComponent.h"
#include "../Movements/MovementStrategies.h"

#include "../Motor/CaminhosArquivo.h"

namespace {

    /// O caminho e montado na hora, e nao guardado num const de namespace.
    std::string Caminho(const std::string& chefe) {
        return Caminhos::Asset(PastaDoChefe::Arquivo(chefe, PastaDoChefe::kFases));
    }

    /// Um chefe de cada vez, lido na primeira vez que alguem o pede. Montar o Thiago nao abre mais
    /// os arquivos dos outros quatro.
    std::map<std::string, FasesLidas> gPorChefe;

    const FasesLidas& Lidas(const std::string& chefe) {

        const auto ja = gPorChefe.find(chefe);
        if (ja != gPorChefe.end()) return ja->second;

        FasesLidas lidas;
        const std::string caminho = Caminho(chefe);

        const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
        if (!texto) {
            SDL_Log("FASES: nao foi possivel abrir %s. O chefe \"%s\" nao tera ataque "
                    "nenhum: as fases dele vivem nesse arquivo, e nao ha mais configuracao "
                    "de reserva em C++. Confira se a pasta Assets foi copiada junto com o "
                    "executavel, e se o jogo esta rodando de um diretorio abaixo da raiz "
                    "do projeto.", caminho.c_str(), chefe.c_str());
        } else {
            lidas = LerFases(*texto);

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
        if (tipo == "CircleSpreadAttack") return std::make_unique<CircleSpreadAttack>(spawner, boss);
        if (tipo == "WaveAttack")         return std::make_unique<WaveAttack>(spawner, boss);
        if (tipo == "ConsultaAttack")     return std::make_unique<ConsultaAttack>(spawner, boss);
        if (tipo == "CaixaAttack")        return std::make_unique<CaixaAttack>(spawner, boss);
        if (tipo == "PontosAttack")       return std::make_unique<PontosAttack>(spawner, boss);
        if (tipo == "TokenAttack")        return std::make_unique<TokenAttack>(spawner, boss);
        if (tipo == "PalavraAttack")      return std::make_unique<PalavraAttack>(spawner, boss);
        return nullptr;
    }

    /// Instancia o movimento do chefe.
    std::unique_ptr<IMovementStrategy> CriarMovimento(const DescricaoDeMovimento& m) {
        if (m.tipo == "RandomWander")     return std::make_unique<RandomWanderStrategy>(m.a, m.b);
        if (m.tipo == "HoverAbovePlayer") return std::make_unique<HoverAbovePlayerStrategy>(m.a, m.b);
        if (m.tipo == "Orbitar")          return std::make_unique<OrbitarStrategy>(m.a, m.b);
        if (m.tipo == "Varredura")        return std::make_unique<VarreduraStrategy>(m.a, m.b);
        if (m.tipo == "Mergulho")         return std::make_unique<MergulhoStrategy>(m.a, m.b);
        if (m.tipo == "Escada")           return std::make_unique<EscadaStrategy>(m.a, m.b);
        if (m.tipo == "Deslocamento")     return std::make_unique<DeslocamentoStrategy>(m.a, m.b);
        if (m.tipo == "OndaQuadrada")     return std::make_unique<OndaQuadradaStrategy>(m.a, m.b);
        if (m.tipo == "Conduzido")        return std::make_unique<ConduzidoStrategy>(m.a);
        return std::make_unique<GoToCenterStrategy>();
    }

    /// Monta o AttackParams. So escreve o que o arquivo disse.
    std::unique_ptr<AttackParams> CriarParams(const DescricaoDeAtaque& a) {

        // As estrategias com bloco proprio fazem dynamic_cast para o struct derivado e nao disparam
        // nada se ele nao vier.
        std::unique_ptr<AttackParams> params;
        if (a.consulta) {
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
            // Para todo ataque CaixaAttack, com bloco ou sem.
            auto caixa = std::make_unique<CaixaAttackParams>();
            if (a.caixa && a.caixa->proporcao)         caixa->proporcao         = *a.caixa->proporcao;
            if (a.caixa && a.caixa->meiaAlturaInicial) caixa->meiaAlturaInicial = *a.caixa->meiaAlturaInicial;
            params = std::move(caixa);
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

    // As transicoes sao conferidas antes de registrar qualquer coisa.
    const auto problemas = ValidarTransicoes(lidas.fases, nomeDoConjunto);
    if (!problemas.empty()) {
        for (const auto& p : problemas) {
            SDL_Log("FASES em %s: %s", Caminho(nomeDoConjunto).c_str(), p.c_str());
        }
        SDL_Log("FASES: as fases de \"%s\" foram recusadas pela validacao. "
                "Nada foi registrado.", nomeDoConjunto.c_str());
        return false;
    }

    // "ataquesDe" e resolvido aqui, e nao na camada pura.
    auto ataquesDaFase = [&lidas](const DescricaoDeFase& f) -> const std::vector<DescricaoDeAtaque>& {
        if (f.ataquesDe.empty()) return f.ataques;
        for (const auto& origem : lidas.fases) {
            if (origem.nome == f.ataquesDe) return origem.ataques;
        }
        // A lista vazia deixa a fase sem ataque em vez de dereferenciar nada.
        return f.ataques;
    };

    for (const DescricaoDeFase& fase : lidas.fases) {

        for (const DescricaoDeAtaque& doArquivo : ataquesDaFase(fase)) {

            // O modo da batalha decide os numeros.
            const DescricaoDeAtaque a = NoNivel(doArquivo, boss->NivelDeDificuldade());

            ProjectileFactory* spawner = boss->GetProjectileFactory(a.projetil);
            if (!spawner) {
                SDL_Log("FASES: a fase \"%s\" pede a fabrica de projeteis \"%s\", que este chefe "
                        "nao registrou. O ataque foi ignorado.",
                        fase.nome.c_str(), a.projetil.c_str());
                continue;
            }

            auto estrategia = CriarEstrategia(a.estrategia, spawner, boss);
            if (!estrategia) {
                // Nao deveria acontecer: LerFases ja recusou estrategia desconhecida.
                SDL_Log("FASES: a estrategia \"%s\" da fase \"%s\" nao tem construtor em "
                        "CriarEstrategia. O ataque foi ignorado.",
                        a.estrategia.c_str(), fase.nome.c_str());
                continue;
            }

            // As regras vem por nome (de regras.json) ou escritas em linha.
            ProjectileConfigurator configurator = nullptr;
            if (a.temRegrasEmLinha) {
                configurator = ConfiguratorDeRegras(a.regras);
            }
            else if (!a.regrasNome.empty()) {
                configurator = ConfiguratorDeArquivo(nomeDoConjunto, a.regrasNome);
            }

            // A janela da etapa, se o ataque pediu uma.
            float janelaDe = 0.0f, janelaAte = 1.0e9f;
            if (!a.etapa.empty() && !JanelaDaEtapa(fase.etapas, a.etapa, janelaDe, janelaAte)) {
                SDL_Log("FASES: a fase \"%s\" usa um ataque da etapa \"%s\", que ela nao tem. "
                        "O ataque dispara na fase inteira.", fase.nome.c_str(), a.etapa.c_str());
                janelaDe = 0.0f; janelaAte = 1.0e9f;
            }

            boss->AddAttackPattern(fase.nome, std::move(estrategia), CriarParams(a),
                                   a.cooldown, configurator, janelaDe, janelaAte, a.gatilho, a.variar);
        }

        auto estado = std::make_unique<BossAttackState>(fsm, fase.nome, fase.duracao, fase.proximo);
        fsm->RegisterState(std::move(estado));

        boss->RegisterMovementStrategy(fase.nome, CriarMovimento(NoNivel(fase.movimento, boss->NivelDeDificuldade())));

        // O clone e so uma anotacao aqui.
        if (fase.clone.existe) {
            boss->RegistrarClone(fase.nome, fase.clone.ataquesDe);
        }

        // O anuncio tambem.
        if (!fase.anuncio.empty()) {
            boss->RegistrarAnuncio(fase.nome, fase.anuncio);
        }

        // E as etapas: onde cada uma acaba, e o que o placar diz durante ela.
        if (!fase.etapas.empty()) {
            std::vector<std::pair<float, std::string>> fimETexto;
            float fim = 0.0f;
            for (const auto& e : fase.etapas) {
                fim += e.duracao;
                fimETexto.emplace_back(fim, e.anuncio.empty() ? e.nome : e.anuncio);
            }
            boss->RegistrarEtapas(fase.nome, std::move(fimETexto));
        }

        // E o intercambio, pelo mesmo caminho: quem move os alunos e a batalha.
        if (fase.intercambio) {
            boss->RegistrarIntercambio(fase.nome, fase.intercambio->aCada,
                                       fase.intercambio->aviso);
        }
    }

    SDL_Log("FASES: o chefe \"%s\" foi montado com %zu fase(s).",
            nomeDoConjunto.c_str(), lidas.fases.size());
    return true;
}
