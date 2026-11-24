//
// Created by nslop on 27/08/2024.
//

#include <algorithm>

#include "../../Motor/Game.h"
#include "../../Interface/Hud.h"
#include "../../Motor/Daltonizar.h"
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

    // O rastro vem antes, por baixo, e nao pisca com o desenho (o aluno protegido pisca).
    if (!mEcos.empty() && mSpriteSheetSurface) DesenharEcos(renderer);

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

        const float scale = mOwner->GetScale() * mEscalaPropria;

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

        const double angulo = -Math::ToDegrees(mOwner->GetRotation());
        if (mBrilho && mAlcanceDoBrilho > Destrutivel::kEspessura) {
            // Dois aneis, o de fora mais apagado: o brilho some com a distancia, e o contorno vem
            // por cima dele.
            SDL_Color fora = mCorDoBrilho;
            fora.a = static_cast<Uint8>(mCorDoBrilho.a / 2);
            DesenharSilhueta(renderer, clipRect, renderQuad, angulo, flip, mAlcanceDoBrilho, fora);
            DesenharSilhueta(renderer, clipRect, renderQuad, angulo, flip,
                             (mAlcanceDoBrilho + Destrutivel::kEspessura) / 2, mCorDoBrilho);
        }
        if (mContorno) {
            DesenharSilhueta(renderer, clipRect, renderQuad, angulo, flip, Destrutivel::kEspessura, mCorDoContorno);
        }

        ApplyColorMod();

        // MRotation e guardado em radianos; SDL_RenderCopyEx espera graus.
        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &clipRect, &renderQuad,
                         -Math::ToDegrees(mOwner->GetRotation()), nullptr, flip);
    }
}

void DrawAnimatedComponent::DefinirRastro(const bool ligado) {
    mRastro = ligado;
    mProximoEco = 0.0f;
}

void DrawAnimatedComponent::DefinirRastroDeCor(const bool ligado, const SDL_Color cor) {
    if (ligado) mCorDoRastro = cor;
    else mCorDoRastro.reset();
    // A primeira copia sai logo, onde o desenho esta.
    mOndeFicouOEco.reset();
}

bool DrawAnimatedComponent::QuadroAtual(SDL_Rect& quadro, SDL_Rect& destino) const {
    const auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end() || animIt->second.empty()) return false;
    const auto& frames = animIt->second;
    auto frameIdx = static_cast<size_t>(mAnimTimer);
    if (frameIdx >= frames.size()) frameIdx = frames.size() - 1;
    const int spriteIdx = mUsingFallbackSheet ? 0 : frames[frameIdx];
    if (spriteIdx < 0 || static_cast<size_t>(spriteIdx) >= mSpriteSheetData.size()) return false;
    // As mesmas contas do Draw.
    const Vector2 pos = mOwner->GetPosition();
    const float scale = mOwner->GetScale() * mEscalaPropria;
    quadro = mSpriteSheetData[spriteIdx];
    const int w = static_cast<int>(quadro.w * scale);
    const int h = static_cast<int>(quadro.h * scale);
    destino = {static_cast<int>(pos.x - w / 2.0f), static_cast<int>(pos.y - h / 2.0f), w, h};
    return true;
}

void DrawAnimatedComponent::DesenharEcos(SDL_Renderer* renderer) {
    // Somando a luz, e nao por cima: a copia de um desenho escuro ainda brilha na cor dela. A do
    // rastro de cor sai da silhueta, quando ha uma (sem ela, da folha tingida).
    SDL_Texture* silhueta = nullptr;
    for (const Eco& e : mEcos) {
        if (e.deCor) {
            silhueta = Silhueta();
            break;
        }
    }
    SDL_BlendMode daFolha = SDL_BLENDMODE_BLEND, daSilhueta = SDL_BLENDMODE_BLEND;
    SDL_GetTextureBlendMode(mSpriteSheetSurface, &daFolha);
    SDL_SetTextureBlendMode(mSpriteSheetSurface, SDL_BLENDMODE_ADD);
    if (silhueta) {
        SDL_GetTextureBlendMode(silhueta, &daSilhueta);
        SDL_SetTextureBlendMode(silhueta, SDL_BLENDMODE_ADD);
    }
    for (const Eco& e : mEcos) {
        SDL_Texture* textura = e.deCor && silhueta ? silhueta : mSpriteSheetSurface;
        const float u = 1.0f - e.idade / kVidaDoEco;
        Daltonizar::Tinto(textura, e.cor.r, e.cor.g, e.cor.b);
        SDL_SetTextureAlphaMod(textura, static_cast<Uint8>(std::clamp(u * static_cast<float>(e.cor.a), 0.0f, 255.0f)));
        SDL_RenderCopyEx(renderer, textura, &e.quadro, &e.destino, e.angulo, nullptr, SDL_FLIP_NONE);
        // Duas vezes no comeco da vida: o eco novo do modo deus e quase branco de tanta luz.
        if (!e.deCor && u > 0.6f) SDL_RenderCopyEx(renderer, textura, &e.quadro, &e.destino, e.angulo, nullptr, SDL_FLIP_NONE);
    }
    // A folha volta como estava: a mistura, a cor do componente e opaca. Da silhueta, so a mistura:
    // quem a usa poe a cor e a luz antes.
    SDL_SetTextureBlendMode(mSpriteSheetSurface, daFolha);
    SDL_SetTextureAlphaMod(mSpriteSheetSurface, 255);
    if (silhueta) SDL_SetTextureBlendMode(silhueta, daSilhueta);
    ApplyColorMod();
}

SDL_Texture* DrawAnimatedComponent::Silhueta() {
    if (!mSilhuetaTentada) {
        mSilhuetaTentada = true;
        mSilhueta = mOwner->GetScene()->GetGame()->LoadSilhueta(mCaminhoDaFolha);
    }
    return mSilhueta;
}

void DrawAnimatedComponent::DefinirContorno(const bool ligado, const SDL_Color cor) {
    mContorno = ligado;
    mCorDoContorno = cor;
}

void DrawAnimatedComponent::DefinirBrilho(const bool ligado, const SDL_Color cor, const int alcance) {
    mBrilho = ligado;
    mCorDoBrilho = cor;
    mAlcanceDoBrilho = alcance;
}

void DrawAnimatedComponent::DesenharSilhueta(SDL_Renderer* renderer, const SDL_Rect& quadro,
                                             const SDL_Rect& destino, const double angulo,
                                             const SDL_RendererFlip flip, const int distancia,
                                             const SDL_Color cor) {
    if (distancia <= 0) return;
    // A silhueta so e carregada quando o contorno (ou o brilho) liga pela primeira vez.
    if (!Silhueta()) return;

    Daltonizar::Tinto(mSilhueta, cor.r, cor.g, cor.b);
    SDL_SetTextureAlphaMod(mSilhueta, cor.a);
    const int e = distancia;
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
    // O rastro anda com o relogio, e nao com a animacao.
    for (Eco& e : mEcos) e.idade += deltaTime;
    mEcos.erase(std::remove_if(mEcos.begin(), mEcos.end(), [](const Eco& e) { return e.idade >= kVidaDoEco; }),
                mEcos.end());
    if (mRastro) {
        mProximoEco -= deltaTime;
        Eco e;
        if (mProximoEco <= 0.0f && QuadroAtual(e.quadro, e.destino)) {
            mProximoEco = kEntreEcos;
            e.angulo = -Math::ToDegrees(mOwner->GetRotation());
            const Hud::Rgb c = Hud::ArcoIris(mMatiz);
            e.cor = SDL_Color{c.r, c.g, c.b, kLuzDoEco};
            mMatiz += 0.06f;
            mEcos.push_back(e);
        }
    } else if (mCorDoRastro) {
        // O de cor: no maximo uma copia a cada kEntreEcos, e so depois de andar kPassoDoRastro.
        mProximoEco -= deltaTime;
        const Vector2 aqui = mOwner->GetPosition();
        const bool andou = !mOndeFicouOEco || (aqui - *mOndeFicouOEco).Length() >= kPassoDoRastro;
        Eco e;
        if (mProximoEco <= 0.0f && andou && QuadroAtual(e.quadro, e.destino)) {
            mProximoEco = kEntreEcos;
            mOndeFicouOEco = aqui;
            e.angulo = -Math::ToDegrees(mOwner->GetRotation());
            e.cor = *mCorDoRastro;
            e.deCor = true;
            mEcos.push_back(e);
        }
    }

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

    // Player::AtualizarAnimacao chama isto todo frame com o mesmo nome.
    if (mAnimName == name) return;

    mAnimName = name;
    mAnimTimer = 0.0f; // troca de animacao comeca do frame inicial
}

void DrawAnimatedComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    mAnimations.emplace(name, spriteNums);
}