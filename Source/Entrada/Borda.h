//
// A batida de um botao: o quadro em que ele passa de solto a apertado.
//

#pragma once

/// Lembra se o botao estava apertado no quadro anterior, para cada aperto contar uma vez so.
class Borda {
public:
    /// 'apertadaNoComeco': o botao que ja vinha segurado da tela anterior nao conta como batida.
    constexpr Borda(const bool apertadaNoComeco = false) : mAntes(apertadaNoComeco) {}

    /// True so no quadro em que 'agora' passa de solto a apertado.
    bool Bateu(const bool agora) {
        const bool bateu = agora && !mAntes;
        mAntes = agora;
        return bateu;
    }

    /// O que ja esta apertado agora nao conta como batida nova.
    void Segurar() { mAntes = true; }

private:
    bool mAntes;
};
