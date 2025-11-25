//
// Created by nslop on 03/10/2024.
//
#include <vector>
#include "Player.h"

#include "PlayerProjectile.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

#define NUM_PROJECTILES_VECTOR 500
#define ATK_TIMER_START_FULL 0.12f
#define INVINCIBILITY_TIMER 1.2f
#define PLAYER_SPEED_START 300.0f
#define BASE_SHOOT_FORWARD_SPEED 800.0f
#define BLINK_INVINCIBLE_FREQUENCY 0.1f

Player::Player(Scene* scene) :
    Actor(scene),
    playerSpeed(PLAYER_SPEED_START),
    atkTimer(ATK_TIMER_START_FULL),
    mMoving(false),
    isInvincible(false),
    invincibleTimer(0.0f)

{

    std::string spritePath = "../Assets/Player/DPIBHPlayer.png";
    std::string dataPath = "../Assets/Player/DPIBHPlayer.json";
    auto drawComp = AddComponent<DrawAnimatedComponent>(spritePath, dataPath);
    drawComp->AddAnimation("Moving", {0,1,2,3});
    drawComp->AddAnimation("Idle", {1});
    drawComp->SetAnimation("Idle");

    AddComponent<RigidBodyComponent>();
    auto colliderComp = AddComponent<CircleColliderComponent>(static_cast<float>(drawComp->GetSpriteWidth()) / 10.f);
    colliderComp->SetTag(ColliderTag::Player);

}


void Player::OnProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);

    MoveInput(keyState);
    ShootInput(keyState);
    SpecialInput(keyState);

}

void Player::OnUpdate(float deltaTime) {

    InvincibleUpdate(deltaTime);
    HandleAnimation();
    DecreaseAtkTimer(deltaTime);
    BorderLimitCheck();
    UpdateOverload(deltaTime);

}

void Player::InvincibleUpdate(float deltaTime) {
    if (!isInvincible) return;

    static float blink;
    if (invincibleTimer == 0.0f) {
        blink = BLINK_INVINCIBLE_FREQUENCY;
    }

    invincibleTimer += deltaTime;
    if (invincibleTimer >= INVINCIBILITY_TIMER) {
        isInvincible = false;
        invincibleTimer = 0.0f;
        GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
        blink = BLINK_INVINCIBLE_FREQUENCY;
        return;
    }

    auto blinking = GetComponent<DrawAnimatedComponent>()->GetIsVisible();
    blink += deltaTime;
    if (blink >= BLINK_INVINCIBLE_FREQUENCY) {
        blink = 0.0f;
        GetComponent<DrawAnimatedComponent>()->SetIsVisible(!blinking);
        blinking = !blinking;
    }






}
void Player::MoveInput(const Uint8 *keyState) {

    float newXSpeed = 0;
    float newYSpeed = 0;

    // ------ verificação de movimento CIMA/BAIXO ------
    if(keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
        newYSpeed -= playerSpeed;
    else if(keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
        newYSpeed += playerSpeed;

    // ------ verificação de movimento ESQUERDA/DIREITA ------
    if(keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_RIGHT])
        newXSpeed -= playerSpeed;
    else if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_LEFT])
        newXSpeed += playerSpeed;

    //vendo se está movendo ou não para mudar a variável mMoving (pra mudar a animação de acordo)
    if(newXSpeed == 0 && newYSpeed == 0) mMoving = false;
    else mMoving = true;

    GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(newXSpeed, newYSpeed));
}
void Player::ShootInput(const Uint8 *keyState) {

    if(keyState[SDL_SCANCODE_SPACE] && mOverloadTimer <= 0.0f) {
        Shoot();
    }

}
void Player::SpecialInput(const Uint8 *keyState) {

    if(keyState[SDL_SCANCODE_B] && mExtraPoints > 0 && mOverloadTimer <= 0.0f) {
        mExtraPoints--;
        mOverloadTimer = OVERLOAD_DURATION;
        if (const auto battle = dynamic_cast<Battle*>(GetScene())) {
            battle->OnPlayerUsedExtraPoint();
        }
    }

}

void Player::HandleAnimation() {

    if(mMoving) GetComponent<DrawAnimatedComponent>()->SetAnimation("Moving");
    else GetComponent<DrawAnimatedComponent>()->SetAnimation("Idle");

}

//ShootInput Subfunction
void Player::Shoot() {


    if (atkTimer > 0) return;

    auto battle = dynamic_cast<Battle*>(GetScene());
    if (!battle) return;

    auto projManager = battle->GetProjectileManager();
    if (!projManager) return;

    // TODO - MATANDO o Object Pooling - mudar pra chamada de função em ProjectileManager
    auto projectile = std::make_unique<PlayerProjectile>(GetScene(), this);

    // Posiciona o projétil na frente do jogador
    projectile->SetPosition(GetPosition());

    // Define velocidade inicial (sobe)
    projectile->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0.0f, -BASE_SHOOT_FORWARD_SPEED));

    // Ativa e torna visível
    projectile->ActivateProjectile();

    // Entrega ownership ao manager
    projManager->AddPlayerProjectile(std::move(projectile));

    atkTimer = ATK_TIMER_START_FULL;
}

//OnUpdate Subfunction
void Player::DecreaseAtkTimer(float deltaTime) {
    if(atkTimer > 0)
        atkTimer -= deltaTime;
}

void Player::BorderLimitCheck() {
    SDL_FRect bounds = mScene->GetPlayfieldBounds();
    auto pWidth = GetComponent<DrawAnimatedComponent>()->GetSpriteWidth();
    auto pHeight = GetComponent<DrawAnimatedComponent>()->GetSpriteHeight();

    SetPosition(Vector2(Math::Clamp(GetPosition().x, bounds.x + pWidth, bounds.x + bounds.w - pWidth),
            Math::Clamp(GetPosition().y, bounds.y + pHeight, bounds.y + bounds.h - pHeight)));
}

void Player::UpdateOverload(float deltaTime) {

    auto dc = GetComponent<DrawAnimatedComponent>();
    if (!dc) return;

    if (mOverloadTimer > 0.0f) {
        mOverloadTimer -= deltaTime;

        // --- LÓGICA DO PISCAR VERMELHO ---

        // 1. Calcula a velocidade do piscar (Frequência)
        // Quanto MENOR o timer (mais perto de acabar), MAIOR a frequência.
        // Começa com 5.0f e aumenta + 15.0f para cada segundo que passou.
        float freq = 5.0f + (OVERLOAD_DURATION - mOverloadTimer) * 15.0f;

        mPulseTimer += freq * deltaTime;
        // 2. Oscilador (Seno)
        // Gera um número entre -1 e 1
        float osc = Math::Sin(mPulseTimer);

        // 3. Define a cor
        // Quando oscilador for 1 -> Branco (255, 255, 255)
        // Quando oscilador for -1 -> Vermelho (255, 0, 0)

        // Mapeia [-1, 1] para [0, 255] nos canais Verde e Azul
        Uint8 gb = static_cast<Uint8>((osc + 1.0f) * 127.5f);

        // Vermelho sempre 255. Verde e Azul oscilam.
        // Se GB for 255 -> Branco. Se GB for 0 -> Vermelho Puro.
        dc->SetColor(255, gb, gb);


    } else {
        // --- ESTADO NORMAL ---
        // Garante que o player volte a ser branco quando o tempo acabar

        dc->SetColor(255, 255, 255);
        mPulseTimer = 0.0f;
    }
}

void Player::AddExtraPoint() {
    mExtraPoints = Math::Clamp(mExtraPoints + 1, 0, 3);
}
