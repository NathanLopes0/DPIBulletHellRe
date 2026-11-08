//
// O texto de tela que as cenas compartilham.
//

#pragma once

#include <string>

#include "../Motor/Math.h"

class DrawTextComponent;
class Font;
class Scene;

/**
 * @brief Um texto de tela: centrado no ponto, no tamanho natural, quebrando
 * linha na largura e ENCOLHENDO (sem deformar) se nao couber na caixa.
 *
 * ERAM DEZ COPIAS DESTA RECEITA, uma por cena (Opcoes::Texto,
 * TelaDeRanking::Texto, EscolhaDoOrientador::Texto...), cada uma com as
 * mesmas seis linhas e um detalhe proprio. Os detalhes continuam nas cenas -
 * onde fica o texto, de que tamanho e a caixa, a cor -, e a receita fica aqui:
 * o dia em que o jeito de desenhar texto mudar, muda em um lugar.
 *
 * A QUEBRA ACOMPANHA A LARGURA: o padrao do componente e 500 px, e uma caixa
 * mais larga quebraria no meio sem pedir. Texto vazio vira um espaco: vazio
 * nao gera textura.
 *
 * O ator e da cena (Scene::AddActor); o que volta e o componente, que e o que
 * quem chama quer mexer depois - o ator sai dele, com GetOwner().
 */
DrawTextComponent* CriarTexto(Scene& cena, Font* fonte, const std::string& texto, const Vector2& onde,
                              int tamanho, int largura, int altura, int ordem = 255);
