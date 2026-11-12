
#pragma once

#include <string>
#include <unordered_map>
#include <SDL_ttf.h>
#include <SDL_render.h>
#include "Math.h"

/// Uma fonte TTF, aberta em cada tamanho na primeira vez que ele e pedido.
class Font
{
public:
	Font();
	~Font();

	/// Guarda o arquivo e confere que ele abre. Os tamanhos abrem sob demanda; o destrutor os fecha.
	bool Load(const std::string& fileName);

	/// Desenha o texto numa textura. 'escala' sao os pixels da tela por pixel do jogo (EscalaDaTela): a
	/// textura sai 'escala' vezes maior, e quem desenha a divide por ela para voltar ao tamanho do jogo.
	class SDL_Texture* RenderText(SDL_Renderer* renderer, const std::string& text, const Vector3& color = Color::White,
	                              int pointSize = 30, unsigned wrapLength = 900, float escala = 1.0f);

private:
	/// A fonte no tamanho pedido, aberta agora se ainda nao estava.
	TTF_Font* NoTamanho(int pointSize);

	std::string mArquivo;
	std::unordered_map<int, TTF_Font*> mFontData;
};
