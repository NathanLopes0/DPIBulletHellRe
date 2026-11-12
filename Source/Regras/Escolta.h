//
// O orientador em campo: onde ele fica.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"

/// O orientador do TCC voa junto do aluno, e nao no lugar dele: e invulneravel, nao tem colisor, e
/// o que ele faz e atirar.
namespace Escolta {

    /// Sozinho: quanto ao lado e quanto acima do aluno ele fica.
    constexpr float kAoLado = 74.0f;
    constexpr float kAcima = 58.0f;

    /// Em dupla: quanto acima do meio dos dois.
    constexpr float kAcimaDaDupla = 96.0f;

    /// O espaco que ele guarda das bordas do campo.
    constexpr float kMargem = 36.0f;

    /// Quao depressa ele alcanca o posto: a fracao que falta cai a e^(-k t).
    constexpr float kRapidez = 6.0f;

    int Lado(float xDoAluno, int ladoAtual, float esquerda, float direita);

    /// Onde ele quer estar. 'alunos' sao as posicoes de quem joga; sem ninguem, devolve o meio do
    /// campo, no alto.
    Vector2 Posto(const std::vector<Vector2>& alunos, int lado, float esquerda, float direita);

    /// Um passo de 'dt' segundos de 'atual' na direcao de 'alvo'.
    Vector2 Seguir(const Vector2& atual, const Vector2& alvo, float dt);
}
