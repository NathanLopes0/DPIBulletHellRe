//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#pragma once

#include "../Boss.h"

/**
 * @class Vitor
 * @brief O professor de Programacao Concorrente e Distribuida (INF 310).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/vitor/fases.json e as regras de projetil no regras.json ao
 * lado. O que mora aqui e a UNICA coisa que dados nao sabem fazer - girar a
 * fresta da secao critica entre uma rajada e a outra.
 *
 * A IDEIA DO CHEFE: concorrencia e "varias coisas ao mesmo tempo", e e por isso
 * que a fase 3 dele se divide em dois corpos. As duas primeiras fases existem
 * para serem aprendidas SEPARADAS, porque a terceira as poe juntas:
 *
 *   fase 1  TAREFAS EM PARALELO - varias faixas descendo de uma vez
 *   fase 2  EXCLUSAO MUTUA      - um anel com UMA fresta, e a fresta gira
 *   fase 3  FORK                - dois corpos, um fazendo cada uma das duas
 */
class Vitor : public Boss {
public:
    explicit Vitor(Scene* scene);
    ~Vitor() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

};
