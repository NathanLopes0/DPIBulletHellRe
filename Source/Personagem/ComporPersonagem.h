//
// Monta as camadas da personagem numa textura unica.
//

#pragma once

#include <string>

#include "Personagens.h"

class Game;

/**
 * A ponte entre a REGRA de composicao - que vive em Source/Personagens, e puro,
 * e testado - e os pixels. Aqui se abre PNG, se multiplica canal e se cria
 * textura; por isso este arquivo fica de fora do alvo de testes.
 *
 * POR QUE UMA TEXTURA SO, e nao uma componente de desenho por camada: o Player
 * chama GetComponent<DrawAnimatedComponent>() em varios lugares - o piscar da
 * invencibilidade, a troca entre Idle e Moving, a largura que vira raio do
 * colisor. Com uma componente por camada, cada uma dessas chamadas pegaria so a
 * primeira e as outras ficariam para tras, visiveis e erradas.
 *
 * Composta, a personagem e uma sprite animada como qualquer outra, com o mesmo
 * atlas de quatro quadros, e nada depois disso precisa saber que ela foi
 * montada de pedacos.
 */
namespace Personagens {

    /// O resultado da composicao, pronto para virar um DrawAnimatedComponent.
    struct Composta {
        /// Passe no lugar do caminho da imagem: a textura ja esta no cache do
        /// Game sob esta chave.
        std::string chaveDaTextura;

        /// O .json de quadros a usar. Todas as pecas tem o mesmo recorte - ha
        /// teste travando isso -, entao serve o de qualquer camada.
        std::string atlas;

        bool ok = false;
    };

    /**
     * @brief Compoe (ou reaproveita) a textura desta aparencia.
     *
     * Reaproveita quando a mesma aparencia ja foi composta: a chave sai de
     * ChaveDaAparencia, que e determinista.
     *
     * Devolve ok = false quando nao deu - aparencia sem camada, PNG ausente,
     * renderizador ausente. Quem chama deve ter um plano B; uma personagem que
     * nao monta nao pode impedir ninguem de jogar.
     */
    Composta Compor(Game* game, const Catalogo& catalogo, const Aparencia& aparencia);
}
