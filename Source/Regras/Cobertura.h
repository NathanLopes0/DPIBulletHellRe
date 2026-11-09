//
// A cobertura, na dupla: derrubar o projetil que ia no parceiro.
//

#pragma once

#include "Nota.h"
#include "../Motor/Math.h"

/// O que a dupla conquista junta e pago inteiro (requisitos-coop 2.1): o tiro de um aluno que
/// derruba um projetil a caminho do outro protegeu a dupla, e paga a nota sem a divisao. "A caminho"
/// e pelo caminho, e nao pelo alvo que o projetil guarda: um anel inteiro sai "do" aluno da vez, e
/// so o pedaco dele que ia passar pelo parceiro era perigo. Nao conhece batalha, aluno nem SDL.
namespace Cobertura {

    /// Quantos segundos a frente o caminho do projetil e olhado.
    constexpr float kHorizonte = 1.0f;

    /// O ganho bruto de uma cobertura, pago inteiro: o de um acerto no professor, que na dupla vale
    /// o dobro do acerto repartido.
    constexpr float kGanho = Nota::kGanhoPorAcerto;

    /// Se o projetil em 'pos', andando a 'vel' em linha reta, passa a menos de 'raio' de 'alvo' nos
    /// proximos 'horizonte' segundos - ou ja esta perto assim dele.
    bool IaPassarPerto(const Vector2& pos, const Vector2& vel, const Vector2& alvo, float raio,
                       float horizonte = kHorizonte);
}
