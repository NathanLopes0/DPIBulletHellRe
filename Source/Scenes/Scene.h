//
// Created by nslop on 04/04/2024.
//

#pragma once

#include <SDL_stdinc.h>
#include <memory>
#include <SDL_rect.h>
#include <vector>

class Actor;
class Game;
class DrawComponent;
class ColliderComponent;

class Scene {
public:
    enum class SceneType
    {
        None,
        MainMenu,
        Identificacao,
        CriacaoDePersonagem,
        Opcoes,
        Ranking,
        Atracao,
        StageSelect,
        EscolhaDoOrientador,
        Battle
    };

    explicit Scene(Game* game, SceneType sceneType);
    virtual ~Scene() = default;


    void ProcessInput(const Uint8* keyState);
    void Update(float deltaTime);

    virtual void Load() = 0;

    [[nodiscard]] Game* GetGame() const { return mGame; }
    [[nodiscard]] SceneType GetType() const { return mType; }
    [[nodiscard]] const std::vector<DrawComponent*>& GetDrawables() const { return mDrawables; }

    Actor* AddActor(std::unique_ptr<Actor> actor);

    void AddCollider(ColliderComponent* collider);
    void RemoveCollider(ColliderComponent* collider);
    std::vector<ColliderComponent*>& GetColliders() { return mColliders; }

    void AddDrawable(DrawComponent* drawable);
    void RemoveDrawable(DrawComponent* drawable);

    [[nodiscard]] virtual SDL_FRect GetPlayfieldBounds() const;

protected:
    // As classes filhas vão dar override nestas para adicionar
    // sua lógica específica. Elas têm um corpo vazio por padrão.
    virtual void OnProcessInput(const Uint8* keyState);
    virtual void OnUpdate(float deltaTime);


    void RemoveDeadActors();

    /**
     * @brief A CENA CONGELADA: os atores nao recebem input nem quadro.
     *
     * E a pausa da batalha (Pausa.h). Os atores param do jeito que estao - o
     * chefe no meio do ataque, a maquina de estados no meio da fase, a
     * animacao no quadro em que estava -, e voltam exatamente dali, porque o
     * tempo deles e o deltaTime que deixou de chegar.
     *
     * NAO E O ActorState::Paused. Pausar cada ator mexeria no estado de quem
     * ja estava parado de proposito (o aluno durante o carregamento), e
     * despausar teria de lembrar quem era quem. Aqui a cena so deixa de
     * chamar, e o estado de cada um fica intocado.
     *
     * O OnUpdate e o OnProcessInput da cena continuam rodando: e por eles que
     * a propria cena desenha o menu e ouve quem quer sair dele.
     */
    bool mAtoresCongelados = false;

    Game* mGame;
    SceneType mType;

    // ATENCAO A ORDEM DE DECLARACAO ABAIXO.
    // Membros sao destruidos na ORDEM INVERSA da declaracao. ~DrawComponent
    // chama Scene::RemoveDrawable(this), que faz std::find em mDrawables.
    // Com mDrawables declarado DEPOIS de mActors, ele era destruido ANTES dos
    // atores, e cada componente destruido ia procurar a si mesmo dentro de um
    // vetor ja liberado - heap-use-after-free em TODA troca de cena
    // (confirmado com AddressSanitizer).
    // Declarando as listas de observadores primeiro, elas passam a morrer
    // depois dos atores, que e a ordem correta.

    // Lista de observadores para desenho.
    std::vector<DrawComponent*> mDrawables;

    //Lista de observadores para colisores
    std::vector<ColliderComponent*> mColliders;

    // A cena é DONA de todos os atores que vivem nela
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<std::unique_ptr<Actor>> mPendingActors;

    bool mIsUpdatingActors{};
};
