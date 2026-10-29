//
// O flexbox do CSS - onde cada item de uma linha fica, para cada valor de
// justify-content.
//

#pragma once

#include <string>
#include <vector>

#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA (mesma disciplina de Caixa, Fita e Tabela).
 *
 * A fase 2 do Lucas e o CSS, e a grade do DevTools dela (Estilo.h) poe cada
 * linha de cartoes pelo justify-content da regra em vigor. Aqui ficam as contas
 * do flexbox e a varredura do professor, que a fase 3 tambem usa.
 */
namespace Flex {

    /// Os cinco valores de justify-content que a fase usa, com os nomes do CSS.
    enum class Justificar { Inicio, Fim, Centro, EspacoEntre, EspacoEmVolta };

    /**
     * @brief Le o valor pelo nome do CSS: flex-start, flex-end, center,
     * space-between ou space-around. Devolve false para qualquer outro nome.
     */
    bool PeloNome(const std::string& nome, Justificar& saida);

    /// @brief O nome do CSS de volta - e o que o placar escreve.
    std::string Nome(Justificar j);

    /**
     * @brief O centro de cada um dos 'n' itens, de largura 'item', numa linha
     * que vai de 'inicio' a 'fim'. Exatamente as regras do CSS:
     *
     *   flex-start      encostados no inicio, sem espaco entre eles
     *   flex-end        encostados no fim
     *   center          encostados uns nos outros, no meio da linha
     *   space-between   o primeiro no inicio, o ultimo no fim, o resto dividido
     *   space-around    o mesmo espaco em volta de cada um (meio espaco nas pontas)
     *
     * Com um item so, space-between se comporta como flex-start, como no CSS.
     * Sem item, nao ha posicao.
     */
    std::vector<float> Posicoes(Justificar j, int n, float item, float inicio, float fim);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA -------------------------------

    /// @brief Se o arranjo deixa os itens JUNTOS (flex-start, flex-end,
    /// center) - e o livre sao as laterais -, ou espalhados pela linha
    /// (space-between, space-around) - e o livre e o vao entre dois.
    bool Junto(Justificar j);

    /**
     * @brief Para onde a proxima fileira se rearranja, saindo de 'de': do
     * junto para um espalhado, e do espalhado para um junto, sorteado entre
     * os do outro lado. Antes o arquivo alternava sempre center e
     * space-between; a troca do lugar seguro no meio da queda continua - e o
     * que faz a fase -, e qual regra vem nao se decora: quem diz e o placar.
     */
    Justificar Proxima(Variacao::Sorteio& s, Justificar de);

    // ---- A VARREDURA DO PROFESSOR --------------------------------------------

    /// O PROFESSOR VARRE O ALTO DA TELA, de margem a margem, nas fases 2 e 3.
    /// Devagar no Normal - a 170 px/s, como era, o tiro do
    /// aluno (quase 1 s de voo) nao o achava, e ate quem o seguia perdia nota
    /// na fase -, mais depressa no Dificil.
    constexpr float kYDaVarredura = 144.0f;
    constexpr float kMargemDaVarredura = 150.0f;
    constexpr float kVarredura = 90.0f;
    constexpr float kVarreduraNoDificil = 120.0f;

    /// @brief O x do professor 't' segundos depois de comecar a varrer de 'x0':
    /// vai e volta entre as margens, a 'velocidade', comecando para a direita.
    float XDaVarredura(float t, float x0, float velocidade, float largura);
}
