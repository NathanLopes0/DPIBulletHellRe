//
// Created by nslop on 21/11/2024.
//

#pragma once

#include "../../../Projectile.h"

class Boss;

/**
 * @class BossProjectile
 * @brief Uma especialização de Projétil que é disparada por um Boss.
 * Responsável por carregar seus componentes específicos e atualizar seus Behaviors.
 */
class BossProjectile : public Projectile {
public:
    explicit BossProjectile(Scene* scene, Boss* owner);
    ~BossProjectile() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] Boss* GetBossOwner() const;

    Vector2 GetPlayerPosition() const;

    /// Velocidade atual do jogador; (0,0) se nao houver jogador ou cena Battle.
    /// Usada pela mira preditiva dos caminhos.
    Vector2 GetPlayerVelocity() const;

    /// true quando ha cena Battle com jogador vivo. Permite ao chamador
    /// distinguir "jogador na origem" de "nao ha jogador".
    bool HasPlayer() const;

    Vector2 GetPlayerDirection() const;

    /**
     * @brief Define a folga que o projetil tem para alem da borda antes de morrer.
     *
     * A margem e: tamanho do sprite * emSprites + tamanho da tela / divisorDeTela.
     * Passar 0 em divisorDeTela remove a parcela proporcional a tela.
     *
     * Os dois numeros existem porque os projeteis do jogo usavam DUAS regras
     * diferentes. A geral era 1 sprite + um doze avos da tela; os baloes do Andre
     * tinham uma subclasse propria so para usar 2 sprites e nenhuma parcela de
     * tela, o que os faz morrer MAIS CEDO (para um sprite de 32, o balao morre em
     * -64 e os outros em -132).
     *
     * Parametrizar em vez de manter a subclasse: a diferenca e de numero, nao de
     * comportamento, e era o ultimo motivo para existir uma classe de projetil por
     * professor.
     */
    void DefinirMargemDeSaida(float emSprites, float divisorDeTela);

protected:
    // Implementação obrigatória do contrato da classe base.
    [[nodiscard]] bool virtual IsOffScreen() const override;

    /// Ver DefinirMargemDeSaida. Os padroes sao a regra geral, entao um projetil
    /// que nunca chama o metodo se comporta como antes desta mudanca.
    float mMargemEmSprites = 1.0f;
    float mMargemDivisorDeTela = 12.0f;

};
