//
// Segmentos de reta desenhados: ponteiros, buscas e chamadas no campo.
//

#pragma once

#include <vector>
#include <SDL_pixels.h>

#include "DrawComponent.h"
#include "../../Core/Math.h"

/**
 * @class DrawLinhasComponent
 * @brief Desenha uma lista de segmentos, cada um com a sua cor, em coordenadas
 * de TELA - e nao relativas ao dono, como os outros desenhos.
 *
 * Nasceu para a fase 3 do Luizinho mostrar os ponteiros - cada variavel dele
 * e uma linha ate o bloco para onde aponta, e o aluno ve o que esta ligado
 * antes de o coletor passar -, e hoje desenha tambem a busca do nome no
 * escopo, o fluxograma e as chamadas do Andre. Quem preenche a lista e o
 * professor, a cada quadro; o componente so desenha o que recebeu.
 *
 * As linhas tem 2 px: uma linha de 1 px some atras de um projetil de 32.
 */
class DrawLinhasComponent : public DrawComponent {
public:
    struct Linha {
        Vector2 de;
        Vector2 ate;
        SDL_Color cor;
    };

    DrawLinhasComponent(class Actor* owner, int drawOrder);
    void Draw(SDL_Renderer* renderer) override;

    void Limpar() { mLinhas.clear(); }
    void Acrescentar(const Vector2& de, const Vector2& ate, const SDL_Color cor) {
        mLinhas.push_back({de, ate, cor});
    }
    [[nodiscard]] size_t Quantas() const { return mLinhas.size(); }

private:
    std::vector<Linha> mLinhas;
};
