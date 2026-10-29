//
// O orientador em campo: onde ele fica.
//

#pragma once

#include <vector>

#include "../../Core/Math.h"

/**
 * CAMADA PURA.
 *
 * O orientador do TCC voa junto do aluno, e nao no lugar dele: e invulneravel,
 * nao tem colisor, e o que ele faz e atirar. Aqui ficam as contas que o
 * ator (Orientador) faz todo quadro, para poderem ser testadas sem cena.
 *
 * ONDE ELE FICA. Sozinho, ao lado do aluno e um pouco acima - de um lado so, e
 * so troca quando a parede tira o espaco: trocar de lado a cada passo faria o
 * orientador atravessar o aluno o tempo todo. Em dupla, no meio dos dois e mais
 * acima, porque ele e dos dois.
 *
 * COMO ELE CHEGA LA. Seguindo com atraso (Seguir): andar colado no aluno faria
 * o orientador tremer junto com cada correcao do manche, e chegar devagar e o
 * que o faz parecer alguem, e nao um adesivo.
 *
 * PARA ONDE ATIRA e o golpe da materia dele, em Golpe.h.
 */
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

    /// @brief De que lado do aluno ele fica (-1 esquerda, +1 direita), dado o
    /// lado de agora. So troca quando o lado de agora nao cabe mais no campo.
    int Lado(float xDoAluno, int ladoAtual, float esquerda, float direita);

    /// @brief Onde ele quer estar. 'alunos' sao as posicoes de quem joga; sem
    /// ninguem, devolve o meio do campo, no alto. O x fica dentro do campo.
    Vector2 Posto(const std::vector<Vector2>& alunos, int lado, float esquerda, float direita);

    /// @brief Um passo de 'dt' segundos de 'atual' na direcao de 'alvo'.
    Vector2 Seguir(const Vector2& atual, const Vector2& alvo, float dt);
}
