//
// Em quem o professor mira, na dupla: em quem mais o incomodou nos ultimos segundos.
//

#pragma once

#include <array>
#include <cstddef>

/// Os papeis da dupla (requisitos-coop, D3). Cada acerto no professor o incomoda, o incomodo
/// esquece com o tempo, e os ataques mirados vao em quem mais o incomodou: quem ataca chama a mira,
/// e o outro fica livre para cobri-lo (Cobertura.h). Quando os dois incomodam parecido, ele reveza
/// um disparo para cada, como sempre fez. Nao conhece batalha, aluno nem SDL.
namespace Papeis {

    /// O incomodo cai pela metade a cada kMeiaVida segundos.
    constexpr float kMeiaVida = 2.0f;

    /// Para a mira ficar num aluno, o incomodo dele precisa passar kRazao vezes o do outro, mais
    /// kMinimo. Abaixo disso os dois revezam - e quem acerta no mesmo ritmo nao tira a mira do outro
    /// a cada tiro.
    constexpr float kRazao = 1.5f;
    constexpr float kMinimo = 2.0f;

    constexpr std::size_t kAlunos = 2;

    class Incomodo {
    public:
        /// O aluno 'quem' acertou o professor.
        void Acertou(std::size_t quem, float peso = 1.0f);

        /// Os segundos passam, e o incomodo esquece.
        void Avancar(float deltaTime);

        [[nodiscard]] float De(std::size_t quem) const;

        /// Em quem a mira esta presa, com 'quantos' alunos em campo; -1 quando revezam (e sempre com
        /// um aluno so, que e o alvo de qualquer jeito).
        [[nodiscard]] int Foco(std::size_t quantos) const;

        /// Em quem o proximo disparo mira: o foco, ou 'vez' - o revezamento de sempre.
        [[nodiscard]] std::size_t Alvo(std::size_t vez, std::size_t quantos) const;

        /// Esquece tudo (uma batalha nova).
        void Zerar() { mValor = {}; }

    private:
        std::array<float, kAlunos> mValor{};
    };
}
