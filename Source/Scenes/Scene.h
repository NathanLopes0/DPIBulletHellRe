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
        EscolhaDoComeco,
        Opcoes,
        Ranking,
        Boletim,
        Horas,
        ComoJogar,
        Atracao,
        EscolhaDaDificuldade,
        StageSelect,
        TrocaDeAlunos,
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

    void AddDrawable(DrawComponent* drawable);
    void RemoveDrawable(DrawComponent* drawable);

    [[nodiscard]] virtual SDL_FRect GetPlayfieldBounds() const;

    /// A janela do jogo perdeu o foco (alguem foi para outra janela). Por padrao nada acontece; a
    /// batalha pausa (Battle::AoPerderOFoco).
    virtual void AoPerderOFoco() {}

    /// Se a cena esta parada num menu de pausa. No gabinete, uma batalha pausada pode voltar ao
    /// menu sozinha, como as outras telas (Game::AtualizarGabinete).
    [[nodiscard]] virtual bool Pausada() const { return false; }

    /// A ociosidade do gabinete vai tirar o jogo desta cena: o que ela precisa fechar antes.
    virtual void AoSairPorOciosidade() {}

protected:
    // As classes filhas vão dar override nestas para adicionar
    // sua lógica específica. Elas têm um corpo vazio por padrão.
    virtual void OnProcessInput(const Uint8* keyState);
    virtual void OnUpdate(float deltaTime);

    void RemoveDeadActors();

    /// A cena congelada: os atores nao recebem input nem quadro. E a pausa da batalha (Pausa.h).
    bool mAtoresCongelados = false;

    Game* mGame;
    SceneType mType;

    // Atencao a ordem de declaracao abaixo.

    // Lista de observadores para desenho.
    std::vector<DrawComponent*> mDrawables;

    // A cena é dona de todos os atores que vivem nela.
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<std::unique_ptr<Actor>> mPendingActors;

    bool mIsUpdatingActors{};
};
