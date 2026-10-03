//
// Created by nslop on 21/11/2024.
//
// BossProjectile.cpp (VERSÃO REFATORADA E MODERNA)

#include "BossProjectile.h"
#include "../../Boss.h"
#include "../../../../Game.h"         // Para GetWindowWidth/Height
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h" // Para GetComponent<>()
#include "../../../../Scenes/Battle/Battle.h"
#include "../../../Player/Player.h"
#include "../../../../Components/RigidBodyComponent.h"


// 1. O construtor passa o 'owner' para o construtor da classe base 'Projectile'.
BossProjectile::BossProjectile(Scene* scene, Boss* owner)
    : Projectile(scene, owner) // Chama o construtor do pai com a cena E o dono
{

}



void BossProjectile::OnUpdate(float deltaTime)
{
    // 1. Executa a lógica base (verificar se saiu da tela, update nos Behaviors).
    Projectile::OnUpdate(deltaTime);

    // 2. Gira o sprite para acompanhar o movimento, quando a arte pede isso.
    //
    // DEPOIS da base de proposito: os Behaviors mexem na velocidade neste mesmo
    // quadro, e girar antes deixaria a seta um quadro atrasada em relacao ao
    // caminho - visivel justamente nas curvas, que e onde isto importa.
    if (!mRotacionarComAVelocidade) return;

    const auto rb = GetComponent<RigidBodyComponent>();
    if (rb == nullptr) return;

    const Vector2 v = rb->GetVelocity();

    // Parado nao tem direcao. Mantem a ultima em vez de saltar para zero: a
    // lista duplamente encadeada PARA por 0,8 s antes de voltar, e sem isto
    // todas as setas apontariam para a direita durante a pausa.
    if (Math::NearZero(v.Length())) return;

    // O sprite e desenhado apontando para +X, e o eixo Y da tela cresce para
    // baixo - dai o sinal trocado, a mesma conta do LaserAttack.
    //
    // HA UM SEGUNDO SINAL, em DrawAnimatedComponent::Draw, e os dois NAO se
    // cancelam: este traz a tela para a convencao do Actor (anti-horario, Y
    // para cima), e aquele leva a convencao do Actor para a do SDL (horario).
    // Sao duas fronteiras diferentes.
    SetRotation(Math::Atan2(-v.y, v.x));
}

// 3. Implementação obrigatória de IsOffScreen
bool BossProjectile::IsOffScreen() const
{
    // Acessar os componentes de forma segura
    if (auto drawComp = GetComponent<DrawAnimatedComponent>())
    {
        // 4. Acessar a Scene e o Game diretamente da classe Actor base
        const auto pos = GetPosition();
        const auto game = mScene->GetGame();
        const auto windowWidth = static_cast<float>(game->GetWindowWidth());
        const auto windowHeight = static_cast<float>(game->GetWindowHeight());
        const auto spriteWidth = static_cast<float>(drawComp->GetSpriteWidth());
        const auto spriteHeight = static_cast<float>(drawComp->GetSpriteHeight());

        // "Zona de buffer". Os dois numeros vem de DefinirMargemDeSaida e tem como
        // padrao a regra geral de sempre (1 sprite + um doze avos da tela).
        const float hBuffer = (mMargemDivisorDeTela > 0.0f) ? windowWidth / mMargemDivisorDeTela : 0.0f;
        const float vBuffer = (mMargemDivisorDeTela > 0.0f) ? windowHeight / mMargemDivisorDeTela : 0.0f;

        const float hFolga = spriteWidth * mMargemEmSprites + hBuffer;
        const float vFolga = spriteHeight * mMargemEmSprites + vBuffer;

        // Retorna 'true' se estiver FORA dos limites.
        return pos.x < -hFolga ||
               pos.x > windowWidth + hFolga ||
               pos.y < -vFolga ||
               pos.y > windowHeight + vFolga;
    }

    // Failsafe se não houver componente de desenho
    SDL_Log("Erro em BossProjectile.cpp - IsOffScreen: projetil nao tem DrawAnimatedComponent, retornando false");
    return false;
}

// 4. Implementação Getter
Boss* BossProjectile::GetBossOwner() const
{
    // dynamic_cast para converter o Actor* da classe base
    // para o Boss* que sabemos que ele é. Retorna nullptr se o cast falhar.
    return dynamic_cast<Boss*>(mOwner);
}

Vector2 BossProjectile::GetPlayerPosition() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->GetPlayer()) {
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
        if (auto player = battleScene->GetPlayer()) {
            if (auto rb = player->GetComponent<RigidBodyComponent>()) {
                return rb->GetVelocity();
            }
        }
    }
    // Sem log aqui: HasPlayer() ja permite ao chamador saber que nao ha
    // jogador, e este metodo e consultado por projetil na ativacao de caminho.
    // Logar seria spam.
    return Vector2::Zero;
}

bool BossProjectile::HasPlayer() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        return battleScene->GetPlayer() != nullptr;
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
    // Nao valida: quem chama e a fabrica, e o leitor de projeteis.json ja recusou
    // valores sem sentido antes de chegar aqui.
    mMargemEmSprites = emSprites;
    mMargemDivisorDeTela = divisorDeTela;
}
