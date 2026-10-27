//
// A tela de opcoes.
//

#pragma once

#include <memory>
#include <string>

#include "Scene.h"

class Font;
class Actor;
class DrawTextComponent;

/**
 * @class Opcoes
 * @brief Onde as configuracoes da maquina moram (Configuracao.h).
 *
 * CINCO LINHAS: a dificuldade (Dificuldade.h), o volume da musica e o dos
 * efeitos, a tela cheia e a hitbox do aluno visivel. O manche para cima e para
 * baixo escolhe a linha; para os lados, ou o botao 1, troca o valor - e a
 * troca vale e vai para o disco na hora, sem um "salvar" que alguem esqueca de
 * apertar. O botao 2 (ou o ESC) volta ao menu.
 *
 * O botao 1 num volume SOBE e da a volta no maximo, para quem so tem o botao
 * conseguir chegar a qualquer valor; o manche para por la, nos dois extremos.
 */
class Opcoes : public Scene {
public:
    explicit Opcoes(Game* game);
    ~Opcoes() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    enum class Item { Dificuldade, Musica, Efeitos, TelaCheia, Hitbox };
    static constexpr int kItens = 5;

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

    /// As bordas, para uma acao por toque. Comecam em TRUE pela mesma regra
    /// das outras cenas: quem entra aqui segurando o botao 1 do menu nao
    /// troca a dificuldade sem querer.
    bool mVoltarAnterior = true;
    bool mTrocarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
    bool mCimaAnterior = true;
    bool mBaixoAnterior = true;
};
