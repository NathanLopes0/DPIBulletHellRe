//
// Quem acertou o aluno ha pouco: o mesmo causador nao acerta duas vezes seguidas.
//

#include "Acerto.h"

#include <algorithm>

namespace Acerto {

    namespace {
        /// A folga da idade: o relogio do projetil e o do quadro nao batem ao
        /// centesimo, e um tiro nao fica mais novo de um quadro para o outro.
        constexpr float kFolgaDaIdade = 0.05f;
    }

    bool NoCaminho(const Vector2& de, const Vector2& ate, const Vector2& centro, const float raio) {
        const Vector2 trecho = ate - de;
        const float comprimento = trecho.LengthSq();
        // O ponto do trecho mais perto do centro.
        const float u = comprimento > 0.0f ? std::clamp(Vector2::Dot(centro - de, trecho) / comprimento, 0.0f, 1.0f) : 0.0f;
        const Vector2 maisPerto = de + trecho * u;
        return (centro - maisPerto).LengthSq() <= raio * raio;
    }

    void Memoria::Avancar(const float deltaTime) {
        for (Item& i : mItens) i.ha += deltaTime;
        mItens.erase(std::remove_if(mItens.begin(), mItens.end(),
                                    [this](const Item& i) { return i.ha >= mDuracao; }),
                     mItens.end());
    }

    void Memoria::Lembrar(const void* quem, const float idade) {
        if (!quem) return;
        for (Item& i : mItens) {
            if (i.quem == quem) {
                i = Item{quem, idade, 0.0f};
                return;
            }
        }
        mItens.push_back(Item{quem, idade, 0.0f});
    }

    bool Memoria::Lembra(const void* quem, const float idade) const {
        if (!quem) return false;
        for (const Item& i : mItens) {
            if (i.quem == quem && idade + kFolgaDaIdade >= i.idade && i.ha < mDuracao) return true;
        }
        return false;
    }
}
