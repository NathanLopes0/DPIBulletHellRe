//
// A tela que o gabinete mostra quando ninguem esta jogando.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Motor/Gabinete.h"
#include "../Motor/Math.h"

class DrawComponent;
class Font;

/// A tela de atracao do gabinete. Um arcade parado num corredor tem um trabalho: convidar quem
/// passa.
class Atracao : public Scene {
public:
    explicit Atracao(Game* game);
    ~Atracao() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    /// Quanto cada cartaz fica na tela.
    static constexpr float kSegundosPorCartaz = 6.0f;

    /// Quantas posicoes do curso cabem no cartaz do ranking.
    static constexpr size_t kQuantasLinhas = 6;

    /// Um cartaz e so um conjunto de desenhos que aparecem juntos.
    using Cartaz = std::vector<DrawComponent*>;

    Cartaz CriarChamada();
    Cartaz CriarMelhores();

    /// Deixa visivel so o cartaz da vez.
    void MostrarSomente(size_t qual) const;

    DrawComponent* Texto(const std::string& conteudo, float x, float y,
                         int tamanho, int larguraMaxima, const Vector3& cor);

    std::unique_ptr<Font> mFonte;

    std::vector<Cartaz> mCartazes;
    size_t mCartazAtual = 0;

    /// Troca de cartaz. Condicao sempre verdadeira: aqui o tempo nunca para.
    Gabinete::Contagem mTrocaDeCartaz{kSegundosPorCartaz};

    /// A borda de "alguma tecla apertada".
    bool mAlgumaAnterior = true;
};
