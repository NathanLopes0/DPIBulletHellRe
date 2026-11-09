//
// O raspao: o projetil que passou rente ao aluno sem acerta-lo.
//

#pragma once

#include "Nota.h"
#include "../Motor/Math.h"

/// Desviar por pouco e um risco, e paga um pouco de nota - uma vez por projetil. Nao conhece
/// batalha, aluno nem SDL: so a geometria e o quanto vale.
namespace Raspao {

    /// A faixa do raspao vai da borda do colisor do aluno ate kAlcance do centro dele, contados da
    /// borda do projetil. O colisor e pequeno (um decimo do desenho: 6,4 px de 64), e a faixa vai
    /// quase ate a borda do desenho, que e onde o olho ve o tiro passar rente.
    inline constexpr float kAlcance = 24.0f;

    /// Abaixo desta rapidez (px/s) o projetil esta parado: passar rente a ele nao e desviar.
    inline constexpr float kRapidezMinima = 40.0f;

    /// O ganho bruto de um raspao: um terco de um acerto no professor. E de um jogador so, e se
    /// reparte na dupla como os acertos (Nota::PorJogador).
    inline constexpr float kGanho = Nota::kGanhoPorAcerto / 3.0f;

    /// Por quanto tempo o aluno lembra de um projetil que o raspou: a vida inteira de um projetil
    /// (Projectile::kTempoDeVidaMaximo). Um projetil raspa uma vez so, mesmo que volte.
    inline constexpr float kMemoria = 20.0f;

    /// Se o projetil de raio 'raio' em 'projetil' esta na faixa do raspao do aluno em 'aluno', de
    /// colisor 'raioDoAluno': perto o bastante, e sem encostar no colisor.
    bool NaFaixa(const Vector2& aluno, float raioDoAluno, const Vector2& projetil, float raio);

    /// Onde a faisca do raspao nasce: na borda do desenho do aluno, do lado do projetil.
    Vector2 OndeFaisca(const Vector2& aluno, const Vector2& projetil);
}
