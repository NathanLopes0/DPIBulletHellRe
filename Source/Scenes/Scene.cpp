//
// Created by nslop on 04/04/2024.
//


#include <utility>
#include <algorithm>
#include "Scene.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawComponent.h"

Scene::Scene(Game* game, const SceneType sceneType) :
    mGame(game),
    mType(sceneType)
{

}

Actor* Scene::AddActor(std::unique_ptr<Actor> actor) {
    // A lógica de qual vetor usar continua a mesma.
    // A mudança está em como capturamos o ponteiro de retorno.

    if (mIsUpdatingActors) {
        // 1. Adicionamos o ator e guardamos a referência para o unique_ptr recém-criado DENTRO do vetor.
        auto& newActorPtr = mPendingActors.emplace_back(std::move(actor));
        return newActorPtr.get();
    } else {
        auto& newActorPtr = mActors.emplace_back(std::move(actor));
        return newActorPtr.get();
    }
}

void Scene::ProcessInput(const Uint8* keyState) {
    // Delega o input para todos os atores da cena - menos com ela congelada,
    // em que o aluno nao atira nem anda (Scene.h).
    if (!mAtoresCongelados) {
        for (const auto& actor : mActors) {
            if (actor->GetState() == ActorState::Active) {
                actor->ProcessInput(keyState);
            }
        }
    }

    OnProcessInput(keyState);
}

void Scene::Update(float deltaTime) {

    // Atualiza todos os atores da cena - menos com ela congelada (Scene.h).
    mIsUpdatingActors = true;
    if (!mAtoresCongelados) {
        for (const auto& actor : mActors) {
            actor->Update(deltaTime);
        }
    }
    mIsUpdatingActors = false;


    for(auto& pending : mPendingActors) {
        mActors.emplace_back(std::move(pending));
    }
    mPendingActors.clear();


    // A logica especifica da cena roda ANTES da remocao, com todos os atores
    // ainda vivos. Era o contrario: RemoveDeadActors() destruia os atores
    // marcados como Destroy e so depois OnUpdate() rodava - entao qualquer cena
    // que guardasse ponteiros observadores (Battle::mExtraPoints) lia memoria
    // ja liberada. Isso tambem da a cada cena a chance de limpar suas proprias
    // listas de ponteiros dentro do OnUpdate, antes dos objetos morrerem.
    OnUpdate(deltaTime);

    // Remove os atores mortos - ultimo passo do frame.
    RemoveDeadActors();
}

void Scene::RemoveDeadActors() {
    auto it = std::remove_if(mActors.begin(), mActors.end(),
                             [](const std::unique_ptr<Actor>& actor) {
                                 return actor->GetState() == ActorState::Destroy;
                             });
    mActors.erase(it, mActors.end());
}

void Scene::AddDrawable(DrawComponent* drawable)
{
    mDrawables.emplace_back(drawable);

    // ESTAVEL, E NAO std::sort. Entre componentes de MESMA ordem de desenho, o
    // sort comum escolhe uma permutacao qualquer - e escolhe outra a cada vez
    // que a lista muda de tamanho. O estavel mantem a ordem de ENTRADA, que e
    // uma regra que da para explicar: entre iguais, quem chegou antes fica por
    // baixo.
    //
    // ISTO NAO E ARRUMACAO: era um defeito, e ele aparecia. Dois projeteis do
    // chefe que se cruzam tem a mesma ordem, e qual fica por cima decidia o
    // desenho. Acrescentar TRES componentes na faixa da nota - que desenha
    // depois dos projeteis e nao encosta neles - trocou de lugar dois cartoes
    // de uma rajada a 600 px dali. Medido: a mesma batalha, quadro a quadro,
    // com e sem as tres placas, dava 591 pixels diferentes fora da faixa; com
    // o estavel, da zero.
    std::stable_sort(mDrawables.begin(), mDrawables.end(), [](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Scene::RemoveDrawable(DrawComponent* drawable)
{
    if (auto iter = std::find(mDrawables.begin(),
                                            mDrawables.end(), drawable);
                                            iter != mDrawables.end()) {
        mDrawables.erase(iter);
    }
}


void Scene::OnProcessInput(const Uint8* keyState) {}
void Scene::OnUpdate(float deltaTime) {}

SDL_FRect Scene::GetPlayfieldBounds() const
{
    // Por padrão, a área é a tela inteira
    return {
        0.0f,
        0.0f,
        static_cast<float>(mGame->GetWindowWidth()),
        static_cast<float>(mGame->GetWindowHeight())
    };
}

