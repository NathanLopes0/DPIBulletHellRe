//
// O projetil de chefe, unico tipo concreto.
//

#pragma once

#include "BossProjectile.h"

/**
 * @class ProjetilDeChefe
 * @brief O tipo concreto de todo projetil disparado por um chefe.
 *
 * SUBSTITUIU CINCO CLASSES que eram a mesma coisa. SallesBossProjectile,
 * SallesDoubleListProjectile, JulioBossProjectile, RicardoBossProjectile e
 * AndreBossProjectile tinham corpo identico: um construtor que repassava os
 * argumentos e um Reset() que chamava Projectile::Reset() e depois
 * SetAnimation("<nome fixo>").
 *
 * Esse Reset() era REDUNDANTE desde que Projectile ganhou
 * MarcarPadraoDeFabrica: a base ja guarda a animacao com que a fabrica
 * construiu o objeto e a devolve no Reset(). As cinco subclasses estavam
 * reescrevendo, com o nome fixo no codigo, o valor que a base acabara de
 * restaurar sozinha - e qualquer uma que tivesse o nome errado esconderia o erro
 * ate alguem olhar.
 *
 * O comentario que justificava as subclasses dizia que sem elas "os projeteis do
 * Julio dividiriam pool com os de outro professor". Isso nao era verdade:
 * mPool e membro NAO estatico da fabrica, e cada chefe cria a sua propria
 * instancia de fabrica para cada nome de projetil. A separacao dos pools vem de
 * haver uma fabrica por tipo, nao de haver um tipo C++ por projetil.
 *
 * Esta classe nao tem corpo proprio de proposito. Ela existe porque
 * ProjectilePool<T> precisa de um tipo concreto e porque BossProjectile e quem
 * sabe morrer ao sair da tela e consultar a posicao do jogador.
 */
class ProjetilDeChefe : public BossProjectile {
public:
    explicit ProjetilDeChefe(Scene* scene, Boss* owner)
        : BossProjectile(scene, owner) {}
};
