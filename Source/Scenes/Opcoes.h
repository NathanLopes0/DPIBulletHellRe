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
 * Hoje ha uma opcao so, a dificuldade (Dificuldade.h): o manche para os lados
 * ou o botao 1 trocam, e a troca vai para o disco na hora - sem um "salvar" que
 * alguem esqueca de apertar. O botao 2 (ou o ESC) volta ao menu.
 *
 * As outras ideias para esta tela estao no RETOMAR: volume, tela cheia, a
 * hitbox do aluno visivel, menos flashes, uma paleta para daltonicos.
 */
class Opcoes : public Scene {
public:
    explicit Opcoes(Game* game);
    ~Opcoes() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    /// Cria um ator de texto no x pedido, no tamanho natural dele.
    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima);

    /// Escreve na tela o nivel em vigor e o que ele muda.
    void MostrarDificuldade();

    std::unique_ptr<Font> mFonte;

    DrawTextComponent* mValor = nullptr;
    DrawTextComponent* mDescricao = nullptr;

    /// As bordas, para uma acao por toque. Comecam em TRUE pela mesma regra
    /// das outras cenas: quem entra aqui segurando o botao 1 do menu nao
    /// troca a dificuldade sem querer.
    bool mVoltarAnterior = true;
    bool mTrocarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
};
