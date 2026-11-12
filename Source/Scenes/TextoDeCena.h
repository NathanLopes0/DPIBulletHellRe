//
// O texto de tela que as cenas compartilham.
//

#pragma once

#include <string>

#include "../Motor/Math.h"

class DrawTextComponent;
class Font;
class Scene;

/// Um texto de tela: centrado no ponto, no tamanho natural, quebrando linha na largura e encolhendo
/// (sem deformar) se nao couber na caixa.
DrawTextComponent* CriarTexto(Scene& cena, Font* fonte, const std::string& texto, const Vector2& onde,
                              int tamanho, int largura, int altura, int ordem = 255);
