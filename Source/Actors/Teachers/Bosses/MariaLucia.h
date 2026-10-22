//
// A chefe da INF 221 - Engenharia de Software.
//

#pragma once

#include "../Boss.h"

/**
 * @class MariaLucia
 * @brief A professora de Engenharia de Software (INF 221), que tambem da IHC.
 *
 * TODO O BALANCEAMENTO DELA VIVE EM DADOS: as fases em
 * Assets/Attacks/marialucia/fases.json e as regras de projetil no regras.json
 * ao lado.
 *
 * A IDEIA DA CHEFE: as outras disciplinas do jogo sao OBJETOS - uma estrutura
 * de dados, uma linguagem, uma primitiva de concorrencia. Engenharia de
 * Software e um PROCESSO, e cada fase dela e um processo de verdade. A fase 1 e
 * o MODELO CASCATA: cinco etapas em ordem ("etapas" em fases.json), cada uma com
 * o seu ataque, e ela descendo a escada do diagrama, um degrau por etapa
 * (EscadaStrategy). Nada disso precisa de C++ - as etapas sao do motor.
 */
class MariaLucia : public Boss {
public:
    explicit MariaLucia(Scene* scene);
    ~MariaLucia() override = default;

    void OnUpdate(float deltaTime) override;
};
