//
// Created by nslop on 14/10/2025.
//

#pragma once

#include "../Scenes/Scene.h"
#include "../Game.h" // Incluir para o enum int
#include <vector>
#include "../Navegacao.h"
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
    [[nodiscard]] int GetSelectedSubject() const { return mSelectedSubject; }

private:

    // --- Funções Privadas para Organização Lógica ---
    void CreateStageButtons();
    void CreateStaticUI();

    void HandleSelectionInput(const Uint8* keyState);
    size_t mSelectedIndex{};

    // --- Funções Auxiliares de CreateStageButtons ---
    void CreateButton(const std::string &text, int subject, const Vector2 &position);
    /// Para onde a seta leva. A regra esta em Navegacao; aqui so entra a tecla.
    /// Deixou de ser static ao passar a consultar a grade real (mGrade).
    [[nodiscard]] size_t HandleSelectedChange(const Uint8 *keyState, size_t currSelected) const;

    // --- Constantes de Design ---
    /// So uma reserva do vetor, para evitar realocacao: o numero de materias de
    /// verdade vem de materias.json. NAO e mais usado para navegar - era isso que
    /// fazia a seta da esquerda cair no penultimo botao quando entrou o INF 110.
    static constexpr int RESERVA_DE_BOTOES = 16;
    static constexpr float INPUT_DELAY = 0.2f; // Exemplo de constante para o timer

    /// Onde a grade de materias comeca e termina, em fracao da altura.
    ///
    /// Ela ia de ponta a ponta da tela (margem de 1/12) e nao sobrava faixa
    /// para o titulo em cima nem para a linha de estado embaixo: a linha caia
    /// em cima dos losangos da ultima fileira.
    static constexpr float kTopoDaGrade = 0.19f;
    static constexpr float kFundoDaGrade = 0.78f;

    /// Quanto o nome de uma materia fechada fica abaixo do centro do losango.
    /// O losango tem 64 de altura, entao 44 deixa o nome logo embaixo dele.
    static constexpr float kNomeAbaixoDoCadeado = 44.0f;

    /// A linha que explica a materia em foco, entre a grade e o rodape.
    static constexpr float kLinhaDeEstado = 0.885f;

    // --- Membros de Propriedade (Ownership) ---
    std::unique_ptr<Font> mStageSelectFont{};

    // --- Membros de Estado e Observadores ---
    int mSelectedSubject{};
    float mInputTimer{}; // Timer pra mudar de botão selecionado

    /// Ver o BOTAO 2 em HandleSelectionInput: comeca true porque se chega aqui
    /// vindo do ranking com o botao ainda apertado.
    bool mRankingAnterior = true;

    // Vetor de ponteiros OBSERVADORES para fácil acesso aos botões.
    // A memória real é gerenciada pelo vetor mActors.
    std::vector<StageSelectButton*> mButtonObservers;

    /// A forma da grade, montada em CreateStageButtons junto com os botoes.
    /// mGrade[coluna][linha] = indice em mButtonObservers.
    ///
    /// Substitui as tres funcoes que descreviam a grade a mao (GetColumnFromIndex,
    /// GetColumnStartIndex, GetColumnSize). Elas tinham o layout de 4 colunas
    /// 1,4,4,1 fixo no codigo e nao acompanharam a entrada do INF 110.
    Navegacao::Grade mGrade;


    // Helper pra atualizar texto na HUD
    void UpdateStageInfo() const;
    Actor* mScoreInfoActor = nullptr;

    /// Por que a materia em foco esta fechada, ou que ela ainda nao tem
    /// professor, ou como o aluno foi nela. Sai da MESMA regra que fecha a
    /// materia - ver Materias::ExigenciaDe.
    void AtualizarLinhaDeEstado() const;
    Actor* mEstadoAtor = nullptr;

    /// Quem esta jogando, e o aviso de que visitante nao salva.
    void CriarIdentificacaoNaTela();
    Actor* mAlunoAtor = nullptr;
    Actor* mTrocarAtor = nullptr;

    /// Borda do botao de troca, e comeca em TRUE pelo mesmo motivo do
    /// mEntrarAnterior: trocar de aluno passou a ser o BOTAO 2, que e o mesmo
    /// botao com que se volta do ranking - e o ranking volta para ca. Com a
    /// borda em false, voltar do ranking com o botao ainda apertado jogava o
    /// aluno direto para a tela de matricula.
    bool mTrocarAnterior = true;

    /// Borda do ENTER, e comeca em TRUE de proposito.
    ///
    /// A cena anterior (identificacao) termina com o ENTER apertado - e ele que
    /// manda para ca. Com a borda comecando em false, o primeiro quadro desta cena
    /// via "ENTER apertado agora" e entrava direto na fase selecionada, sem o
    /// jogador ter escolhido nada. Comecando em true, o ENTER so conta depois de
    /// ser SOLTO e apertado de novo.
    bool mEntrarAnterior = true;
};