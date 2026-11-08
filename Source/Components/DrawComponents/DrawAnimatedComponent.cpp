//
// Created by nslop on 27/08/2024.
//

#include "../../Motor/Game.h"
#include "../../Actors/Actor.h"
#include "../../Regras/Destrutivel.h"
#include "DrawAnimatedComponent.h"

DrawAnimatedComponent::DrawAnimatedComponent(class Actor* owner, const std::string &spriteSheetPath, const std::string &spriteSheetData, int drawOrder)
        :DrawSpriteComponent(owner, spriteSheetPath, 0, 0, drawOrder)
{
    LoadSpriteSheet(spriteSheetPath, spriteSheetData);

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
    // Load sprite sheet texture.
    mSpriteSheetSurface = mOwner->GetScene()->GetGame()->LoadTexture(texturePath);
    mCaminhoDaFolha = texturePath;
    mSilhueta = nullptr;
    mSilhuetaTentada = false;

    // Esvazia antes de ler.
    mSpriteSheetData.clear();
    mUsingFallbackSheet = false;

    // Os quadros vem do cache do game (Game::QuadrosDaFolha).
    mSpriteSheetData = mOwner->GetScene()->GetGame()->QuadrosDaFolha(dataPath);

    if (mSpriteSheetData.empty()) {
        // As dimensoes vem da propria textura, entao isso funciona nos dois casos sem numero magico.
        int w = 0, h = 0;
        if (mSpriteSheetSurface) {
            SDL_QueryTexture(mSpriteSheetSurface, nullptr, nullptr, &w, &h);
        }
        if (w <= 0 || h <= 0) { w = 64; h = 64; }

        mSpriteSheetData.push_back({0, 0, w, h});

        // Sinaliza ao Draw que os indices das animacoes registradas nao valem mais nada.
        mUsingFallbackSheet = true;
    }
}

void DrawAnimatedComponent::Draw(SDL_Renderer *renderer) {

    if (!mIsVisible || !mSpriteSheetSurface) return;

    // Busca sem operator[].
    const auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end() || animIt->second.empty()) {
        return;
    }

    const auto& frames = animIt->second;

    auto frameIdx = static_cast<size_t>(mAnimTimer);
    if (frameIdx >= frames.size()) frameIdx = frames.size() - 1;

    // Em modo fallback ha um unico frame.
    const int spriteIdx = mUsingFallbackSheet ? 0 : frames[frameIdx];

    if (spriteIdx >= 0 && static_cast<size_t>(spriteIdx) < mSpriteSheetData.size())
    {
        Vector2 pos = mOwner->GetPosition();

        float scale = mOwner->GetScale();

        const SDL_Rect& clipRect = mSpriteSheetData[spriteIdx];

        int scaledWidth = static_cast<int>(clipRect.w * scale);
        int scaledHeight = static_cast<int>(clipRect.h * scale);

        SDL_Rect renderQuad = {
            static_cast<int>(pos.x - scaledWidth/2.0f),
            static_cast<int>(pos.y - scaledHeight/2.0f),
            scaledWidth,  // Largura Final
            scaledHeight  // Altura Final
        };

        SDL_RendererFlip flip = SDL_FLIP_NONE;

        if (mContorno) {
            DesenharContorno(renderer, clipRect, renderQuad, -Math::ToDegrees(mOwner->GetRotation()), flip);
        }

        ApplyColorMod();

        // MRotation e guardado em radianos; SDL_RenderCopyEx espera graus.
        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &clipRect, &renderQuad,
                         -Math::ToDegrees(mOwner->GetRotation()), nullptr, flip);
    }
}

void DrawAnimatedComponent::DefinirContorno(const bool ligado, const SDL_Color cor) {
    mContorno = ligado;
    mCorDoContorno = cor;
}

void DrawAnimatedComponent::DesenharContorno(SDL_Renderer* renderer, const SDL_Rect& quadro,
                                             const SDL_Rect& destino, const double angulo,
                                             const SDL_RendererFlip flip) {
    // A silhueta so e carregada quando o contorno liga pela primeira vez.
    if (!mSilhuetaTentada) {
        mSilhuetaTentada = true;
        mSilhueta = mOwner->GetScene()->GetGame()->LoadSilhueta(mCaminhoDaFolha);
    }
    if (!mSilhueta) return;

    SDL_SetTextureColorMod(mSilhueta, mCorDoContorno.r, mCorDoContorno.g, mCorDoContorno.b);
    SDL_SetTextureAlphaMod(mSilhueta, mCorDoContorno.a);
    const int e = Destrutivel::kEspessura;
    for (int dy = -e; dy <= e; dy += e) {
        for (int dx = -e; dx <= e; dx += e) {
            if (dx == 0 && dy == 0) continue;
            SDL_Rect deslocado = destino;
            deslocado.x += dx;
            deslocado.y += dy;
            SDL_RenderCopyEx(renderer, mSilhueta, &quadro, &deslocado, angulo, nullptr, flip);
        }
    }
}

void DrawAnimatedComponent::Update(float deltaTime)
{
    if(mIsPaused) return;

    // Mesma protecao do Draw.
    const auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end() || animIt->second.empty()) {
        return;
    }

    const auto frameCount = static_cast<float>(animIt->second.size());

    mAnimTimer += mAnimFPS * deltaTime;

    // FrameCount e sempre >= 1 aqui, entao o loop termina.
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

    // Player::HandleAnimation chama isto todo frame com o mesmo nome.
    if (mAnimName == name) return;

    mAnimName = name;
    mAnimTimer = 0.0f; // troca de animacao comeca do frame inicial
}

void DrawAnimatedComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    mAnimations.emplace(name, spriteNums);
}