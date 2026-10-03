//
// Created by nslop on 23/07/2024.
//

#pragma once

#include "Scene.h"
#include <memory> // Necessário para std::unique_ptr
#include <string>
#include <vector>

// Forward declarations para manter o header limpo
class Font;
class Actor;
class DrawCaixaComponent;

/**
 * @class MainMenu
 * @brief O menu de entrada: por onde o aluno escolhe como comeca.
 *
 * As tres opcoes vivem numa tabela, e nao espalhadas em ifs: acrescentar uma e
 * acrescentar uma linha, e a navegacao nao precisa saber quantas sao. Ver
 * kOpcoes em MainMenu.cpp.
 *
 * Por que NOVO JOGO e CARREGAR PERFIL ainda levam a mesma tela: a separacao
 * entre criar e carregar um perfil e um incremento proprio, descrito em
 * Documentacao/requisitos-perfil-e-personagem.md. Este aqui entrega so o menu.
 */
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

    // A cena é DONA da fonte que ela carrega para o menu, então uso unique_ptr.
    std::unique_ptr<Font> mMainMenuFont;

    // A cena OBSERVA os atores que ela cria (dono = Game), então uso ponteiros brutos
    // para poder interagir com eles se necessário.

    Actor* mBackgroundActor{}; // Ator que segura a imagem de Background
    Actor* mTitleActor{}; // Ator que segura o título

    /// Um ator de texto por opcao, na ordem de kOpcoes.
    std::vector<Actor*> mOpcaoAtores;

    /// A moldura que mostra qual opcao esta em foco. UMA so, que muda de lugar:
    /// uma por opcao obrigaria a apagar e acender todas a cada passo.
    Actor* mSelecaoAtor{};

    int mSelecionada = 0;

    /// @brief Bordas das teclas de navegacao.
    ///
    /// Menu anda de um em um: com leitura por estado, segurar a seta desceria a
    /// lista inteira num piscar. A selecao de fase resolve isto com temporizador;
    /// aqui a borda basta e responde na hora.
    bool mCimaAnterior{};
    bool mBaixoAnterior{};

    /// Comeca em TRUE de proposito: quem volta ao menu pode chegar com ENTER
    /// ainda apertado da tela anterior, e sem isto a opcao em foco seria
    /// escolhida sozinha. O mesmo motivo do mEntrarAnterior da StageSelect.
    bool mConfirmarAnterior = true;
};
