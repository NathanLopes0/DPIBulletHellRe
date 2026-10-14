//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#pragma once

#include "../Boss.h"

/**
 * @class Vladimir
 * @brief O professor de Linguagens Formais e Automatos (INF 331).
 *
 * TODO O BALANCEAMENTO DELE VIVE EM DADOS, como o do Ricardo: as fases em
 * Assets/Attacks/vladimir/fases.json e as regras de projetil no regras.json ao
 * lado. Esta classe existe para dar um tipo ao chefe e um lugar para o que
 * precisar de C++ depois - hoje, nada.
 *
 * A IDEIA DO CHEFE, para quem for mexer nas fases: ele e o professor de
 * LINGUAGENS nos dois sentidos - as formais da disciplina e as estrangeiras da
 * diretoria de Relacoes Internacionais. Dai vem o que o separa dos outros
 * cinco: os outros surpreendem, e ele ANUNCIA. Cada rajada e uma palavra de
 * uma linguagem, e os projeteis sao os simbolos dela.
 */
class Vladimir : public Boss {
public:
    explicit Vladimir(Scene* scene);
    ~Vladimir() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
};
