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

private:
    /**
     * Por onde passa a fresta do anel, em graus.
     *
     * UM ANEL COM FRESTA FIXA E UM ANEL QUE NAO AMEACA. Foi a licao da fase 3
     * do Vladimir: CircleSpreadAttack distribui os angulos a partir de zero,
     * sempre, entao as aberturas ficam nas mesmas direcoes em todo disparo e
     * quem estiver numa delas fica nela a batalha inteira. Aqui o leque e uma
     * WaveAttack - que RESPEITA centralAngle - e este campo o faz girar.
     */
    float mFrestaAtual = 90.0f;

    /**
     * Quanto a fresta anda entre uma rajada e a outra.
     *
     * 137 GRAUS, E NAO UM DIVISOR DE 360. Com 90 ou 120 a fresta voltaria ao
     * mesmo lugar a cada quatro ou tres rajadas e o jogador decoraria o ciclo;
     * com 137 ela so repete depois de 360 disparos. E o mesmo motivo de os
     * cooldowns da fase final do Vladimir serem primos entre si.
     */
    static constexpr float kPassoDaFresta = 137.0f;
};
