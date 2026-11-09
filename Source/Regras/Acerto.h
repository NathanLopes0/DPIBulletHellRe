//
// Quem acertou o aluno ha pouco: o mesmo causador nao acerta duas vezes seguidas.
//

#pragma once

#include <vector>

#include "Dificuldade.h"
#include "../Motor/Math.h"

/// No Dificil a invencibilidade e curta (Dificuldade::Invencibilidade): o tiro que vem atras do
/// primeiro tambem acerta.
namespace Acerto {

    constexpr float kMesmoCausador = Dificuldade::kInvencibilidadeNormal;

    /// Se um tiro que andou de 'de' ate 'ate' neste quadro passou a menos de 'raio' de 'centro' -
    /// o trecho inteiro, e nao so onde ele parou: um tiro rapido nao atravessa um alvo entre um
    /// quadro e o outro, nem quando o jogo engasga.
    bool NoCaminho(const Vector2& de, const Vector2& ate, const Vector2& centro, float raio);

    class Memoria {
    public:
        /// Os segundos passam; o que ficou velho e esquecido.
        void Avancar(float deltaTime);

        /// 'quem', com 'idade' segundos de vida, acabou de acertar.
        void Lembrar(const void* quem, float idade);

        [[nodiscard]] bool Lembra(const void* quem, float idade) const;

        [[nodiscard]] size_t Quantos() const { return mItens.size(); }

    private:
        struct Item {
            const void* quem = nullptr;
            float idade = 0.0f;   ///< a idade dele no acerto
            float ha = 0.0f;      ///< ha quanto tempo foi
        };
        std::vector<Item> mItens;
    };
}
