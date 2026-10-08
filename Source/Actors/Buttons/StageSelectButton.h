//
// Created by nslop on 26/08/2024.
//

#ifndef DPIBULLETHELLRE_STAGESELECTBUTTON_H
#define DPIBULLETHELLRE_STAGESELECTBUTTON_H


#include <string>
#include "Button.h"
#include "../../Game.h"
#include "../../Math.h"

class Font;
class StageSelectButton : public Button {

public:
    explicit StageSelectButton(Scene* scene, const std::string& buttonText,
                               int subject, const std::string& fontPath, bool isLocked);
    ~StageSelectButton() override = default;

    void OnUpdate(float deltaTime) override;
    void SetText(const std::string& newText);

    /// @brief Pinta o botao de verde: a materia ja foi aprovada.
    /// Quem sabe disso e a cena, que tem o progresso do aluno.
    void SetAprovado(const bool aprovado) { mAprovado = aprovado; }

    /// @brief O rotulo desenhado EMBAIXO de uma materia fechada.
    ///
    /// E so um observador: o ator pertence a cena, como todos os outros. O
    /// botao guarda o ponteiro para poder acender o nome quando ganha o foco -
    /// dentro do losango fechado nao cabe texto, porque o cadeado ocupa o meio.
    void SetRotuloFechado(Actor* rotulo) { mRotuloFechado = rotulo; }

    [[nodiscard]] int GetSubject() const { return mSubject; }
    [[nodiscard]] bool IsLocked() const { return mIsLocked; }

private:

    /// O nome de uma materia fechada: visivel, mas claramente apagado - e
    /// branco quando ela esta em foco, que e quando o aluno quer le-lo.
    static inline const Vector3 kNomeFechado{0.56f, 0.56f, 0.68f};
    static inline const Vector3 kNomeFechadoEmFoco{1.00f, 1.00f, 1.00f};

    int mSubject{};
    std::unique_ptr<Font> mFont;
    bool mIsLocked;

    /// Ver SetAprovado.
    bool mAprovado = false;

    /// Ver SetRotuloFechado. OBSERVADOR: quem e dono e a cena.
    Actor* mRotuloFechado = nullptr;
};


#endif //DPIBULLETHELLRE_STAGESELECTBUTTON_H
