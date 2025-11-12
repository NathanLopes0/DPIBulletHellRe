//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"

#include "Battle.h"

#include "BattleHUD.h"
#include "ProjectileManager.h"
#include "../../Json.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teacher/Boss.h"
#include "../../Actors/Teacher/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/Teacher/BossFactory/IBossFactory.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"

#define GRADE_CHANGE_UP 0.56f
#define GRADE_CHANGE_DOWN 2.f



Battle::Battle(Game* game, Game::GameSubject selectedStage)
    : Scene(game, SceneType::Battle)
    , mGrade(40.0f)
    , mStage(selectedStage)
{

}

Battle::~Battle() = default;

void Battle::Load() {

    // 1. Criar os sistemas primeiro
    mProjectileManager = std::make_unique<ProjectileManager>(this);

    mGrade = mGame->GetGrade(mStage);

    // 2. Criar os atores principais
    LoadPlayer();
    LoadBoss();
    LoadHUD();

    // 3. Iniciar a lógica da batalha (se necessário)
    if (mBoss) {
        SDL_Log("Starting Boss");
        mBoss->Start();
    }

}


void Battle::LoadBoss() {


    // 1. Pega a fábrica correta do Game (como uma ferramenta temporária), e se ela existir, continua
    if (IBossFactory *factory = mGame->GetFactory(mStage)) {
        auto boss = factory->CreateBoss(this);
        boss->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth())/2.0f,
                                        static_cast<float>(mGame->GetWindowHeight())/6.0f));

        mBoss = dynamic_cast<Boss*>(this->AddActor(std::move(boss)));


    } else {
        SDL_Log("Erro fatal: Nenhuma BossFactory encontrada para a matéria %s", mStage);
        // Talvez adicionar um this->QuitGame() ou algo do tipo aqui.
    }
}

void Battle::LoadHUD() {

    //Cria a HUD para segurar todas as infos
    mHUD = std::make_unique<BattleHUD>(this);
    LoadTimeBar();

}
void Battle::LoadPlayer() {

    auto player = std::make_unique<Player>(this);


    // 2. Configura a posição inicial
    const auto midWidth = static_cast<float>(mGame->GetWindowWidth()) / 2.0f;
    const auto startingHeight = static_cast<float>(mGame->GetWindowHeight()) * 7.0f / 8.0f;
    player->SetPosition(Vector2(midWidth, startingHeight));

    mPlayer = dynamic_cast<Player*>(this->AddActor(std::move(player)));

}

void Battle::OnUpdate(float deltaTime) {

    // A cena base já atualiza todos os atores.
    // Aqui podemos adicionar lógica específica da batalha.

    // Atualiza os sistemas principais apenas da Battle (os atores são atualizados pela Scene)
    if (mProjectileManager) mProjectileManager->Update(deltaTime);

    // Orquestra a checagem de colisões
    CheckCollisions();

    if (mHUD) mHUD->OnUpdate(deltaTime);

    TimeBarUpdate();

    // SDL_Log("%f", mGrade);

}

void Battle::OnProcessInput(const Uint8* keyState) {
    if (mPlayer) {
        mPlayer->ProcessInput(keyState);
    }
}

void Battle::CheckCollisions() {
    if (!mPlayer || !mBoss) return;

    auto *playerCollider = mPlayer->GetComponent<CircleColliderComponent>();
    auto *bossCollider = mBoss->GetComponent<CircleColliderComponent>();

    if (!playerCollider || !bossCollider) return;

    const auto &bossProjectiles = mProjectileManager->GetBossProjectiles();
    const auto &playerProjectiles = mProjectileManager->GetPlayerProjectiles();


    // --- REGRA 1 - Projéteis do Boss vs Player --- //
    for (const auto &bossProj: bossProjectiles) {
        if (bossProj->GetState() != ActorState::Active) continue;
        if (auto *bossProjCollider = bossProj->GetComponent<CircleColliderComponent>()) {
            if (playerCollider->Intersect(*bossProjCollider)) {
                mPlayer->OnCollision(bossProj.get());
                bossProj->OnCollision(mPlayer);
                GradeDown();
            }
        }
    }

    // --- REGRA 2 - Projéteis do Player vs Boss
    for (const auto &playerProj: playerProjectiles) {
        if (playerProj->GetState() != ActorState::Active) continue;
        if (auto *playerProjCollider = playerProj->GetComponent<CircleColliderComponent>()) {
            if (bossCollider->Intersect(*playerProjCollider)) {
                mBoss->OnCollision(playerProj.get());
                playerProj->OnCollision(mBoss);
                GradeUp();
            }
        }
    }

    // --- REGRA 3 - Player vs Boss  (Contato Direto)
    if (playerCollider->Intersect(*bossCollider)) {
        mPlayer->OnCollision(mBoss);
        mBoss->OnCollision(mPlayer);
        GradeDown();
    }
}

void Battle::GradeUp() {
    mGrade += GRADE_CHANGE_UP;
    mGrade = Math::Clamp(mGrade, 0.0f, 100.f);
}

void Battle::GradeDown() {
    mGrade -= GRADE_CHANGE_DOWN;
    mGrade = Math::Clamp(mGrade, 0.0f, 100.f);
}


void Battle::TimeBarUpdate() const {
    if (!mBoss || !mHUDTimeBarActor) return;

    Vector2 bossPos = mBoss->GetPosition();

    auto bossSprite = mBoss->GetComponent<DrawAnimatedComponent>();
    auto bossWidth = bossSprite->GetSpriteWidth();
    auto bossHeight = bossSprite->GetSpriteHeight();

    float offSetX = static_cast<float>(bossWidth) / 2.5f;
    float offSetY = static_cast<float>(bossHeight) / 4.f;

    mHUDTimeBarActor->SetPosition(Vector2(bossPos.x + offSetX,
                                    bossPos.y - offSetY));

    mHUDClockActor->SetPosition(mHUDTimeBarActor->GetPosition() + Vector2(5, static_cast<float>(bossHeight) / 1.4f));
}


void Battle::ResetHUDTimer(float newDuration) {
    if (mHUD) {
        mHUD->ResetTimeBar(newDuration);
    }
}

void Battle::LoadTimeBar() {

    // Cria o Actor que vai segurar a barra que mostra o tempo do estado
    auto barActor = std::make_unique<Actor>(this);

    if (!mBoss) {
        SDL_Log("ERRO: LoadHUD chamado antes do mBoss ser criado!");
        return;
    }

    auto bossSprite = mBoss->GetComponent<DrawAnimatedComponent>();
    auto bossHeight = bossSprite->GetSpriteHeight();

    auto barCompPtr = barActor->AddComponent<ProgressBarComponent>();

    // Configura a barra que acabamos de colocar no boss
    barCompPtr->SetOrientation(ProgressBarComponent::Orientation::Vertical);
    barCompPtr->SetSize(10, static_cast<int>(static_cast<float>(bossHeight) / 1.4f));
    barCompPtr->SetColors({20,20,20,255}, {255, 100, 150, 255});

    Actor* barActorPtr = barActor.get();

    // Adiciona ator à cena
    this->AddActor(std::move(barActor));

    // Dá poder ao HUD pra modificar a barra
    mHUD->SetTimeBar(barActorPtr, barCompPtr);

    // Salva o ponteiro do Actor que tem a barra de progresso na cena Battle, pra poder seguir o Boss
    mHUDTimeBarActor = barActorPtr;

    // TODO - desenhei um relógio legal pra ficar logo embaixo dessa barra.
    // TODO - ele vai ser um Actor com um DrawAnimatedComponent que vai seguir a barra
    // TODO - (ou o Boss, deve ser mais fácil), estando sempre a uma distância fixa dele, como a barra mesmo.

    LoadTimeClock();
}

void Battle::LoadTimeClock() {

    // Cria o ator que vai segurar a animação do relógior
    auto clockActor = std::make_unique<Actor>(this);
    auto clockActorPtr = clockActor.get();

    std::string clockSpriteSheet = "../Assets/Icons/TestTimeIcon.png";
    std::string clockSpriteData = "../Assets/Icons/TestTimeIcon.json";

    auto clockComp = clockActor->AddComponent<DrawAnimatedComponent>(clockSpriteSheet, clockSpriteData, 201);

    clockComp->AddAnimation("Idle", {0});
    clockComp->AddAnimation("Active", {0,1,2,3,4,5});
    clockComp->SetAnimation("Active");

    this->AddActor(std::move(clockActor));
    mHUDClockActor = clockActorPtr;

}



