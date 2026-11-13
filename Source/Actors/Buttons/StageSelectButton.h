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

    /// A arte (DPIBHLosango) e pixel art com a metade do tamanho do botao.
    static constexpr float kEscalaDoLosango = 2.0f;
    static constexpr int kTamanhoDoNome = 17;
    /// O reflexo do foco: 16 quadros, pouco mais de um segundo por volta.
    static constexpr float kQuadrosPorSegundo = 14.0f;

    /// Ao ganhar o foco o losango cresce kPulo e volta em kDuracaoDoPulo segundos.
    static constexpr float kPulo = 0.12f;
    static constexpr float kDuracaoDoPulo = 0.2f;
    float mPulo = 0.0f;
    bool mEstavaSelecionado = false;

    /// Observador: quem e dono e a cena.
    Actor* mRotuloFechado = nullptr;
};

#endif // DPIBULLETHELLRE_STAGESELECTBUTTON_H.
