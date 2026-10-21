//
// Segmentos de reta desenhados: o grafo de referencias do coletor.
//

#pragma once

#include <vector>
#include <SDL_pixels.h>

#include "DrawComponent.h"
#include "../../Math.h"

/**
 * @class DrawLinhasComponent
 * @brief Desenha uma lista de segmentos, cada um com a sua cor, em coordenadas
 * de TELA - e nao relativas ao dono, como os outros desenhos.
 *
 * Existe para a fase 3 do Luizinho mostrar os ponteiros: cada referencia entre
 * dois blocos de memoria e uma linha, e o aluno ve o que esta ligado ao
 * professor antes de o coletor passar. Quem preenche a lista e o professor, a
 * cada quadro; o componente so desenha o que recebeu.
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
