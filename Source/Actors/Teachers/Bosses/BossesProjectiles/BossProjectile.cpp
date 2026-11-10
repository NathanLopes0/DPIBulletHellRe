//
// Created by nslop on 21/11/2024.
//

#include "BossProjectile.h"
#include "../../Boss.h"
#include "../../../../Motor/Game.h"         // Para GetWindowWidth/Height
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h" // Para GetComponent<>()
#include "../../../../Scenes/Battle/Battle.h"
#include "../../../Player/Player.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Regras/Destrutivel.h"

// 1. O construtor passa o 'owner' para o construtor da classe base 'Projectile'.
BossProjectile::BossProjectile(Scene* scene, Boss* owner)
    : Projectile(scene, owner) // Chama o construtor do pai com a cena E o dono
{

}

void BossProjectile::MarcarPadraoDeFabrica() {
    Projectile::MarcarPadraoDeFabrica();
    mRotacaoPadrao = mRotacionarComAVelocidade;
}

void BossProjectile::Reset() {
    Projectile::Reset();

    // A rotacao e aparencia, e aparencia volta ao padrao de fabrica no pool.
    mRotacionarComAVelocidade = mRotacaoPadrao;

    // A blindagem tambem e do disparo.
    mBlindado = false;

    mEnfeite = false;

    // E o posto: uma historia entregue no quadro voltaria do pool atirando.
    mPosto.clear();

    mGolpes = 0;

    if (auto* anim = GetComponent<DrawAnimatedComponent>()) {
        anim->SetColor(255, 255, 255);
        // O brilho de quem ia num aluno (Battle::MarcarOCaminhoDosAlunos) nao volta do pool.
        anim->DefinirBrilho(false);
    }
}

void BossProjectile::OnUpdate(float deltaTime)
{
    // 1.
    Projectile::OnUpdate(deltaTime);

    // O contorno de quem o tiro do aluno destroi (Destrutivel.h).
    if (auto* anim = GetComponent<DrawAnimatedComponent>()) {
        anim->DefinirContorno(Destrutivel::Pode(mDerrubavel, mBlindado, mAbrigo, mEnfeite),
                              SDL_Color{Destrutivel::kVermelho, Destrutivel::kVerde, Destrutivel::kAzul,
                                        Destrutivel::kOpacidade});
    }

    // 2.
    if (!mRotacionarComAVelocidade) return;

    const auto rb = GetComponent<RigidBodyComponent>();
    if (rb == nullptr) return;

    const Vector2 v = rb->GetVelocity();

    // Parado nao tem direcao.
    if (Math::NearZero(v.Length())) return;

    // O sprite e desenhado apontando para +X, e o eixo Y da tela cresce para baixo.
    SetRotation(Math::Atan2(-v.y, v.x));
}

// 3. Implementação obrigatória de IsOffScreen.
bool BossProjectile::IsOffScreen() const
{
    // Acessar os componentes de forma segura.
    if (auto drawComp = GetComponent<DrawAnimatedComponent>())
    {
        // 4.
        const auto pos = GetPosition();
        const auto game = mScene->GetGame();
        const auto windowWidth = static_cast<float>(game->GetWindowWidth());
        const auto windowHeight = static_cast<float>(game->GetWindowHeight());
        const auto spriteWidth = static_cast<float>(drawComp->GetSpriteWidth());
        const auto spriteHeight = static_cast<float>(drawComp->GetSpriteHeight());

        // "Zona de buffer".
        const float hBuffer = (mMargemDivisorDeTela > 0.0f) ? windowWidth / mMargemDivisorDeTela : 0.0f;
        const float vBuffer = (mMargemDivisorDeTela > 0.0f) ? windowHeight / mMargemDivisorDeTela : 0.0f;

        const float hFolga = spriteWidth * mMargemEmSprites + hBuffer;
        const float vFolga = spriteHeight * mMargemEmSprites + vBuffer;

        // Retorna 'true' se estiver fora dos limites.
        return pos.x < -hFolga ||
               pos.x > windowWidth + hFolga ||
               pos.y < -vFolga ||
               pos.y > windowHeight + vFolga;
    }

    // Failsafe se não houver componente de desenho.
    SDL_Log("Erro em BossProjectile.cpp - IsOffScreen: projetil nao tem DrawAnimatedComponent, retornando false");
    return false;
}

Vector2 BossProjectile::GetPlayerPosition() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->JogadorEm(mAlvo)) {
            return player->GetPosition();
        }
        SDL_Log("Erro em BossProjectile.cpp - GetPlayerPosition: nao foi possivel encontrar o Player"
                "Retornando vetor base");
        return {};
    }

    SDL_Log("Erro em BossProjectile.cpp - GetPlayerPosition: tentou encontrar cena Battle e nao achou."
            "Retornando vetor base");
    return {};
}

Vector2 BossProjectile::GetPlayerVelocity() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->JogadorEm(mAlvo)) {
            if (auto rb = player->GetComponent<RigidBodyComponent>()) {
                return rb->GetVelocity();
            }
        }
    }
    // Logar seria spam.
    return Vector2::Zero;
}

bool BossProjectile::HasPlayer() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        return battleScene->JogadorEm(mAlvo) != nullptr;
    }
    return false;
}

Vector2 BossProjectile::GetPlayerDirection() const {
    const Vector2 playerPos = GetPlayerPosition();
    Vector2 direction = playerPos - GetPosition();
    direction.Normalize();

    return direction;
}

void BossProjectile::DefinirMargemDeSaida(const float emSprites, const float divisorDeTela) {
    mMargemEmSprites = emSprites;
    mMargemDivisorDeTela = divisorDeTela;
}
