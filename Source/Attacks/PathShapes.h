//
// Formas de caminho prontas para o PathBehavior.
//

#pragma once

#include <vector>
#include <memory>
#include <map>
#include <string>
#include "../Math.h"

/**
 * @namespace PathShapes
 * @brief Geradores de caminhos para o PathBehavior.
 *
 * ESPAÇO DE CAMINHO
 * Todas as funções devolvem offsets, não coordenadas de tela:
 *   +X = a direção em que o projétil viaja quando o PathBehavior ativa
 *   +Y = a perpendicular, à direita desse movimento
 *
 * Ou seja, você desenha o caminho como se o projétil sempre saísse para a
 * direita, e o PathBehavior gira tudo para a direção real do disparo. Isso
 * significa que a mesma forma funciona em qualquer ângulo, e que um anel de
 * CircleSpreadAttack produz a forma girada para cada direção do anel.
 *
 * NÚMERO DE SEGMENTOS
 * Mais segmentos deixam a curva mais suave, mas cada waypoint é um alvo que o
 * projétil precisa alcançar dentro de uma tolerância. Muitos pontos muito
 * próximos fazem o projétil gastar frames "chegando" em cada um e a trajetória
 * fica truncada. Entre 8 e 16 costuma ser o ponto certo.
 *
 * LEGIBILIDADE ANTES DE TUDO
 * Caminho elaborado vira bala imprevisível, e bala imprevisível é injusta, não
 * difícil. Prefira uma forma que o jogador consiga descrever depois de ver
 * duas vezes.
 */
namespace PathShapes {

    /**
     * @brief Forma COMPARTILHADA entre projeteis (Flyweight).
     *
     * Antes cada projetil recebia uma copia do vector de waypoints. Num anel de
     * 300 projeteis com 12 pontos cada, eram 300 alocacoes de conteudo
     * identico, uma por tiro - o que e ironico num projeto cujo tema e nao
     * realocar objetos.
     *
     * Agora as funcoes abaixo memoizam por parametro: chamar Loop(85,500,12)
     * mil vezes devolve mil vezes o MESMO ponteiro. A forma e const, entao o
     * compartilhamento e seguro; quem varia por projetil (origem, rotacao,
     * velocidade) vive no PathBehavior, nao na forma.
     */
    using Path = std::shared_ptr<const std::vector<Vector2>>;


    /**
     * @brief Arco: avança enquanto faz uma barriga para um dos lados e volta
     * ao eixo no fim. Lê como um tiro que "contorna" alguma coisa.
     *
     * @param forward Distância total percorrida na direção do disparo.
     * @param lateral Largura da barriga. Negativo faz a curva para o outro
     *        lado — alterne o sinal entre projéteis vizinhos para que os arcos
     *        se cruzem.
     * @param segments Quantidade de pontos da curva.
     */
    /**
     * @brief Reta: um unico waypoint a frente.
     *
     * Combinada com Mira(MirarNoJogador), reproduz um homing. E o caso mais
     * simples possivel de caminho, e existe justamente para mostrar que um
     * "comportamento reativo" e so uma forma trivial com referencial dinamico.
     *
     * @param distance Quao longe fica o waypoint. Precisa passar da borda da
     *        tela, senao o projetil para de seguir antes de sair dela.
     */
    Path Reta(float distance = 1200.f);

    Path Arc(float forward, float lateral, int segments = 10);

    /**
     * @brief Laço: o projétil dá uma volta completa e retoma o rumo original.
     *
     * É a forma mais legível do conjunto — o jogador vê a bala girar e entende
     * imediatamente que ela vai voltar. Boa para telegrafar um ataque lento.
     *
     * @param radius Raio do laço.
     * @param exitDistance Quanto avança em linha reta depois de fechar a volta.
     * @param segments Pontos usados para desenhar o círculo.
     */
    Path Loop(float radius, float exitDistance = 400.f, int segments = 12);

    /**
     * @brief Ziguezague: avança alternando para os dois lados.
     *
     * Diferente do WobbleBehavior, aqui as quinas são exatas e previsíveis,
     * então o jogador consegue ler o padrão e se posicionar.
     *
     * @param step Avanço por perna do zigue-zague.
     * @param amplitude Deslocamento lateral de cada perna.
     * @param legs Quantidade de pernas.
     */
    Path Zigzag(float step, float amplitude, int legs = 5);


    /**
     * @brief Resultado da leitura de um arquivo de formas.
     *
     * Os problemas vem separados em vez de virarem log aqui dentro: assim a
     * leitura continua PURA e testavel, e quem chama decide como reportar.
     * Cada problema e uma frase pronta dizendo qual forma e qual campo.
     */
    struct FormasLidas {
        std::map<std::string, std::vector<Vector2>> formas;
        std::vector<std::string> problemas;
    };

    /**
     * @brief Le formas a partir do TEXTO de um arquivo JSON. Funcao PURA:
     * nao abre arquivo, nao escreve log.
     *
     * Formato aceito, com as duas maneiras de descrever uma forma:
     *
     *   {
     *     "Laco":     { "gerador": {"tipo":"Loop","a":70,"b":500,"n":12} },
     *     "Serpente": { "pontos": [[60,-40],[120,-50],[160,-20]] }
     *   }
     *
     * "gerador" reaproveita as formas que ja existem em C++ (Reta, Arc, Loop,
     * Zigzag), com os parametros na ordem em que a funcao os recebe. "pontos" e
     * a lista literal, em espaco de caminho: +X e a direcao de disparo.
     *
     * Uma forma invalida e ignorada e vira uma frase em 'problemas' - o arquivo
     * inteiro nunca e descartado por causa de uma entrada errada.
     */
    FormasLidas LerFormas(const std::string& textoJson);

    /**
     * @brief Devolve, pelo nome, uma forma definida em Assets/Paths/formas.json.
     *
     * O arquivo e lido UMA vez, na primeira chamada, e as formas entram no mesmo
     * cache Flyweight das formas em codigo.
     *
     * Arquivo ausente, JSON malformado ou nome inexistente devolvem uma reta e
     * escrevem UMA linha no log dizendo exatamente o que faltou - nunca derrubam
     * o jogo nem fazem o ataque sumir em silencio. E a mesma filosofia do
     * placeholder de textura.
     */
    Path DoArquivo(const std::string& nome);

    /** @brief Quantas formas distintas estao no cache. So para diagnostico. */
    size_t CachedShapeCount();
}
