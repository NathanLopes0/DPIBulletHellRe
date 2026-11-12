//
// O raspao: o projetil que passou rente ao aluno sem acerta-lo.
//

#include "Raspao.h"

namespace Raspao {

    bool NaFaixa(const Vector2& aluno, const float raioDoAluno, const Vector2& projetil, const float raio) {
        const float d2 = (projetil - aluno).LengthSq();
        const float encosta = raio + raioDoAluno;
        const float alcance = raio + kAlcance;
        return d2 > encosta * encosta && d2 <= alcance * alcance;
    }

    Vector2 OndeFaisca(const Vector2& aluno, const Vector2& projetil) {
        const Vector2 d = projetil - aluno;
        const float n = d.Length();
        if (n <= 0.0f) return aluno;
        return aluno + d * (kAlcance / n);
    }
}
