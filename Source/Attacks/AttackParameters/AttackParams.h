//
// Created by gensh on 18/10/2025.
//
#pragma once

#include "../../Motor/Math.h" // Para Vector2

class ProjectileFactory;
class Actor;
/// Um "pacote de dados" universal para configurar qualquer ataque.
struct AttackParams
{

    virtual ~AttackParams() = default;
    // ---- 1. parâmetros contextuais (Definidos pelo Chamador) ----

    /// De onde o ataque se origina (ex: posição do Boss).
    Vector2 firePosition = Vector2::Zero;

    /// Direção principal do ataque (vetor normalizado). (ex: em direção ao player).
    Vector2 fireDirection = Vector2(0.0f, 1.0f);

    // ---- 2. parâmetros de gameplay (Knobs de Design) ----

    /// Quantidade de projéteis a serem criados.
    int numProjectiles = 10;

    /// Velocidade base dos projéteis criados.
    float projectileSpeed = 200.0f;

    /// O ângulo total de um "leque" de ataque (em graus).
    float angle = 90.0f;

    /// O ângulo central do leque (em graus). 0=Direita, 90=Baixo, 180=Esquerda, 270=Cima. (Pode ser
    /// ignorado se fireDirection for usado)
    float centralAngle = 90.0f;

    /// Atraso entre a criação de cada projétil (em segundos). Usado por ataques tipo "Wave".
    float creationSpeed = 0.05f;
};