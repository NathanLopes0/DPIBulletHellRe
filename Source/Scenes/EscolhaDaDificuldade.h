//
// A escolha da dificuldade, antes da selecao de fases.
//

#pragma once

#include <array>
#include <memory>
#include <string>

#include "Scene.h"
#include "../Regras/Dificuldade.h"

class Font;
class Actor;
class DrawCaixaComponent;

/// Normal ou Dificil: quem vai jogar escolhe, depois de se identificar e antes da selecao de fases.
/// A dificuldade e de quem joga.
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

    /// As bordas, para uma acao por toque. Comecam em true: o botao 1 que confirmou a matricula nao
    /// confirma a dificuldade junto.
    bool mConfirmarAnterior = true;
    bool mVoltarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
};
