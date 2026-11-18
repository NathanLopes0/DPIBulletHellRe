//
// A tela de opcoes.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Entrada/Borda.h"

class Font;
class Actor;
class DrawTextComponent;

/// Onde as configuracoes da maquina moram (Configuracao.h). Quatro linhas: o volume da musica e o
/// dos efeitos, a tela cheia e a hitbox do aluno visivel.
class Opcoes : public Scene {
public:
    explicit Opcoes(Game* game);
    ~Opcoes() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:

    enum class Item { Musica, Efeitos, TelaCheia, Hitbox, Daltonismo };
    static constexpr int kItens = 5;

    /// As linhas desta vez, na ordem da tela. No gabinete a tela cheia nao entra: la o jogo esta
    /// sempre em tela cheia, e a linha nao mudaria nada.
    std::vector<Item> mItens;
    [[nodiscard]] int Quantos() const { return static_cast<int>(mItens.size()); }

    /// Cria um ator de texto no x pedido, no tamanho natural dele.
    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima);

    /// Anda o valor da linha escolhida 'passo' casas. 'peloBotao': o botao 1,
    /// que num volume da a volta.
    void Trocar(int passo, bool peloBotao);

    /// Reescreve o valor de cada linha, destaca a escolhida e escreve o que ela faz.
    void Mostrar();

    std::unique_ptr<Font> mFonte;

    struct Linha {
        class DrawCaixaComponent* caixa = nullptr;
        DrawTextComponent* valor = nullptr;
    };
    Linha mLinhas[kItens];
    DrawTextComponent* mDescricao = nullptr;
    int mEscolhida = 0;

    /// As bordas, para uma acao por toque.
    Borda mBordaVoltar{true};
    Borda mBordaTrocar{true};
    Borda mBordaEsquerda{true};
    Borda mBordaDireita{true};
    Borda mBordaCima{true};
    Borda mBordaBaixo{true};
};
