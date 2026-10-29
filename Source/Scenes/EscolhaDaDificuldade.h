//
// A escolha da dificuldade, antes da selecao de fases.
//

#pragma once

#include <array>
#include <memory>
#include <string>

#include "Scene.h"
#include "../Gameplay/Dificuldade.h"

class Font;
class Actor;
class DrawCaixaComponent;

/**
 * @class EscolhaDaDificuldade
 * @brief Normal ou Dificil: quem vai jogar escolhe, depois de se identificar e
 * antes da selecao de fases.
 *
 * A DIFICULDADE E DE QUEM JOGA. Ela morava nas Opcoes, como opcao da maquina,
 * e quem chegava no gabinete herdava o Dificil de quem tinha jogado antes - sem
 * saber, porque o unico aviso era um selo na batalha. Aqui ela e escolhida a
 * cada partida, e o cursor comeca no Normal (Game::DefinirModoDeJogo).
 *
 * DOIS CARTOES LADO A LADO, um por nivel, com o que ele muda. O manche de
 * qualquer um dos dois alunos troca - a escolha e da dupla, como a pausa -, o
 * botao 1 confirma e leva as fases, e o botao 2 volta para a identificacao
 * (trocar de aluno). A selecao de fases volta para ca.
 */
class EscolhaDaDificuldade : public Scene {
public:
    explicit EscolhaDaDificuldade(Game* game);
    ~EscolhaDaDificuldade() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:
    /// Um texto centrado em (x, y). 'linhas': quantas linhas ele pode ocupar
    /// quando quebra - a caixa do texto e um limite, e o que nao cabe encolhe.
    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima,
                 int linhas = 1);

    /// Destaca o cartao escolhido.
    void Mostrar();

    std::unique_ptr<Font> mFonte;
    std::array<DrawCaixaComponent*, Dificuldade::kQuantos> mCartoes{};
    Dificuldade::Nivel mEscolhido = Dificuldade::Nivel::Normal;

    /// As bordas, para uma acao por toque. Comecam em TRUE: o botao 1 que
    /// confirmou a matricula nao confirma a dificuldade junto.
    bool mConfirmarAnterior = true;
    bool mVoltarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
};
