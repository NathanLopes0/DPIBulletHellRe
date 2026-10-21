//
// O coletor de lixo: memento mori, "lembra-te de que morreras".
//

#pragma once

#include <utility>
#include <vector>

#include "../Math.h"

/**
 * CAMADA PURA.
 *
 * Na fase 3 do Luizinho (Gerencia de memoria) os projeteis sao BLOCOS DE
 * MEMORIA: o professor os aloca, eles param no campo e ficam. De tempos em
 * tempos o coletor PARA O MUNDO, marca o que ninguem alcanca e varre. Quem faz
 * isso no jogo e Luizinho; aqui fica o que pode errar:
 *
 *   - o GRAFO DE REFERENCIAS: um bloco aponta para outro, e o professor (a
 *     raiz) aponta para um bloco, quando estao a menos de 'alcance' pixels. Um
 *     bloco e VIVO se ha um caminho da raiz ate ele; os outros sao LIXO, mesmo
 *     que apontem uns para os outros - e o que um coletor de marcar e varrer
 *     sabe fazer e uma contagem de referencias nao sabe;
 *   - o RELOGIO do coletor: quando o mundo para e quando a varredura acontece.
 */
namespace Coletor {

    /// O indice da raiz numa aresta: o professor, e nao um bloco.
    inline constexpr int kRaiz = -1;

    struct Grafo {
        std::vector<bool> vivo;                    ///< por bloco: alcancado da raiz?
        std::vector<std::pair<int, int>> arestas;  ///< todas as referencias; kRaiz e o professor
    };

    /**
     * @brief As referencias entre os blocos e a raiz, e quem esta vivo.
     *
     * Duas pontas a menos de 'alcance' (inclusive) se referenciam. As arestas
     * saem em ordem: primeiro as da raiz, depois as de bloco com bloco, cada
     * par uma vez so, o menor indice primeiro - quem desenha nao depende da
     * ordem, mas o teste depende.
     */
    Grafo Referencias(const std::vector<Vector2>& blocos, Vector2 raiz, float alcance);

    /// O mundo roda enquanto o professor aloca, e para enquanto o coletor marca.
    enum class Mundo { Rodando, Parado };

    /// O que mudou neste quadro.
    enum class Evento { Nenhum, Para, Varre };

    /**
     * @brief O relogio: um ciclo de 'aCada' segundos, cujos ultimos 'pausa'
     * segundos sao o mundo parado. A varredura e o fim da pausa, e o ciclo
     * recomeca.
     */
    struct Relogio {
        float aCada = 0.0f;
        float pausa = 0.0f;
        float t = 0.0f;
    };

    /// @brief Um relogio que funcione: ciclo positivo e pausa entre zero e o ciclo.
    bool Valido(const Relogio& r);

    /// @brief Em que estado o mundo esta agora.
    Mundo Estado(const Relogio& r);

    /**
     * @brief Anda 'dt' segundos. Devolve Para no quadro em que o mundo para e
     * Varre no quadro em que a pausa acaba; um quadro so ve um dos dois, mesmo
     * que 'dt' seja maior que a pausa - a varredura fica para o quadro
     * seguinte, e o mundo nunca deixa de ter parado antes de varrer.
     */
    Evento Avancar(Relogio& r, float dt);
}
