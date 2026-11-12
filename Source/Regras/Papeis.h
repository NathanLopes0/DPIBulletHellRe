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

    /// A mira inteira do professor: o incomodo de cada aluno, a vez do revezamento e a vez dada a um
    /// disparo que e de alguem. Com os papeis desligados (Experimento.h), so o revezamento de antes
    /// deles. 'quantos' e sempre quantos alunos estao em campo.
    class Mira {
    public:
        explicit Mira(bool papeis = true) : mPapeis(papeis) {}

        /// Em qual posicao esta a vez do proximo disparo: a vez dada, se houver; senao a de quem mais
        /// incomodou ou, se os dois incomodam parecido, a do revezamento.
        [[nodiscard]] std::size_t Alvo(std::size_t quantos) const;

        /// Em quem a mira esta presa (o "v" vermelho sobre o aluno); -1 quando os dois revezam.
        [[nodiscard]] int Foco(std::size_t quantos) const;

        /// O proximo disparo e de 'posicao', mesmo que a mira esteja no outro. Fora da lista, nada.
        void DarAVez(std::size_t posicao, std::size_t quantos);

        /// Um disparo saiu: a vez dada acabou, e o revezamento anda.
        void Revezar(std::size_t quantos);

        /// O aluno 'quem' acertou o professor, e o incomoda.
        void Acertou(const std::size_t quem) { mIncomodo.Acertou(quem); }
        void Avancar(const float deltaTime) { mIncomodo.Avancar(deltaTime); }

    private:
        Incomodo mIncomodo;
        std::size_t mVez = 0;
        bool mVezDada = false;
        bool mPapeis = true;
    };
}
