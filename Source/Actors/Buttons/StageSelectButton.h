//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECTBUTTON_H
#define DPIBULLETHELLRE_STAGESELECTBUTTON_H

#include <string>
#include "Button.h"
#include "../../Motor/Game.h"
#include "../../Motor/Math.h"

class Font;
class StageSelectButton : public Button {

public:
    explicit StageSelectButton(Scene* scene, const std::string& buttonText,
                               int subject, const std::string& fontPath, bool isLocked);
    ~StageSelectButton() override = default;

    void OnUpdate(float deltaTime) override;
    void SetText(const std::string& newText);

    /// Quem sabe disso e a cena, que tem o progresso do aluno.
    void SetAprovado(const bool aprovado) { mAprovado = aprovado; }

    /// O rotulo desenhado embaixo de uma materia fechada. E so um observador: o ator pertence a
    /// cena, como todos os outros.
    void SetRotuloFechado(Actor* rotulo) { mRotuloFechado = rotulo; }

    [[nodiscard]] int GetSubject() const { return mSubject; }

private:

    /// O nome de uma materia fechada: visivel, mas claramente apagado - e
    /// branco quando ela esta em foco, que e quando o aluno quer le-lo.
    static inline const Vector3 kNomeFechado{0.56f, 0.56f, 0.68f};
    static inline const Vector3 kNomeFechadoEmFoco{1.00f, 1.00f, 1.00f};

    int mSubject{};
    std::unique_ptr<Font> mFont;
    bool mIsLocked;

    bool mAprovado = false;

    /// A arte (DPIBHLosango) tem o dobro do tamanho do botao.
    static constexpr float kEscalaDoLosango = 0.5f;
    static constexpr int kTamanhoDoNome = 17;

    /// Ao ganhar o foco o losango cresce kPulo e volta em kDuracaoDoPulo segundos; enquanto tem o
    /// foco, respira kRespiro, kRitmoDoRespiro radianos por segundo.
    static constexpr float kPulo = 0.14f;
    static constexpr float kDuracaoDoPulo = 0.22f;
    static constexpr float kRespiro = 0.03f;
    static constexpr float kRitmoDoRespiro = 4.0f;
    float mPulo = 0.0f;
    float mRespiro = 0.0f;
    bool mEstavaSelecionado = false;

    /// Observador: quem e dono e a cena.
    Actor* mRotuloFechado = nullptr;
};

#endif // DPIBULLETHELLRE_STAGESELECTBUTTON_H.
