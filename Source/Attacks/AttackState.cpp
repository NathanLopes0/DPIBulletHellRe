//
// Created by gensh on 19/10/2025.
//

#include "AttackState.h"

#include <SDL_log.h>

#include "../Components/AIComponents/FSMComponent.h"
#include "../Actors/Teacher/Boss.h"
#include "../Scenes/Scene.h"
#include "IMovementStrategy.h"
#include "../Game.h"
#include "../Actors/Projectile.h"

AttackState::AttackState(FSMComponent* fsm, const std::string& name,
                         float stateDuration, float attackInterval,
                         std::unique_ptr<IMovementStrategy> moveStrategy)
    : FSMState(fsm, name), // <-- Chama o construtor base
      mOwnerBoss(nullptr), // Inicia nulo
      mStateDuration(stateDuration),
      mAttackInterval(attackInterval),
      mMovementStrategy(std::move(moveStrategy)),
      mAttackTimer(0.0f)
{
}

void AttackState::AddAttackStrategy(std::unique_ptr<IAttackStrategy> strategy) {
    mAttackStrategies.push_back(std::move(strategy));
}

void AttackState::Start() {

    mOwnerBoss = dynamic_cast<Boss*>(mFSM->GetOwner());

    if (!mOwnerBoss) {
        SDL_Log("AttackState::Start - FSM Owner is not a Boss!");
        return;
    }

    // --- O resto da nossa lógica de "OnEnter" ---
    if (mMovementStrategy) {
        mOwnerBoss->SetMovementStrategy(mMovementStrategy.get());
    }
    mAttackTimer = 0.0f; // Ataca no primeiro frame
}

void AttackState::Update(float deltaTime) {
    // Se o Start() falhou, não faz nada.
    if (!mOwnerBoss) return;

    // Atualiza o timer de INTERVALO de ataque
    mAttackTimer -= deltaTime;

    // Lógica de Ataque
    if (mAttackTimer <= 0.0f) {
        // Preenche os parâmetros
        AttackParams params;
        params.owner = mOwnerBoss;
        //params.spawner = mOwnerBoss->GetProjectileFactory();
        params.firePosition = mOwnerBoss->GetPosition();
        // adicionar os outros parametros de ataque


        for (const auto& strategy : mAttackStrategies) {
            auto projectiles = strategy->Execute(params);
            //mOwnerBoss->GetScene()->AddProjectiles(std::move(projectiles));
        }

        mAttackTimer = mAttackInterval;
    }
}

void AttackState::Exit() {
    if (mOwnerBoss) {
        //mOwnerBoss->SetMovementStrategy(nullptr); // Para de mover
    }
    mOwnerBoss = nullptr; // Limpa o ponteiro
}

void AttackState::HandleStateTransition(float stateTime) {
    // A FSM passa o tempo total que esteve neste estado.
    // Nós usamos isso para decidir a transição.
    if (stateTime >= mStateDuration) {
        // Como ele sabe para qual estado ir?
        // A FSMComponent provavelmente tem um mapa de transições.
        // ex: mTransitions["Phase1"] = "SpellCard"
        // Precisamos ver a FSMComponent para ter certeza.

        // Por enquanto, vamos supor que a FSM tem um método
        // para ir para o "próximo" estado definido.
        //mFSM->RequestTransitionToNext(); // Estou inventando, preciso ver o FSMComponent.h
    }
}