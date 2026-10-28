//
// A barra da nota: o que ela mostra entre um quadro e outro.
//

#include "BarraDaNota.h"

#include <algorithm>
#include <cmath>

namespace BarraDaNota {

    Estado Comecar(const float nota) {
        Estado e;
        e.mostrada = e.rastro = e.nota = nota;
        return e;
    }

    void Avancar(Estado& e, const float nota, const float dt) {
        // SUBIU: o clarao, e o rastro vai junto (nao ha o que mostrar perdido).
        if (nota > e.nota + 1e-4f) e.brilho = 1.0f;
        // CAIU: o rastro fica onde a nota estava, e a barra sacode.
        if (nota < e.nota - 1e-4f) {
            e.rastro = std::max(e.rastro, e.nota);
            e.rastroParado = kRastroParado;
            if (e.nota - nota >= kQuedaQueSacode) e.tremor = 1.0f;
        }
        e.nota = nota;

        // O PREENCHIMENTO CORRE ATRAS da nota: rapido, mas nao de uma vez.
        const float passo = std::min(1.0f, kPerseguicao * dt);
        e.mostrada += (nota - e.mostrada) * passo;
        if (std::fabs(nota - e.mostrada) < 0.01f) e.mostrada = nota;

        // O RASTRO: parado um instante, depois encolhe ate o preenchimento.
        if (e.rastro <= e.mostrada) {
            e.rastro = e.mostrada;
        } else if (e.rastroParado > 0.0f) {
            e.rastroParado = std::max(0.0f, e.rastroParado - dt);
        } else {
            e.rastro = std::max(e.mostrada, e.rastro - kRastroEncolhe * dt);
        }

        e.brilho = std::max(0.0f, e.brilho - kApagaBrilho * dt);
        e.tremor = std::max(0.0f, e.tremor - kApagaTremor * dt);
    }

    float X(const float nota, const int largura) {
        return std::clamp(nota, 0.0f, 100.0f) / 100.0f * static_cast<float>(largura);
    }
}
