//
// Created by nslop on 23/07/2024.
//

#pragma once

#include "Scene.h"
#include "../Entrada/Borda.h"
#include <memory> // Necessário para std::unique_ptr
#include <string>
#include <vector>

// Forward declarations para manter o header limpo.
class Font;
class Actor;
class DrawCaixaComponent;

/// O menu de entrada: por onde o aluno escolhe como comeca.
class MainMenu : public Scene {
public:
    explicit MainMenu(Game* game);

    void LoadBackground();

    void LoadTitle();

    ~MainMenu() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

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

    int mSelecionada = 0;

    /// Bordas das teclas de navegacao. Menu anda de um em um: com leitura por estado, segurar a
    /// seta desceria a lista inteira num piscar.
    Borda mBordaCima;
    Borda mBordaBaixo;

    /// Comeca em true: quem volta ao menu pode chegar com enter ainda apertado da tela anterior, e
    /// sem isto a opcao em foco seria escolhida sozinha.
    Borda mBordaConfirmar{true};
};
