//
// Monta as camadas da personagem numa textura unica.
//

#pragma once

#include <string>

#include "Personagens.h"

class Game;

/// A ponte entre a regra de composicao - que vive em Source/Personagens, e puro, e testado - e os
/// pixels.
namespace Personagens {

    /// O resultado da composicao, pronto para virar um DrawAnimatedComponent.
    struct Composta {
        std::string chaveDaTextura;

        /// O .json de quadros a usar. Todas as pecas tem o mesmo recorte - ha
        /// teste travando isso -, entao serve o de qualquer camada.
        std::string atlas;

        bool ok = false;
    };

    /// Compoe (ou reaproveita) a textura desta aparencia - com o capelo, se o aluno ja se formou.
    Composta Compor(Game* game, const Catalogo& catalogo, const Aparencia& aparencia, bool formado = false);
}
