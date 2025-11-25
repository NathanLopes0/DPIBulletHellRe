//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"

#include "Battle.h"

#include <locale>
#include <sstream>
#include <iomanip>

#include "BattleHUD.h"
#include "ProjectileManager.h"
#include "../../Json.h"
#include "../../Font.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"
#include "../../Actors/Teachers/ExtraPointItem.h"



Battle::Battle(Game* game, Game::GameSubject selectedStage)
    : Scene(game, SceneType::Battle)
    , mGrade(40.0f)
    , mStage(selectedStage)
{

}

Battle::~Battle() = default;

//region LoadFunctions
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
        SDL_Log("Erro fatal: Nenhuma BossFactory encontrada para a matéria %s, voltando pra StageSelect", mStage);
        mGame->RequestSceneChange(SceneType::StageSelect);
    }
}
void Battle::LoadHUD() {

    //Cria a HUD para segurar todas as infos
    mHUD = std::make_unique<BattleHUD>(this);
    LoadTimeBar();
    LoadGradeBar();

}
void Battle::LoadPlayer() {

    auto player = std::make_unique<Player>(this);


    // 2. Configura a posição inicial
    const auto midWidth = static_cast<float>(mGame->GetWindowWidth()) / 2.0f;
    const auto startingHeight = static_cast<float>(mGame->GetWindowHeight()) * 7.0f / 8.0f;
    player->SetPosition(Vector2(midWidth, startingHeight));

    mPlayer = dynamic_cast<Player*>(this->AddActor(std::move(player)));

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
    barCompPtr->SetColors({40, 40, 40, 255}, {255, 191, 0, 255});

    Actor* barActorPtr = barActor.get();

    // Adiciona ator à cena
    this->AddActor(std::move(barActor));

    // Dá poder ao HUD pra modificar a barra
    mHUD->SetTimeBar(barActorPtr, barCompPtr);

    // Salva o ponteiro do Actor que tem a barra de progresso na cena Battle, pra poder seguir o Boss
    mHUDTimeBarActor = barActorPtr;

    // Dá load no reloginho que tem embaixo da barra
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
void Battle::LoadGradeBar() {
    auto gradeBarActor = std::make_unique<Actor>(this);

    constexpr int barHeight = 25;
    const int windowWidth = mGame->GetWindowWidth();
    const int windowHeight = mGame->GetWindowHeight();
    gradeBarActor->SetPosition(Vector2(0.f, static_cast<float>(windowHeight) - barHeight * 1.5));

    auto gradeBarComp = gradeBarActor->AddComponent<ProgressBarComponent>();
    gradeBarComp->SetOrientation(ProgressBarComponent::Orientation::Horizontal);
    gradeBarComp->SetSize(windowWidth, barHeight);

    mGradeBarActor = gradeBarActor.get();
    this->AddActor(std::move(gradeBarActor));

    // Configurar Actor e fonte que vai ter a nota no meio da barra
    mGradeBarFont = std::make_unique<Font>();
    mGradeBarFont->Load("../Assets/Fonts/Zelda.ttf");
    auto gradeBarTextActor = std::make_unique<Actor>(this);
    gradeBarTextActor->AddComponent<DrawTextComponent>("omg", mGradeBarFont.get(), 36, barHeight, 24, 304);

    const auto textPosX = static_cast<float>(windowWidth) / 2.f;
    const auto textPosY = static_cast<float>(windowHeight - barHeight);
    gradeBarTextActor->SetPosition(Vector2(textPosX, textPosY));

    mGradeTextActor = gradeBarTextActor.get();
    this->AddActor(std::move(gradeBarTextActor));

}

//endregion LoadFunctions
//region UpdateFunctions
void Battle::OnUpdate(float deltaTime) {

    // A cena base já atualiza todos os atores.
    // Aqui podemos adicionar lógica específica da batalha.

    // Atualiza os sistemas principais apenas da Battle (os atores são atualizados pela Scene)
    if (mProjectileManager) mProjectileManager->Update(deltaTime);

    // Isso NÃO destroi o objeto do ponto extra, apenas o remove da lista de pontos extras da batalha.
    mExtraPoints.erase(std::remove_if(mExtraPoints.begin(), mExtraPoints.end(),
        [](const ExtraPointItem* item) {
            return item->GetState() == ActorState::Destroy;
        }), mExtraPoints.end());

    // Orquestra a checagem de colisões
    CheckCollisions();

    if (mHUD) mHUD->OnUpdate(deltaTime);

    TimeBarUpdate();
    GradeBarUpdate();

    // Se a nota cair a 0, termina a fase
    if (mGrade <= 0) {
        mGame->RequestSceneChange(SceneType::MainMenu);
    }

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
void Battle::GradeBarUpdate() {
    auto gradeComp = mGradeBarActor->GetComponent<ProgressBarComponent>();
    if (!gradeComp) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GradeBarActor not found");
        return;
    }

    const float fillPercent = mGrade / 100.f;
    gradeComp->SetFillPercent(fillPercent);

    // -- DEFINIÇÃO DA PALETA DE CORES -- //

    static const SDL_Color BG_COLOR = {20,20,20,255};

    // STATE1 - REPROVADE
    static const SDL_Color FAIL_START = {100, 0, 0, 255};
    static const SDL_Color FAIL_END = {255, 0, 0, 255};

    // STATE2 - FINAL
    static const SDL_Color SECRET_START = {180, 150, 0, 255};
    static const SDL_Color SECRET_END = {255, 140, 0, 255};

    // STATE3 - APROVADE
    static const SDL_Color PASS_START = {144, 238, 144, 255};
    static const SDL_Color PASS_END = {0, 255, 128, 255};

    // -- --------- -- ------ -- ----- -- //

    float colorPercent = 0.0f;

    if (mGrade < 40.f) {
        colorPercent = mGrade / 40.f;
        gradeComp->SetGradientColors(BG_COLOR, FAIL_START, FAIL_END);
        gradeComp->SetEffect(ProgressBarComponent::VisualEffect::Pulsing);
    } else if (mGrade < 60.f) {
        colorPercent = (mGrade - 40.f) / 20.f;
        gradeComp->SetGradientColors(BG_COLOR, SECRET_START, SECRET_END);
        gradeComp->SetEffect(ProgressBarComponent::VisualEffect::None);
    } else {
        colorPercent = (mGrade - 60.f) / 40.f;
        gradeComp->SetGradientColors(BG_COLOR, PASS_START, PASS_END);
        gradeComp->SetEffect(ProgressBarComponent::VisualEffect::None);
    }

    gradeComp->SetColorPercent(colorPercent);
    GradeTextUpdate();

}
void Battle::GradeTextUpdate() {

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << mGrade;
    mGradeTextActor->GetComponent<DrawTextComponent>()->SetText(ss.str());

}
//endregion UpdateFunctions


void Battle::OnProcessInput(const Uint8* keyState) {
    if (mPlayer) {
        mPlayer->ProcessInput(keyState);
    }
}
void Battle::CheckCollisions() {
    if (!mPlayer || !mBoss) return;
    bool changedGrade = false;

    auto *playerCollider = mPlayer->GetComponent<CircleColliderComponent>();
    auto *bossCollider = mBoss->GetComponent<CircleColliderComponent>();

    if (!playerCollider || !bossCollider) return;

    const auto &bossProjectiles = mProjectileManager->GetBossProjectiles();
    const auto &playerProjectiles = mProjectileManager->GetPlayerProjectiles();


    // --- REGRA 1 - Projéteis do Boss vs Player --- //
    for (const auto &bossProj: bossProjectiles) {
        if (bossProj->GetState() != ActorState::Active) continue;
        if (auto *bossProjCollider = bossProj->GetComponent<CircleColliderComponent>()) {
            if (playerCollider->Intersect(*bossProjCollider) && !mPlayer->GetIsInvincible()) {
                mPlayer->SetIsInvincible(true);
                mPlayer->OnCollision(bossProj.get());
                bossProj->OnCollision(mPlayer);
                GradeDown();
                changedGrade = true;
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
                changedGrade = true;
            }
        }
    }

    // --- REGRA 3 - Player vs Boss  (Contato Direto)
    if (playerCollider->Intersect(*bossCollider) && !mPlayer->GetIsInvincible()) {
        mPlayer->SetIsInvincible(true);
        mPlayer->OnCollision(mBoss);
        mBoss->OnCollision(mPlayer);
        GradeDown();
        changedGrade = true;
    }

    // --- REGRA 4 - Player vs ExtraPoints
    for (auto item : mExtraPoints) {
        // Se o item já morreu neste frame, pula
        if (item->GetState() != ActorState::Active) continue;

        auto itemCollider = item->GetComponent<CircleColliderComponent>();
        if (itemCollider && playerCollider->Intersect(*itemCollider)) {

            // 1. Efeito da Coleta
            mPlayer->AddExtraPoint();
            // TODO - Colocar um efeito sonoro aqui

            // 2. Destroi o item
            item->SetState(ActorState::Destroy);
        }
    }

    if (changedGrade) {
        mGame->SetGrade(mStage, mGrade);
    }

}

void Battle::GradeUp(const float amount) {
    mGrade = Math::Clamp(mGrade + amount, 0.0f, 100.f);
}
void Battle::GradeDown() {
    mGrade -= GRADE_CHANGE_DOWN;
    mGrade = Math::Clamp(mGrade, 0.0f, 100.f);
}

void Battle::ResetHUDTimer(const float newDuration) {
    if (mHUD) {
        mHUD->ResetTimeBar(newDuration);
    }
}

SDL_FRect Battle::GetPlayfieldBounds() const
{
    // A altura da sua barra de nota, como definido em LoadGradeBar
    constexpr float gradeBarHeight = 25.0f;

    return {
        0.0f, // x
        0.0f, // y
        static_cast<float>(mGame->GetWindowWidth()), // w
        static_cast<float>(mGame->GetWindowHeight()) - gradeBarHeight // h
    };
}

void Battle::OnPlayerUsedExtraPoint() {
    if (mProjectileManager) {
        mProjectileManager->ClearBossProjectiles();
    }
    GradeUp(10.f);
    // TODO - Audio - efeito sonoro de limpeza de campo
}

void Battle::SpawnExtraPoint(Vector2 position) {
    auto item = std::make_unique<ExtraPointItem>(this);
    item->SetPosition(position);

    mExtraPoints.emplace_back(item.get());

    AddActor(std::move(item));
}
