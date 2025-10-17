//
// Created by nslop on 16/09/2024.
//

#include "SDL.h"

#include "Battle.h"
#include "ProjectileManager.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teacher/Boss.h"
#include "../../Actors/Teacher/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/Teacher/BossFactory/IBossFactory.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"



Battle::Battle(Game* game, Game::GameSubject selectedStage)
    : Scene(game, SceneType::Battle)
    , mGrade(40.0f) // A matéria selecionada
    , mStage(selectedStage) // O GDD diz que a nota começa em 40
{

}

Battle::~Battle() = default;

void Battle::Load() {

    // 1. Criar os sistemas primeiro
    mProjectileManager = std::make_unique<ProjectileManager>(this);

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
    if (const IBossFactory *factory = mGame->GetFactory(static_cast<int>(mStage))) {

        // 2. A fábrica nos dá um ponteiro bruto para o Boss recém-criado. Se der certo, entra no if
        // obs: o nome dessa estrutura é if-statement, eu não conhecia antes.
        // Basicamente você faz uma atribuição num if e vê se ela deu certo, se deu entra no if
        if (auto bossOwner = factory->Create(*mGame)) {

            // 2.5 - transferimos a posse para o sistema Scene (seguro, tira warnings)
            Actor* actorPtr = this->AddActor(std::move(bossOwner));

            // 3. Guardar esse ponteiro no nosso observador.
            // Agora a batalha tem um "atalho" diretamente para o chefe
            mBoss = dynamic_cast<Boss*>(actorPtr);
        }

    } else {
        SDL_Log("Erro fatal: Nenhuma BossFactory encontrada para a matéria %s", mStage);
        // Talvez adicionar um this->QuitGame() ou algo do tipo aqui.
    }
}
void Battle::LoadHUD() {

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
}

void Battle::OnProcessInput(const Uint8* keyState) {
    if (mPlayer) {
        mPlayer->ProcessInput(keyState);
    }
}

void Battle::CheckCollisions() {
    if (!mPlayer || !mBoss) return;

    auto* playerCollider = mPlayer->GetComponent<CircleColliderComponent>();
    auto* bossCollider = mBoss->GetComponent<CircleColliderComponent>();

    if (!playerCollider || !bossCollider) return;

    const auto& bossProjectiles = mProjectileManager->GetBossProjectiles();
    const auto& playerProjectiles = mProjectileManager->GetPlayerProjectiles();


    // --- REGRA 1 - Projéteis do Boss vs Player --- //
    for (const auto& bossProj : bossProjectiles) {
        if (auto* bossProjCollider = bossProj->GetComponent<CircleColliderComponent>()) {
            if (playerCollider->Intersect(*bossProjCollider)) {

                mPlayer->OnCollision(bossProj.get());
                bossProj->OnCollision(mPlayer);

            }
        }
    }

    // --- REGRA 2 - Projéteis do Player vs Boss
    for (const auto& playerProj : playerProjectiles) {
        if (auto* playerProjCollider = playerProj->GetComponent<CircleColliderComponent>()) {
            if (bossCollider->Intersect(*playerProjCollider)) {

                mBoss->OnCollision(playerProj.get());
                playerProj->OnCollision(mBoss);

            }
        }
    }

    // --- REGRA 3 - Player vs Boss  (Contato Direto)
    if (playerCollider->Intersect(*bossCollider)) {
        mPlayer->OnCollision(mBoss);
        mBoss->OnCollision(mPlayer);
    }
}
