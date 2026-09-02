//
// Created by nslop on 27/08/2024.
//


#include <fstream>
#include "../../Game.h"
#include "../../Json.h"
#include "../../Actors/Actor.h"
#include "DrawAnimatedComponent.h"

DrawAnimatedComponent::DrawAnimatedComponent(class Actor* owner, const std::string &spriteSheetPath, const std::string &spriteSheetData, int drawOrder)
        :DrawSpriteComponent(owner, spriteSheetPath, 0, 0, drawOrder)
{
    LoadSpriteSheet(spriteSheetPath, spriteSheetData);

    // O JSON era aberto e parseado uma SEGUNDA vez aqui so para ler
    // largura/altura do primeiro frame. LoadSpriteSheet ja leu isso para dentro
    // de mSpriteSheetData, entao basta consultar (mesma premissa de antes: todos
    // os frames tem o mesmo tamanho). Isso corta metade do custo de JSON de cada
    // projetil criado, o que importa muito durante o Prewarm.
    if (!mSpriteSheetData.empty()) {
        mWidth = mSpriteSheetData[0].w;
        mHeight = mSpriteSheetData[0].h;
    }
}

DrawAnimatedComponent::~DrawAnimatedComponent()
{
    mSpriteSheetData.clear();
}

void DrawAnimatedComponent::LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath)
{
    // Load sprite sheet texture
    mSpriteSheetSurface = mOwner->GetScene()->GetGame()->LoadTexture(texturePath);

    // Load sprite sheet data
    std::ifstream spriteSheetFile(dataPath);
    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile);


    for(const auto& frame : spriteSheetData["frames"]) {

        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();

        mSpriteSheetData.push_back({x, y, w, h});
    }

}

void DrawAnimatedComponent::Draw(SDL_Renderer *renderer) {

    if (!mIsVisible || !mSpriteSheetSurface) return;

    // Busca SEM operator[]. Em um unordered_map, mAnimations[mAnimName] INSERE
    // um vetor vazio quando a chave nao existe. Com mAnimName == "" (nenhum
    // SetAnimation chamado), o codigo antigo indexava [0] de um vetor vazio,
    // cujo data() e nullptr: dereferencia de ponteiro nulo a cada frame.
    const auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end() || animIt->second.empty()) {
        return;
    }

    const auto& frames = animIt->second;

    // mAnimTimer e float e pode passar do tamanho da animacao num deltaTime
    // grande, entao o indice do frame e limitado ao intervalo valido.
    auto frameIdx = static_cast<size_t>(mAnimTimer);
    if (frameIdx >= frames.size()) frameIdx = frames.size() - 1;

    const int spriteIdx = frames[frameIdx];

    // Comparacao com cast explicito: antes era 'int < size_t', e um indice
    // negativo virava um numero gigante ao ser convertido para unsigned,
    // passando pela checagem.
    if (spriteIdx >= 0 && static_cast<size_t>(spriteIdx) < mSpriteSheetData.size())
    {
        Vector2 pos = mOwner->GetPosition();
        Vector2 cameraPos = mOwner->GetScene()->GetGame()->GetCameraPos();

        float scale = mOwner->GetScale();

        const SDL_Rect& clipRect = mSpriteSheetData[spriteIdx];

        int scaledWidth = static_cast<int>(clipRect.w * scale);
        int scaledHeight = static_cast<int>(clipRect.h * scale);

        SDL_Rect renderQuad = {
            static_cast<int>(pos.x - scaledWidth/2.0f - cameraPos.x),
            static_cast<int>(pos.y - scaledHeight/2.0f - cameraPos.y),
            scaledWidth,  // Largura Final
            scaledHeight  // Altura Final
        };

        SDL_RendererFlip flip = SDL_FLIP_NONE;

        ApplyColorMod();

        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &clipRect, &renderQuad, mOwner->GetRotation(), nullptr, flip);
    }
}


void DrawAnimatedComponent::Update(float deltaTime)
{
    if(mIsPaused) return;

    // Mesma protecao do Draw. Sem ela, uma animacao inexistente ou vazia fazia
    // o while comparar (int)mAnimTimer >= 0 (sempre verdadeiro) e subtrair 0
    // indefinidamente: o jogo inteiro travava em loop infinito.
    const auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end() || animIt->second.empty()) {
        return;
    }

    const auto frameCount = static_cast<float>(animIt->second.size());

    mAnimTimer += mAnimFPS * deltaTime;

    // frameCount e sempre >= 1 aqui, entao o loop termina.
    while (mAnimTimer >= frameCount) {
        mAnimTimer -= frameCount;
    }
    if (mAnimTimer < 0.0f) mAnimTimer = 0.0f;
}

void DrawAnimatedComponent::SetAnimation(const std::string& name)
{
    // Falha alto em vez de deixar o componente num estado invalido silencioso.
    if (mAnimations.find(name) == mAnimations.end()) {
        SDL_Log("DrawAnimatedComponent::SetAnimation: animacao '%s' nao registrada "
                "neste componente. Mantendo a anterior ('%s').",
                name.c_str(), mAnimName.c_str());
        return;
    }

    // Player::HandleAnimation chama isto TODO frame com o mesmo nome. Sem esta
    // guarda, o reset do timer abaixo congelaria a animacao no primeiro frame.
    if (mAnimName == name) return;

    mAnimName = name;
    mAnimTimer = 0.0f; // troca de animacao comeca do frame inicial
}

void DrawAnimatedComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    mAnimations.emplace(name, spriteNums);
}