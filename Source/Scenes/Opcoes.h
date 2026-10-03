//
// A tela de opcoes. Vazia por enquanto, de proposito.
//

#pragma once

#include <memory>
#include <string>

#include "Scene.h"

class Font;
class Actor;

/**
 * @class Opcoes
 * @brief Onde as configuracoes do jogo vao morar.
 *
 * ESTA TELA NAO FAZ NADA AINDA, e isso e intencional: ela existe para que a
 * terceira opcao do menu ja tenha destino proprio desde o inicio. Quando houver
 * o que configurar - volume, teclas, tela cheia - o lugar ja esta aqui, e o
 * menu nao precisa mudar.
 *
 * Deixar a opcao sem destino ate la seria pior: ou ela nao responderia (e
 * pareceria defeito), ou alguem a esconderia e teria que reacrescenta-la depois.
 */
class Opcoes : public Scene {
public:
    explicit Opcoes(Game* game);
    ~Opcoes() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    /// Cria um ator de texto centrado horizontalmente, no tamanho natural dele.
    Actor* Texto(const std::string& conteudo, float y, int tamanho, int larguraMaxima);

    std::unique_ptr<Font> mFonte;

    /// Borda do ESC, para uma acao por toque. Comeca em TRUE pela mesma regra
    /// das outras cenas: nada hoje entra aqui com ESC apertado, mas a guarda
    /// custa um bool e ja nos custou um bug quando faltou.
    bool mVoltarAnterior = true;
};
