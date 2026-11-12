//
// O contorno de quem o tiro do aluno destroi.
//

#include "Destrutivel.h"

namespace Destrutivel {

    bool Pode(const bool derrubavel, const bool blindado, const bool abrigo, const bool enfeite) {
        return derrubavel && !blindado && !abrigo && !enfeite;
    }

    std::uint8_t AlfaDaSilhueta(const std::uint8_t a) {
        return a >= kLimiarDaSilhueta ? 255 : 0;
    }
}
