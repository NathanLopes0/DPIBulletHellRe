#include "Font.h"

#include <algorithm>
#include <cmath>

Font::Font()
{
}

Font::~Font()
{
	for (auto& font : mFontData)
	{
		TTF_CloseFont(font.second);
	}
}

bool Font::Load(const std::string& fileName)
{
	mArquivo = fileName;
	return NoTamanho(24) != nullptr;
}

TTF_Font* Font::NoTamanho(const int pointSize)
{
	const auto iter = mFontData.find(pointSize);
	if (iter != mFontData.end()) return iter->second;

	TTF_Font* font = TTF_OpenFont(mArquivo.c_str(), pointSize);
	if (font == nullptr)
	{
		SDL_Log("Failed to load font %s in size %d: %s", mArquivo.c_str(), pointSize, TTF_GetError());
		return nullptr;
	}
	mFontData.emplace(pointSize, font);
	return font;
}

SDL_Texture* Font::RenderText(SDL_Renderer* renderer, const std::string& text, const Vector3& color,
                              const int pointSize, const unsigned wrapLength, const float escala)
{
	// Vermelho em x, verde em y, azul em z.
	SDL_Color sdlColor;
	sdlColor.r = static_cast<Uint8>(color.x * 255);
	sdlColor.g = static_cast<Uint8>(color.y * 255);
	sdlColor.b = static_cast<Uint8>(color.z * 255);
	sdlColor.a = 255;

	// O tamanho na tela de verdade: o texto nasce nitido, em vez de ser ampliado depois.
	const int tamanho = std::max(1, static_cast<int>(std::lround(static_cast<float>(pointSize) * escala)));
	TTF_Font* font = NoTamanho(tamanho);
	if (!font) return nullptr;

	const auto quebra = static_cast<Uint32>(std::lround(static_cast<float>(wrapLength) * escala));
	SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), sdlColor, quebra);
	if (!surf)
	{
		SDL_Log("Failed to create surface for text: %s", text.c_str());
		return nullptr;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	if (!texture)
	{
		SDL_Log("Failed to render surface to texture");
		return nullptr;
	}

	// Linear: a caixa que encolhe o texto (SetAjustarAoTexto) o reduz sem serrilhar.
	SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
	return texture;
}
