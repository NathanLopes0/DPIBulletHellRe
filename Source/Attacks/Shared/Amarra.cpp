//
// A letra amarrada: ligare, "atar".
//

#include "Amarra.h"

#include <cmath>

namespace Amarra {

bool SentidoPeloNome(const std::string& nome, Sentido& sentido) {
    if (nome == "horario")     { sentido = Sentido::Horario;     return true; }
    if (nome == "antihorario") { sentido = Sentido::AntiHorario; return true; }
    return false;
}

Lance Passo(Corda& corda, const Vector2 posicao, const Vector2 velocidade,
            const Vector2 no, const float dt) {

    Lance lance{posicao, velocidade, Evento::Nenhum};
    if (corda.cortada) return lance;

    const float modulo = velocidade.Length();
    if (modulo <= 0.0f) return lance;

    if (corda.solta < 0.0f) {
        // A CORDA NASCE NO NO, com a direcao do voo. Nem a distancia nem a
        // direcao da posicao servem: no primeiro quadro o professor ja andou um
        // pouco, e cada letra da roda estaria a uma distancia diferente dele -
        // as letras esticariam em quadros diferentes e a roda sairia torta.
        corda.angulo = std::atan2(velocidade.y, velocidade.x);
        corda.solta = 0.0f;
    }

    // O giro no sentido HORARIO DA TELA aumenta o angulo: com o y para baixo,
    // ir de +x para +y e girar no sentido do relogio.
    const float sinal = (corda.sentido == Sentido::Horario) ? 1.0f : -1.0f;

    if (corda.amarrada < 0.0f) {
        corda.solta += modulo * dt;
        if (corda.solta >= corda.comprimento) {
            corda.solta = corda.comprimento;
            corda.amarrada = 0.0f;
            lance.evento = Evento::Amarrou;
        }
    } else {
        corda.amarrada += dt;
        corda.angulo += sinal * modulo / corda.comprimento * dt;
    }

    const Vector2 raio(std::cos(corda.angulo), std::sin(corda.angulo));
    lance.posicao = no + raio * corda.solta;
    lance.velocidade = (corda.amarrada < 0.0f)
                           ? raio * modulo
                           : Vector2(-raio.y, raio.x) * (sinal * modulo);

    if (corda.amarrada >= corda.duracao) {
        corda.cortada = true;
        lance.evento = Evento::Cortou;
    }
    return lance;
}

}
