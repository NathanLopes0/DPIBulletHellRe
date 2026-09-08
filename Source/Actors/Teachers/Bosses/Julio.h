//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#pragma once

#include "../Boss.h"

/**
 * @class Julio
 * @brief Boss da matéria INF 420.
 *
 * Tema: um modelo sendo treinado. As três fases percorrem exploração aleatória,
 * convergência por gradiente e overfitting — cada uma com um padrão de
 * movimento e de projétil correspondente.
 *
 * Toda a customização passa pelos dois ganchos do Boss; ExecuteAttack não é
 * sobrescrito.
 */
class Julio : public Boss {
public:
    explicit Julio(Scene* scene);
    ~Julio() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    /**
     * @brief Gancho 1 — ajusta os parâmetros ANTES do disparo.
     *
     * Fase 1 espalha a mira aleatoriamente (exploração), fase 2 aponta
     * exatamente para o jogador (convergência) e fase 3 aponta para o jogador
     * com um leque largo (variância alta).
     */
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;

    /**
     * @brief Gancho 2 — customiza os projéteis DEPOIS de criados.
     *
     * Pinta a rajada inteira com a animação da fase atual. Diferente do André,
     * que sorteia uma cor, aqui a cor é determinística: ela comunica ao jogador
     * em que fase da luta ele está.
     */
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;
};
