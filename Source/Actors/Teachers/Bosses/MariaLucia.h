//
// A chefe da INF 221 - Engenharia de Software.
//

#pragma once

#include "../Boss.h"

/**
 * @class MariaLucia
 * @brief A professora de Engenharia de Software (INF 221).
 *
 * TODO O BALANCEAMENTO DELA VIVE EM DADOS, como o do Vladimir: as fases em
 * Assets/Attacks/marialucia/fases.json e as regras de projetil no regras.json
 * ao lado. Esta classe existe para dar um tipo a chefe e um lugar para o que
 * precisar de C++ depois - hoje, nada.
 *
 * A IDEIA DA CHEFE, para quem for mexer nas fases: as outras disciplinas do
 * jogo sao OBJETOS - uma estrutura de dados, uma linguagem, uma primitiva de
 * concorrencia. Engenharia de Software e um PROCESSO, e por isso ela e a unica
 * chefe em que o TEMPO e parte do problema.
 *
 * Onde isso aparece: na fase 3 o campo NAO se limpa. O debito para onde cai e
 * fica, e o segundo quinze da fase e a consequencia do segundo dois. Em todas
 * as outras fases do jogo, de todos os outros chefes, o campo no fim esta como
 * estava no comeco.
 */
class MariaLucia : public Boss {
public:
    explicit MariaLucia(Scene* scene);
    ~MariaLucia() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
};
