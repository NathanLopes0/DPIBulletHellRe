//
// Created by nslop on 03/04/2024.
//
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <SDL_ttf.h>
#include "SDL_image.h"
#include "Random.h"
#include "Game.h"
#include "AudioSystem.h"

Game::Game(int windowWidth, int windowHeight)
    :mWindow(nullptr),
    mRenderer(nullptr),
    mTicksCount(0),
    mIsRunning(true),
    mUpdatingActors(false),
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight),
    mChangeScene(false),
    mGameState(GameScene::MainMenu)
{

}

bool Game::Initialize() {

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    //criando janela do jogo
    mWindow = SDL_CreateWindow("DPI Bullet Hell", 0, 0, mWindowWidth, mWindowHeight, 0);
    if(!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    //posicionando no centro da tela
    SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    //criando render
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    //iniciando sistema de fontes
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    mAudio = new AudioSystem();

    //new random seed
    Random::Init();

    //start counter for deltaTime
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    InitializeActors();

    return true;
}

//função que seleciona a cena e chama a função Load.
void Game::InitializeActors()
{
    switch (mGameState) {
        case GameScene::MainMenu : {
            mScene = new MainMenu(this);
            mScene->Load();
            break;
        }
        case GameScene::StageSelect : {
            mScene = new StageSelect(this);
            mScene->Load();
            break;
        }
        case GameScene::Battle : {
            mScene = new Battle(this);
            mScene->Battle();
            break;
        }

    }
}