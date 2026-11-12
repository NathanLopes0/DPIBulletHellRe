//
// O ajuste automatico de dificuldade: quem vem reprovando ganha uma prova um pouco mais leve.
//
#pragma once

#include <vector>

#include "Dificuldade.h"

/// O estado de flow (Csikszentmihalyi) e o canal em que o desafio acompanha a habilidade: desafio
/// demais vira ansiedade, e quem reprova seguidas vezes esta fora do canal, por cima. O ajuste traz
/// a prova de volta, devagar e sem avisar: a cada kReprovacoesPorDegrau reprovacoes seguidas numa
/// materia ainda nao aprovada, um degrau, ate kMaiorDegrau; cada degrau tira um pouco da velocidade
/// dos projeteis e da perda por acerto. Aprovar zera a conta. So no Normal: no Dificil o aluno
/// escolheu o desafio. Puro: quem conta as reprovacoes e o Progresso, e quem aplica e a Battle.
namespace Ajuste {

    constexpr int kReprovacoesPorDegrau = 2;
    constexpr int kMaiorDegrau = 2;

    /// Quanto cada degrau tira da velocidade dos projeteis do professor, e da perda por acerto.
    constexpr float kVelocidadePorDegrau = 0.06f;
    constexpr float kPerdaPorDegrau = 0.15f;

    /// O que a prova muda num degrau.
    struct Degrau {
        int nivel = 0;
        float velocidade = 1.0f;   ///< multiplica a velocidade de cada disparo do professor
        float perda = 1.0f;        ///< multiplica o que um acerto tira da nota
    };

    /// O degrau de quem reprovou 'seguidas' vezes seguidas nesta materia. Zero para quem ja passou nela
    /// e no Dificil.
    int NivelPara(int seguidas, bool aprovado, Dificuldade::Nivel nivel);

    /// Na dupla, o menor dos degraus: so alivia quando os dois vem reprovando. Vazio, zero.
    int DaDupla(const std::vector<int>& niveis);

    /// O que o degrau muda (fora de 0 a kMaiorDegrau, o mais perto deles).
    Degrau Do(int nivel);
}
