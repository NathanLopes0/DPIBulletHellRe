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

    void Memoria::Avancar(const float deltaTime) {
        for (Item& i : mItens) i.ha += deltaTime;
        mItens.erase(std::remove_if(mItens.begin(), mItens.end(),
                                    [](const Item& i) { return i.ha >= kMesmoCausador; }),
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
            // O MESMO ENDERECO E A MESMA VIDA: a idade de agora nao e menor que
            // a do acerto (um tiro mais novo e outro tiro, que o pool reciclou).
            if (i.quem == quem && idade + kFolgaDaIdade >= i.idade && i.ha < kMesmoCausador) return true;
        }
        return false;
    }
}
