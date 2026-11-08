//
// O painel de menu que a pausa e o resultado da batalha compartilham.
//

#pragma once

#include <string>
#include <vector>

#include "../Motor/Math.h"

class Actor;
class Font;
class Scene;

/// Um menu por cima da cena: veu, placa, titulo, linhas de texto, as opcoes com a moldura da que
/// esta em foco, e o rodape.
class PainelDeMenu {
public:
    struct Medidas {
        int larguraDaPlaca = 620;
        int alturaDaPlaca = 380;
        /// Ordem de desenho do veu; o resto vai por cima dele, ate ordem + 3.
        int ordem = 900;
    };

    PainelDeMenu(Scene& cena, Font* fonte, const std::string& titulo,
                 const std::vector<std::string>& opcoes, const std::string& rodape,
                 int linhas, Medidas medidas);

    void Mostrar(bool visivel);

    /// Poe a moldura de foco na opcao 'indice'.
    void Selecionar(int indice);

    void SetTitulo(const std::string& texto, const Vector3& cor);

    /// Escreve as linhas de cima para baixo; as que sobram ficam escondidas. 'destaque' e a linha
    /// pintada de dourado (-1: nenhuma).
    void SetLinhas(const std::vector<std::string>& linhas, int destaque = -1);

private:
    [[nodiscard]] float YDaOpcao(int indice) const;

    Medidas mMedidas;
    Vector2 mMeio;
    bool mVisivel = false;

    Actor* mVeu = nullptr;
    Actor* mPlaca = nullptr;
    Actor* mTitulo = nullptr;
    Actor* mSelecao = nullptr;
    Actor* mRodape = nullptr;
    std::vector<Actor*> mOpcoes;
    std::vector<Actor*> mLinhas;
    size_t mLinhasEmUso = 0;

    static constexpr int kLarguraDaSelecao = 440;
    static constexpr int kAlturaDaSelecao = 54;
    static constexpr float kPassoDasOpcoes = 66.0f;
    static constexpr float kPassoDasLinhas = 36.0f;
};
