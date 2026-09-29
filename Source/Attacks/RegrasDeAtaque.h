//
// Regras de configuracao de projetil, lidas de texto JSON.
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Math.h"
#include "PathAim.h"

/**
 * CAMADA PURA (mesma disciplina da fase 2)
 *
 * Este arquivo nao abre arquivo, nao escreve log, nao conhece Projectile nem
 * SDL. Ele so transforma TEXTO em descricoes. Quem pega essas descricoes e
 * constroi behaviors de verdade e RegrasDeAtaqueArquivo.cpp, que e a ponte fina
 * com o motor.
 *
 * A separacao existe pelo mesmo motivo de sempre: aqui mora toda a logica que
 * pode errar, entao aqui e onde os testes precisam alcancar.
 */

/**
 * @brief O que inserir num projetil, descrito em dados.
 *
 * Um struct unico com campos nomeados, em vez de uma lista opaca de floats: ler
 * `d.forca` num depurador diz mais do que `d.params[2]`. Cada tipo de behavior
 * usa um subconjunto dos campos e ignora o resto.
 */
struct DescricaoDeBehavior {
    std::string tipo;               ///< "Path", "Tracking", "SlowDown", ...

    std::string forma;              ///< Path: nome da forma (codigo ou arquivo)
    std::string mira;               ///< Path: AlinharComVelocidade | MirarNoJogador | MirarPrevendo
    float parametroDaMira = 0.0f;   ///< Path: segundos de antecipacao, para MirarPrevendo

    float velocidade = 0.0f;        ///< Path
    float atraso = 0.0f;            ///< todos
    float forca = 0.0f;             ///< Tracking
    float duracao = 0.0f;           ///< Tracking, Wobble
    float amplitude = 0.0f;         ///< Wobble: graus
    float frequencia = 0.0f;        ///< Wobble
    float fator = 1.0f;             ///< Accelerate, SlowDown
    Vector2 velocidadeInicial;      ///< Activate

    RitmoDeCiclos ritmo;            ///< MiraPeriodica, PulsoDeVelocidade
    float moduloInvestida = 0.0f;   ///< PulsoDeVelocidade
    float moduloPausa = 0.0f;       ///< PulsoDeVelocidade
};

/**
 * @brief Uma regra: a quem se aplica, e o que fazer com quem se encaixa.
 *
 * O formato espelha a divisao da fase 1 de proposito - `motion` no singular,
 * `modifiers` no plural. O que la e garantido pelo compilador, aqui precisa ser
 * verificado na leitura, porque um arquivo de texto nao passa por compilador
 * nenhum.
 */
struct Regra {
    enum Condicao {
        Sempre,    ///< vale para todo projetil da rajada
        Chance,    ///< sorteio por projetil
        Indices,   ///< posicoes especificas dentro da rajada
        Pares,     ///< indices 0, 2, 4...
        Impares    ///< indices 1, 3, 5...
    };

    Condicao condicao = Sempre;
    float chance = 1.0f;
    std::vector<int> indices;

    bool temMotion = false;
    DescricaoDeBehavior motion;
    std::vector<DescricaoDeBehavior> modifiers;

    std::string animacao;   ///< vazio = nao mexe na animacao

    /// Multiplicador de tamanho do sprite E do colisor. Ausente = nao mexe.
    ///
    /// E optional, e nao um zero sentinela, porque escala zero nao quer dizer
    /// "deixa como esta" - quer dizer um projetil invisivel. O projetil volta do
    /// pool com a escala de fabrica (MarcarPadraoDeFabrica), entao uma regra que
    /// nao diz nada sobre escala deixa o projetil no tamanho certo por si.
    std::optional<float> escala;
};

/**
 * @brief Conjuntos de regras lidos de um arquivo, cada um com um nome.
 *
 * Os problemas vem separados, como em PathShapes::LerFormas: a leitura fica
 * pura e quem chama decide como reportar. Cada problema e uma frase pronta
 * dizendo qual conjunto, qual regra e o que esta errado.
 */
struct RegrasLidas {
    std::map<std::string, std::vector<Regra>> conjuntos;
    std::vector<std::string> problemas;
};

/// @brief O tipo nomeado e uma Motion? (Path, Tracking, Wobble, MiraPeriodica)
bool EhMotion(const std::string& tipo);

/// @brief O tipo nomeado e um Modifier? (Accelerate, SlowDown, Activate,
/// Deactivate, PulsoDeVelocidade)
bool EhModifier(const std::string& tipo);

/**
 * @brief Le conjuntos de regras a partir do TEXTO de um arquivo JSON. PURA.
 *
 * Formato:
 * {
 *   "julio_fase2": [
 *     { "indices": [1],   "motion": {"tipo":"Tracking","atraso":0.3,"forca":2.4,"duracao":2.4} },
 *     { "indices": [0,2], "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} }
 *   ]
 * }
 *
 * Uma regra invalida e descartada e vira uma frase em 'problemas'; o conjunto e
 * o arquivo continuam valendo. Um conjunto so e descartado se nem sequer for
 * uma lista.
 */
RegrasLidas LerRegras(const std::string& textoJson);

/**
 * @brief Le UMA lista de regras a partir do texto de um array JSON. PURA.
 *
 * O mesmo laco que LerRegras roda para cada conjunto, exposto para quem ja tem
 * um array em maos - e o caso das regras escritas em linha dentro de um ataque
 * de fases.json:
 *
 *     { "estrategia": "AngledAttack", "regras": [ {"motion": ...} ] }
 *
 * Existe para que aquele caminho NAO seja um segundo interpretador de regras. Um
 * segundo interpretador aceitaria coisas ligeiramente diferentes do primeiro, e
 * a diferenca apareceria como um behavior que funciona em regras.json e e
 * ignorado em linha, sem mensagem de erro.
 *
 * 'onde' e o prefixo das frases de problema (que fase, que ataque). Os problemas
 * sao ACRESCENTADOS a lista recebida.
 */
std::vector<Regra> LerListaDeRegras(const std::string& textoDaLista,
                                    const std::string& onde,
                                    std::vector<std::string>& problemas);

/**
 * @brief A regra vale para o projetil de indice 'indice'? PURA.
 *
 * O sorteio entra como PARAMETRO em vez de ser feito aqui dentro. E o que torna
 * uma regra probabilistica testavel: o teste passa 0.1 e 0.9 e verifica os dois
 * lados, sem depender de gerador de numeros aleatorios.
 */
bool RegraSeAplica(const Regra& regra, int indice, float sorteio);
