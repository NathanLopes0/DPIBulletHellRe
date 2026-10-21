//
// O chefe da INF 340 - Linguagens de Programacao.
//

#pragma once

#include "../Boss.h"

/**
 * @class Luizinho
 * @brief O professor de Linguagens de Programacao (INF 340).
 *
 * A IDEIA DO CHEFE: ele e conhecido por saber de onde vem cada expressao, quase
 * sempre do latim, e por passar isso para os alunos. A batalha faz o mesmo: em
 * cada fase o projetil obedece ao sentido LITERAL da palavra latina por tras de
 * um assunto da ementa, e o placar mostra a palavra e o que ela queria dizer.
 *
 *   fase 1  Amarracoes          ligare        atar
 *   fase 2  Encapsulamento      in capsula    na caixinha
 *   fase 3  Gerencia de memoria memento mori  lembra-te de que morreras
 *   final   Sequenciadores      sequi         seguir
 *
 * O que nao cabe em dados mora aqui: a mira do leque estatico da fase 1, que
 * depende de onde o aluno esta no instante do disparo.
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/luizinho/.
 */
class Luizinho : public Boss {
public:
    explicit Luizinho(Scene* scene);
    ~Luizinho() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
};
