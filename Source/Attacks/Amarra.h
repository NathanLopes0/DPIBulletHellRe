//
// A letra amarrada: ligare, "atar".
//

#pragma once

#include <string>

#include "../Motor/Math.h"

/// Na primeira fase 1 do Luizinho (Amarracoes) a letra da amarracao dinamica nascia amarrada ao
/// professor.
namespace Amarra {

    /// Para que lado a letra gira, como se ve na tela (o y da tela cresce para baixo): no sentido
    /// horario, a letra que esta a direita do no desce.
    enum class Sentido { Horario, AntiHorario };

    /// "horario" ou "antihorario". Devolve false para qualquer outro nome.
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

    /// Um quadro da letra, depois de o RigidBody ter integrado a posicao - a ordem do motor.
    Lance Passo(Corda& corda, Vector2 posicao, Vector2 velocidade, Vector2 no, float dt);
}
