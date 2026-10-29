//
// A letra amarrada: ligare, "atar".
//

#pragma once

#include <string>

#include "../Core/Math.h"

/**
 * CAMADA PURA.
 *
 * Na primeira fase 1 do Luizinho (Amarracoes) a letra da amarracao dinamica
 * nascia AMARRADA ao professor; hoje quem amarra sao os guardrails do Julio,
 * e a fase do Luizinho e o escopo (Escopo.h). A corda que nao se ve vai se
 * desenrolando enquanto a letra se afasta dele, estica, e dali em diante a
 * letra gira em volta do professor, indo junto para onde ele for. Quando o tempo
 * de vida da amarracao acaba, a corda e cortada e a letra sai pela TANGENTE,
 * como a pedra de uma funda.
 *
 * Quem faz isso no jogo e AmarrarBehavior (Behaviors.h); aqui fica a conta de
 * cada quadro, para o teste poder rodar a corda sem cena nem SDL.
 *
 * TUDO E MEDIDO A PARTIR DO NO, e nao da tela. A primeira versao girava a letra
 * pela tangente no espaco da tela e so a punha de volta na circunferencia: com
 * o professor varrendo a 200 px/s e a letra a 200 px/s, a roda virava um arco
 * arrastado atras dele, e a letra que saia para a frente nunca se afastava o
 * bastante para a corda esticar. Guardando o angulo e o quanto de corda ja saiu,
 * a roda e uma roda com o professor parado ou andando.
 *
 * O NO ENTRA COMO PARAMETRO, a cada quadro, e por isso a amarracao e DINAMICA:
 * quem a usa decide a cada vez onde o no esta.
 */
namespace Amarra {

    /// Para que lado a letra gira, como se ve NA TELA (o y da tela cresce para
    /// baixo): no sentido horario, a letra que esta a direita do no desce.
    enum class Sentido { Horario, AntiHorario };

    /// @brief "horario" ou "antihorario". Devolve false para qualquer outro nome.
    bool SentidoPeloNome(const std::string& nome, Sentido& sentido);

    struct Corda {
        float comprimento = 0.0f;       ///< pixels entre o no e a letra, esticada
        float duracao = 0.0f;           ///< segundos que a letra fica amarrada
        Sentido sentido = Sentido::Horario;

        float solta = -1.0f;            ///< corda ja desenrolada; negativo = ainda nao comecou
        float angulo = 0.0f;            ///< radianos, a partir do no, no y da tela
        float amarrada = -1.0f;         ///< segundos desde que esticou; negativo = frouxa
        bool cortada = false;           ///< o tempo acabou e a letra seguiu reta
    };

    /// O que aconteceu neste quadro, para quem precisa trocar a animacao.
    enum class Evento { Nenhum, Amarrou, Cortou };

    struct Lance {
        Vector2 posicao;
        Vector2 velocidade;
        Evento evento = Evento::Nenhum;
    };

    /**
     * @brief Um quadro da letra, DEPOIS de o RigidBody ter integrado a posicao -
     * a ordem do motor.
     *
     * A CORDA NASCE NO NO: no primeiro quadro ela pega a direcao em que a letra
     * voa, e a letra e posta a um quadro de voo do no, venha de onde vier - quem
     * usa isto dispara a letra de cima do professor. Enquanto a corda desenrola,
     * a letra se afasta do no nessa direcao, no modulo em que veio; esticada, ela gira em volta do no com esse mesmo modulo (o contrato
     * de toda Motion). A posicao e sempre o no mais a corda, entao o que o
     * RigidBody fez no quadro e substituido - ele so conta depois do corte.
     * Passada a 'duracao', a corda e cortada e a letra sai pela tangente.
     *
     * Uma letra PARADA nao e amarrada: sem modulo nao ha com que girar, e uma
     * letra adormecida de uma WaveAttack ficaria presa ao no sem nunca andar.
     */
    Lance Passo(Corda& corda, Vector2 posicao, Vector2 velocidade, Vector2 no, float dt);
}
