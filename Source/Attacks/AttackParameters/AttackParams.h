//
// Created by gensh on 18/10/2025.
//
// AttackParams.h
#pragma once

#include "../../Math.h" // Para Vector2

class ProjectileFactory;
class Actor;
/**
 * @struct AttackParams
 * @brief Um "pacote de dados" universal para configurar qualquer ataque.
 *
 * @philosophy Esta struct contém os parâmetros MAIS COMUNS que 90% dos ataques
 * (Circle, Wave, Angled) precisarão. Todos os membros têm valores padrão sãos.
 *
 * PARA ATAQUES ESPECIAIS: Se um ataque precisa de um parâmetro único
 * (ex: 'frequenciaDeOscilacao'), criar um novo struct (ex: WobbleParams)
 * que HERDA de AttackParams. A Strategy pode então usar um
 * dynamic_cast para checar se recebeu o pacote de parâmetros especial.
 */
struct AttackParams
{
    // --- 1. PARÂMETROS CONTEXTUAIS (Definidos pelo Chamador) ---
    // Onde o ataque acontece no mundo.

    /** @brief Dono do ataque */
    Actor* owner{};

    /** @brief Fábrica que criará o projétil */
    ProjectileFactory* spawner{};

    /** @brief De onde o ataque se origina (ex: posição do Boss). */
    Vector2 firePosition = Vector2::Zero;

    /**
     * @brief Direção principal do ataque (vetor normalizado).
     * (ex: em direção ao player).
     * Padrão é (0, 1) = para baixo.
     */
    Vector2 fireDirection = Vector2(0.0f, 1.0f);


    // --- 2. PARÂMETROS DE GAMEPLAY (Knobs de Design) ---
    // Parâmetros que o designer pode querer ajustar a cada disparo.

    /** @brief Quantidade de projéteis a serem criados. */
    int numProjectiles = 10;

    /** @brief Velocidade base dos projéteis criados. */
    float projectileSpeed = 200.0f;

    /** @brief O ângulo total de um "leque" de ataque (em graus). */
    float angle = 90.0f;

    /**
     * @brief O ângulo central do leque (em graus).
     * 0=Direita, 90=Baixo, 180=Esquerda, 270=Cima.
     * (Pode ser ignorado se fireDirection for usado)
     */
    float centralAngle = 90.0f;

    /**
     * @brief Atraso entre a criação de cada projétil (em segundos).
     * Usado por ataques tipo "Wave" ou "Stream".
     */
    float creationSpeed = 0.05f;
};