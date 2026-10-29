//
// A hora do sistema, isolada num lugar so.
//

#pragma once

#include <ctime>
#include <string>

/**
 * O relogio e a unica coisa do mundo externo de que a ficha do aluno precisa, e
 * por isso ele mora aqui em vez de dentro dela: Progresso e Ficha continuam puros
 * e recebem a data ja pronta, como texto.
 *
 * Formatar e PURA e e testada; Agora so le o relogio e chama Formatar.
 */
namespace Relogio {

    /**
     * @brief Um instante como "2026-10-02 14:30:00", em hora LOCAL.
     *
     * Texto, e nao numero, por dois motivos: o save fica legivel por quem abrir no
     * editor, e esta forma ordena alfabeticamente na mesma ordem em que ordena
     * cronologicamente - entao o ranking do professor pode ordenar por data sem
     * converter nada.
     *
     * Hora local e nao UTC porque quem le e o professor, na mesma sala da maquina.
     */
    std::string Formatar(std::time_t instante);

    /// @brief Agora, no formato de Formatar.
    std::string Agora();
}
