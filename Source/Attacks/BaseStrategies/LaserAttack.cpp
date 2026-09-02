//
// Created by gensh on 01/04/2026.
//

#include "LaserAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Player/Player.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"


class DrawAnimatedComponent;

LaserAttack::LaserAttack(ProjectileFactory *spawner, Actor *owner)
    : IAttackStrategy(spawner, owner)
{

}

std::vector<std::unique_ptr<Projectile> > LaserAttack::Execute(const AttackParams &params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const int numProjectiles = params.numProjectiles;
    const float projectileSpeed = params.projectileSpeed;

    // NOTA: uma criação extra "basePrototypeProjectile" existia aqui antes,
    // mas nunca era usada em lugar nenhum (nem lida, nem devolvida). Com
    // Object Pooling, mantê-la faria essa strategy "vazar" um objeto do pool
    // a cada Execute(), então foi removida.

    // Validado UMA vez, antes do loop: antes 'battle' e 'GetPlayer()' eram
    // dereferenciados sem checagem alguma dentro do loop.
    const auto battle = dynamic_cast<Battle*>(mOwner->GetScene());
    if (!battle || !battle->GetPlayer()) {
        SDL_Log("LaserAttack::Execute: sem cena Battle ou sem Player. Ataque cancelado.");
        return projectiles;
    }
    const Vector2 playerPos = battle->GetPlayer()->GetPosition();

    for (int i = 0; i < numProjectiles; i++) {

        auto projectile = Acquire();
        if (!projectile) {
            SDL_Log("Failed to create projectile");
            continue;
        }

        // Posicionamento sempre explicito (mesmo motivo do CircleSpreadAttack:
        // um projetil vindo do pool nao nasce na posicao do dono).
        projectile->SetPosition(params.firePosition);

        auto drawComp = projectile->GetComponent<DrawAnimatedComponent>();
        if (!drawComp) {
            SDL_Log("LaserAttack: projetil sem DrawAnimatedComponent, pulando.");
            continue;
        }

        // Atraso entre projeteis: tempo para o anterior percorrer a propria
        // altura, formando uma linha continua.
        const int sH = drawComp->GetSpriteHeight();
        const float timeToSpawn = (projectileSpeed > 0.0f)
                                      ? static_cast<float>(sH) / projectileSpeed
                                      : 0.0f;

        // Angulo do projetil ATE o player. O Atan2 estava com os operandos
        // invertidos (projetil - player), apontando exatamente para o lado
        // oposto, e gravava o resultado em GRAUS num campo que
        // Actor::GetForward() le como radianos - duas coisas erradas somadas.
        // O -y compensa GetForward() ser (cos, -sin) enquanto o eixo y da tela
        // cresce para baixo: com este sinal, GetForward() devolve exatamente a
        // direcao normalizada ate o player.
        const Vector2 toPlayer = playerPos - projectile->GetPosition();
        projectile->SetRotation(Math::Atan2(-toPlayer.y, toPlayer.x));

        // TODO - dar um jeito do laser ir todos os projeteis em uma direção só.
        // TODO - tentar dar uma direção, desativar, e depois ativar dnovo usando a direção anterior

        //Insere o comportamento de ativação
        auto pDirection = projectile->GetForward();
        pDirection.Normalize();

        // TODO - ler o comentario abaixo dnovo
        // ao inserir o Activate Behavior, o comportamento desativa o projétil até que chegue o tempo ativado. Lembrar disso...
        // talvez o Activate Behavior deveria apenas ativar, e eu desativar manualmente tbm... então cada Activate viria com um Deactivate antes,
        // apesar de eu escrever mais, o Activate faria mais sentido com oq ele deveria apenas fazer, e pode acabar sendo mais personalizavel.
        projectile->insertBehavior<ActivateBehavior>(i * timeToSpawn, pDirection * projectileSpeed);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
