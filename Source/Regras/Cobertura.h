//
// A cobertura, na dupla: derrubar o projetil que ia no parceiro.
//

#pragma once

#include <vector>

#include "Nota.h"
#include "Raspao.h"
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

    /// Um aluno, para as contas abaixo: o centro e o raio do colisor dele. Quem nada atinge (o
    /// escudo, a piscada) nao precisa de cobertura, e nao e 'atingivel'.
    struct Aluno {
        Vector2 centro;
        float raio = 0.0f;
        bool atingivel = false;
    };

    /// O projetil de raio 'raioDoProjetil' vai neste aluno: passaria na faixa do raspao dele
    /// (Raspao::kAlcance), ou mais perto. E a conta da cobertura.
    bool VaiNele(const Vector2& pos, const Vector2& vel, float raioDoProjetil, const Aluno& aluno,
                 float horizonte = kHorizonte);

    /// Em qual dos alunos o projetil vai (VaiNele): o indice do mais perto dele, entre os que ele
    /// pegaria, ou -1. Sozinho tambem e -1: sem parceiro, nao ha quem cubra. E o que diz a cor do
    /// brilho do projetil que vai num aluno - derruba-lo, para o outro, e a cobertura.
    int AlunoNoCaminho(const Vector2& pos, const Vector2& vel, float raioDoProjetil,
                       const std::vector<Aluno>& alunos, float horizonte = kHorizonte);
}
