//
// Created by nslop on 25/09/2024.
//

#pragma once

#include "../Boss.h"

class Salles : public Boss {

public:
    Salles(Scene* scene);
    ~Salles() override = default;
    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams &params, const std::string &stateName) override;

private:
    /// Quantos graus a arvore da fase 3 erra o jogador DE PROPOSITO quando mira
    /// nele. Ver CustomizeAttackParams: e o que impede o corredor vazio do eixo
    /// de cair em cima de quem esta parado.
    ///
    /// 4 graus porque o jogador costuma estar a uns 450px do chefe, e meio
    /// espacamento de folha (30px) a essa distancia da pouco menos de 4 graus.
    static constexpr float kDesvioDaBusca = 4.0f;

    /// Para que lado a busca desceu da ultima vez. Alterna.
    float mLadoDaBusca = 1.0f;
};

