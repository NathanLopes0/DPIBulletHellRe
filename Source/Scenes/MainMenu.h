//
// Created by nslop on 23/07/2024.
//

#pragma once

#include "Scene.h"
#include "../Entrada/Borda.h"
#include "../Entrada/CodigoSecreto.h"
#include <memory> // Necessário para std::unique_ptr
#include <string>
#include <vector>

// Forward declarations para manter o header limpo.
class Font;
class Actor;
class DrawCaixaComponent;
class DrawTextComponent;

/// O menu de entrada: por onde o aluno escolhe como comeca.
class MainMenu : public Scene {
public:
    explicit MainMenu(Game* game);

    void LoadBackground();

    void LoadTitle();

    ~MainMenu() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    void CriarOpcoes();

    /// Poe a moldura em volta da opcao em foco.
    void PosicionarSelecao() const;

    // A cena é dona da fonte que ela carrega para o menu, então uso unique_ptr.
    std::unique_ptr<Font> mMainMenuFont;

    // A cena observa os atores que ela cria (dono = Game), então uso ponteiros brutos para poder
    // interagir com eles se necessário.

    Actor* mBackgroundActor{}; // Ator que segura a imagem de Background
    Actor* mTitleActor{}; // Ator que segura o título

    /// Um ator de texto por opcao, na ordem de kOpcoes.
    std::vector<Actor*> mOpcaoAtores;

    /// A moldura que mostra qual opcao esta em foco. Uma so, que muda de lugar: uma por opcao
    /// obrigaria a apagar e acender todas a cada passo.
    Actor* mSelecaoAtor{};
    /// "BOTAO 1 confirmar", ao lado da moldura, andando com ela: quem chega ao gabinete nao sabe o que
    /// apertar.
    Actor* mDicaAtor{};
    /// O Modo Dupla em foco ganha a moldura e a letra douradas, num pulso lento: o convite para jogar
    /// junto, sem gritar.
    float mPulso = 0.0f;
    void PintarSelecao();

    int mSelecionada = 0;

    /// Bordas das teclas de navegacao. Menu anda de um em um: com leitura por estado, segurar a
    /// seta desceria a lista inteira num piscar.
    Borda mBordaCima;
    Borda mBordaBaixo;

    /// Comeca em true: quem volta ao menu pode chegar com enter ainda apertado da tela anterior, e
    /// sem isto a opcao em foco seria escolhida sozinha.
    Borda mBordaConfirmar{true};

    // ---- O codigo secreto (Entrada/CodigoSecreto.h) ----

    /// Cada toque do jogador 1 entra na sequencia; o que fecha o codigo nao escolhe a opcao.
    /// Devolve se o codigo acabou de fechar.
    bool TocarNoCodigo(const Uint8* keyState, bool cima, bool baixo);
    void MostrarOSegredo();
    CodigoSecreto::Sequencia mCodigo;
    /// Bordas proprias das setas: o codigo nao depende de o menu ter andado.
    Borda mBordaCimaDoCodigo;
    Borda mBordaBaixoDoCodigo;
    Borda mBordaEsquerda;
    Borda mBordaDireita;
    Borda mBordaB{true};
    Borda mBordaA{true};
    /// O aviso dourado de que o codigo entrou, e quanto tempo ele ainda fica (0: sumiu).
    DrawTextComponent* mAvisoDoSegredo{};
    DrawCaixaComponent* mFaixaDoSegredo{};
    float mTempoDoAviso = 0.0f;
};
