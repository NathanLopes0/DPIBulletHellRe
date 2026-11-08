//
// Created by nslop on 14/10/2025.
//

#pragma once

#include "Scene.h"
#include "../Entrada/Borda.h"
#include "../Motor/Game.h" // Incluir para o enum int
#include <vector>
#include "../Entrada/Navegacao.h"
#include <SDL_pixels.h>
#include <memory>

class Font;
class StageSelectButton;
class Actor;

class StageSelect : public Scene {
public:
    explicit StageSelect(Game* game);

    ~StageSelect() override = default;

    // Funções do contrato da Scene.
    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

    void OnUpdate(float deltaTime) override;

private:

    // ---- Funções Privadas para Organização Lógica ----
    void CreateStageButtons();
    void CreateStaticUI();

    /// A ordem de desenho e crescente: o menor fica por baixo.
    static constexpr int kOrdemDasLigacoes = 60;

    /// Do centro do losango ate a ponta. Ele tem 128 de largura.
    static constexpr float kMetadeDoLosango = 64.0f;

    static constexpr float kGrossuraDaLigacao = 2.0f;

    /// As linhas entre colunas: azul frio e discreto. Elas sao cenario, e nao
    /// podem competir com os losangos nem com o texto.
    static inline const SDL_Color kCorDaLigacao{72, 92, 140, 200};

    /// O tronco entre duas colunas, com um ramo para cada materia dos dois lados. Chamada depois de
    /// CreateStageButtons: le a posicao dos botoes.
    void CriarLigacoes();

    /// Um segmento de linha, como caixa preenchida sem moldura.
    void Linha(float x, float y, float largura, float altura);

    void HandleSelectionInput(const Uint8* keyState);
    size_t mSelectedIndex{};

    // ---- Funções Auxiliares de CreateStageButtons ----
    void CreateButton(const std::string &text, int subject, const Vector2 &position);
    /// Para onde a seta leva. A regra esta em Navegacao; aqui so entra a tecla.
    [[nodiscard]] size_t HandleSelectedChange(const Uint8 *keyState, size_t currSelected) const;

    // ---- Constantes de Design ----
    static constexpr int RESERVA_DE_BOTOES = 16;
    static constexpr float INPUT_DELAY = 0.2f; // Exemplo de constante para o timer

    /// Onde a grade de materias comeca e termina, em fracao da altura.
    static constexpr float kTopoDaGrade = 0.19f;
    static constexpr float kFundoDaGrade = 0.78f;

    /// Quanto o nome de uma materia fechada fica abaixo do centro do losango.
    /// O losango tem 64 de altura, entao 44 deixa o nome logo embaixo dele.
    static constexpr float kNomeAbaixoDoCadeado = 44.0f;

    // ---- Membros de Propriedade (Ownership) ----
    std::unique_ptr<Font> mStageSelectFont{};

    // ---- Membros de Estado e Observadores ----
    int mSelectedSubject{};
    float mInputTimer{}; // Timer pra mudar de botão selecionado

    Borda mBordaRanking{true};

    // Vetor de ponteiros observadores para fácil acesso aos botões. A memória real é gerenciada
    // pelo vetor mActors.
    std::vector<StageSelectButton*> mButtonObservers;

    /// A forma da grade, montada em CreateStageButtons junto com os botoes. mGrade[coluna][linha] =
    /// indice em mButtonObservers.
    Navegacao::Grade mGrade;

    // ---- O cartao da materia em foco ----

    static constexpr float kCartaoX = 268.0f;
    static constexpr float kCartaoY = 600.0f;
    static constexpr float kCartaoLargura = 400.0f;
    static constexpr float kCartaoAltura = 184.0f;
    static constexpr int kOrdemDoCartao = 70;

    static inline const SDL_Color kCorDaMolduraDoCartao{96, 118, 170, 220};
    static inline const SDL_Color kCorDoFundoDoCartao{10, 14, 30, 215};

    void UpdateStageInfo() const;

    /// Cria uma das linhas do cartao e devolve o observador dela.
    Actor* TextoDoCartao(float y, int tamanho, const Vector3& cor);

    /// Troca texto e cor de uma linha do cartao.
    static void EscreverNoCartao(Actor* ator, const std::string& texto, const Vector3& cor);

    Actor* mCodigoAtor = nullptr;
    Actor* mNomeAtor = nullptr;
    Actor* mProfessorAtor = nullptr;
    Actor* mEstadoAtor = nullptr;

    /// Quanto uma linha de aluno fica abaixo da anterior, quando ha dois. O
    /// rotulo tem 44 de altura, entao 38 os encosta sem os sobrepor.
    static constexpr float kEntreOsAlunos = 38.0f;

    /// Quem esta jogando, e o aviso de que visitante nao salva. Uma linha por
    /// posicao do painel.
    void CriarIdentificacaoNaTela();
    Actor* mAlunoAtor = nullptr;
    Actor* mTrocarAtor = nullptr;

    /// Com a borda em false, voltar do ranking com o botao ainda apertado jogava o aluno direto
    /// para a tela de matricula.
    Borda mBordaTrocar{true};

    /// Borda do enter, e comeca em true. A cena anterior (identificacao) termina com o enter
    /// apertado - e ele que manda para ca.
    Borda mBordaEntrar{true};
};