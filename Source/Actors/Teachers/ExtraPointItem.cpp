//
// Created by gensh on 24/11/2025.
//

#include "ExtraPointItem.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Game.h"

ExtraPointItem::ExtraPointItem(Scene* scene) : Actor(scene) {

    SetScale(2.0f);
    // 1. Visual
    auto dc = AddComponent<DrawAnimatedComponent>(
        "../Assets/Icons/DPIBHPowerIcon.png",
        "../Assets/Icons/DPIBHPowerIcon.json"
    );
    dc->AddAnimation("Idle", {0,1,2,3,4,5,6,7,8});
    dc->SetAnimation("Idle");

    // 2. Física (Movimento)
    // Vamos dar um empurrãozinho pra cima quando ele nasce, pra fazer um arco bonito
    auto rb = AddComponent<RigidBodyComponent>();
    rb->SetVelocity(Vector2(0.0f, -150.0f)); // Pula pra cima inicial

    // 3. Colisor (Para o Player pegar)
    auto radius = static_cast<float>(dc->GetSpriteWidth());
    AddComponent<CircleColliderComponent>(radius * GetScale());
}

void ExtraPointItem::OnUpdate(float deltaTime) {
    // Lógica de Gravidade "Artificial" simples
    // Queremos que ele caia com velocidade constante depois do pulo inicial
    // Ou usamos o RigidBody se ele tiver gravidade configurada.
    // Vamos fazer manual para ter controle total estilo Arcade:

    auto rb = GetComponent<RigidBodyComponent>();
    Vector2 currentVel = rb->GetVelocity();

    // Acelera para baixo até atingir a velocidade terminal (FALL_SPEED)
    if (currentVel.y < FALL_SPEED) {
        currentVel.y += 300.0f * deltaTime; // Gravidade
    }
    rb->SetVelocity(currentVel);

    // Destroi se sair da tela (Memory Management)
    if (GetPosition().y > mScene->GetGame()->GetWindowHeight() + 50) {
        SetState(ActorState::Destroy);
    }
}
