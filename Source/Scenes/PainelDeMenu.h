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

/**
 * @class PainelDeMenu
 * @brief Um menu por cima da cena: veu, placa, titulo, linhas de texto, as
 * opcoes com a moldura da que esta em foco, e o rodape.
 *
 * A PAUSA E O RESULTADO SAO O MESMO DESENHO com conteudo diferente. A primeira
 * versao da pausa criava os seis atores a mao dentro da Battle; copiar aquilo
 * para o resultado daria duas receitas de veu, placa e moldura que mudariam
 * separadas no primeiro ajuste - e a tela de fim sairia com um tom de cinza
 * diferente da pausa sem ninguem notar de onde veio.
 *
 * O PAINEL SO DESENHA. Qual opcao esta em foco, o que cada uma faz e quando o
 * menu aceita botao e de quem o usa (Pausa.h, Resultado.h): aqui so se diz
 * "mostre", "foque a opcao i" e "escreva isto".
 *
 * Os atores sao da cena, como todos (Scene::AddActor); o painel guarda so os
 * ponteiros, e nao precisa destruir nada.
 *
 * A DISPOSICAO: o titulo e as linhas descem a partir do topo da placa; as
 * opcoes e o rodape sobem a partir de baixo. Assim um resumo com uma linha a
 * menos (o visitante nao tem recorde) so abre espaco no meio, e os botoes
 * ficam no mesmo lugar.
 */
class PainelDeMenu {
public:
    struct Medidas {
        int larguraDaPlaca = 620;
        int alturaDaPlaca = 380;
        /// Ordem de desenho do veu; o resto vai por cima dele, ate ordem + 3.
        int ordem = 900;
    };

    /**
     * @param linhas quantas linhas de texto cabem entre o titulo e as opcoes.
     *        Comecam vazias e escondidas (SetLinhas).
     */
    PainelDeMenu(Scene& cena, Font* fonte, const std::string& titulo,
                 const std::vector<std::string>& opcoes, const std::string& rodape,
                 int linhas, Medidas medidas);

    void Mostrar(bool visivel);

    /// Poe a moldura de foco na opcao 'indice'.
    void Selecionar(int indice);

    void SetTitulo(const std::string& texto, const Vector3& cor);

    /// @brief Escreve as linhas de cima para baixo; as que sobram ficam
    /// escondidas. 'destaque' e a linha pintada de dourado (-1: nenhuma).
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
    /// Quantas linhas estao escritas agora: so elas aparecem em Mostrar.
    size_t mLinhasEmUso = 0;

    static constexpr int kLarguraDaSelecao = 440;
    static constexpr int kAlturaDaSelecao = 54;
    static constexpr float kPassoDasOpcoes = 66.0f;
    static constexpr float kPassoDasLinhas = 36.0f;
};
