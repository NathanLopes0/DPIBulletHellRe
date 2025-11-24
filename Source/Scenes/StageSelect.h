//
// Created by nslop on 14/10/2025.
//

#ifndef DPIBULLETHELLRE_STAGESELECT_H
#define DPIBULLETHELLRE_STAGESELECT_H

#include "../Scenes/Scene.h"
#include "../Game.h" // Incluir para o enum GameSubject
#include <vector>
#include <memory>

class Font;
class StageSelectButton;
class Actor;

class StageSelect : public Scene {
public:
    explicit StageSelect(Game* game);

    ~StageSelect() override = default;

    // Funções do contrato da Scene
    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

    void OnUpdate(float deltaTime) override;

    // Getter para a matéria selecionada
    [[nodiscard]] Game::GameSubject GetSelectedSubject() const { return mSelectedSubject; }

private:

    // --- Funções Privadas para Organização Lógica ---
    void CreateStageButtons();
    void CreateStaticUI();

    void HandleSelectionInput(const Uint8* keyState);
    size_t mSelectedIndex{};

    // --- Funções Auxiliares de CreateStageButtons ---
    void CreateButton(const std::string &text, Game::GameSubject subject, const Vector2 &position);
    size_t HandleSelectedChange(const Uint8 *keyState, size_t currSelected) const;
    size_t HandleUpInput(size_t currSelected) const;
    size_t HandleDownInput(size_t currSelected) const;
    size_t HandleLeftInput(size_t currSelected) const;
    size_t HandleRightInput(size_t currSelected) const;
    [[nodiscard]] bool IsInBorder(size_t currSelected) const;

    // --- Constantes de Design ---
    static constexpr int NUM_STAGES = 10; // Quantas matérias teremos no jogo
    static constexpr float INPUT_DELAY = 0.2f; // Exemplo de constante para o timer

    // --- Membros de Propriedade (Ownership) ---
    std::unique_ptr<Font> mStageSelectFont{};

    // --- Membros de Estado e Observadores ---
    Game::GameSubject mSelectedSubject{};
    float mInputTimer{}; // Timer pra mudar de botão selecionado

    // Vetor de ponteiros OBSERVADORES para fácil acesso aos botões.
    // A memória real é gerenciada pelo vetor mActors.
    std::vector<StageSelectButton*> mButtonObservers;

    // --- Auxiliares de Navegação ---
    // Retorna em qual coluna visual o índice atual está (0 a 3)
    [[nodiscard]] int GetColumnFromIndex(size_t index) const;

    // Retorna o índice inicial da coluna especificada
    [[nodiscard]] size_t GetColumnStartIndex(int colIndex) const;

    // Retorna quantos itens existem na coluna especificada
    [[nodiscard]] size_t GetColumnSize(int colIndex) const;

};

#endif //DPIBULLETHELLRE_STAGESELECT_H