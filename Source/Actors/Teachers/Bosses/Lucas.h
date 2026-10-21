//
// O chefe da INF 321 - Programacao Web.
//

#pragma once

#include "../Boss.h"

/**
 * @class Lucas
 * @brief O professor de Programacao Web (INF 321).
 *
 * A IDEIA DO CHEFE: ele da a materia na ordem classica - HTML, CSS, JavaScript
 * - e no fim ensina o que ele gosta de verdade, que e Elixir no back-end. A
 * batalha conta a mesma historia. As duas primeiras fases sao o front certinho
 * e geometrico; a terceira, a que decide, e o Elixir, onde o que o aluno
 * derruba volta sozinho ("let it crash"); a final mistura tudo.
 *
 *   fase 1  HTML        caixas aninhadas, uma div dentro da outra
 *   fase 2  CSS         a fileira que se redistribui, como um flexbox
 *   fase 3  Elixir      processos que o supervisor reinicia
 *   final   full stack  o front e o back ao mesmo tempo
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/lucas/.
 */
class Lucas : public Boss {
public:
    explicit Lucas(Scene* scene);
    ~Lucas() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
};
