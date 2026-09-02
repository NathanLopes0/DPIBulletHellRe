//
// Created by nslop on 26/11/2024.
//

#include <memory>
#include "CircleSpreadAttack.h"

#include <complex>
#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../../Actors/Actor.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Attacks/AttackParameters/AttackParams.h"


CircleSpreadAttack::CircleSpreadAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{

}

std::vector<std::unique_ptr<Projectile>> CircleSpreadAttack::Execute(const AttackParams& params) {

    // Retorno de unique_ptr
    std::vector<std::unique_ptr<Projectile>> projectiles;

    // Lógica de gameplay lida de params
    const int numProjectiles = params.numProjectiles;
    const float projectileSpeed = params.projectileSpeed;

    // Evita divisão por zero se numProjectiles for 0
    if (numProjectiles == 0) {
        return projectiles;
    }

    // Reserva espaço no vetor para eficiência
    projectiles.reserve(numProjectiles);

    // lógica matemática
    const float angleStep = 360.0f / static_cast<float>(numProjectiles);

    for(int i = 0; i < numProjectiles; i++) {

        auto projectile = Acquire();

        // Failsafe: Se a fábrica falhar, pula este projétil
        if (!projectile) {
            SDL_Log("Failed to create projectile");
            continue;
        }

        // Posicionamento SEMPRE explicito. A premissa antiga ("o projetil ja
        // nasce na posicao do dono") valia quando todo projetil era construido
        // do zero; com Object Pooling, um objeto reciclado mantem a posicao
        // onde MORREU. A condicao antiga era && (bastava x ou y ser zero para
        // nao posicionar nada), e nesse caso o projetil reciclado nascia fora
        // da tela e morria no mesmo frame - o ataque sumia sem erro nenhum.
        // Boss::ExecuteAttack sempre preenche firePosition, entao nao ha caso
        // em que nao queiramos aplica-la.
        projectile->SetPosition(params.firePosition);

        // lógica de direção de cada projétil
        const float xCoordRad = Math::Cos(static_cast<float>(i) * angleStep * Math::Pi / 180);
        const float yCoordRad = Math::Sin(static_cast<float>(i) * angleStep * Math::Pi / 180);
        auto directionVector = Vector2(xCoordRad, yCoordRad);
        directionVector.Normalize();

        //xc = cos(angle * pi / 180)
        //arccos(xc) = angle * pi / 180

        // Velocidade usa o projectileSpeed do params, lida lá em cima da função
        auto rb = projectile->GetComponent<RigidBodyComponent>();
        if (!rb) {
            SDL_Log("Erro em CircleSpreadAttack: projetil sem RigidBodyComponent, pulando.");
            continue;
        }
        rb->SetVelocity(directionVector * projectileSpeed);

        // Transferência de posse (IMPORTANTÍSSIMO)
        //    Usar std::move para mover o unique_ptr 'projectile'
        //    para dentro do vetor.
        projectiles.push_back(std::move(projectile));
    }

    return projectiles; // Retorna o vetor cheio de projéteis E dono de todos
}

