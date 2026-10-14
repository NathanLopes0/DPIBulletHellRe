//
// A tela que o gabinete mostra quando ninguem esta jogando.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Gabinete.h"
#include "../Math.h"

class DrawComponent;
class Font;

/**
 * @brief A TELA DE ATRACAO do gabinete.
 *
 * Um arcade parado num corredor tem um trabalho: convidar quem passa. Ate aqui
 * a maquina ociosa mostrava o menu principal parado, que nao diz o que o jogo e
 * nem que da para jogar.
 *
 * QUEM ABRE ESTA TELA E A OCIOSIDADE, e nao uma opcao de menu: passado o tempo
 * de --ociosidade no menu principal, o Game troca para ca (ver
 * Game::AtualizarGabinete). Fora do gabinete a ociosidade nasce desligada,
 * entao esta tela simplesmente nunca aparece - e e assim que se desenvolve sem
 * ela no caminho.
 *
 * QUALQUER TECLA VOLTA AO MENU. Nao ha o que escolher aqui, entao nao ha botao
 * certo: quem encostou no painel quer jogar.
 *
 * Os cartazes se alternam sozinhos. O relogio e o mesmo Gabinete::Contagem das
 * duas esperas do gabinete - terceiro uso da mesma classe, com a condicao
 * sempre verdadeira.
 */
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
    ///
    /// Guarda os COMPONENTES, e nao os atores: a troca de cartaz e ligar e
    /// desligar desenho, e o Actor nao expoe a lista de componentes dele.
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

    /// A borda de "alguma tecla apertada". Comeca em TRUE porque se chega aqui
    /// por ociosidade - sem ninguem apertando nada -, mas uma tecla PRESA (um
    /// botao emperrado do painel) nao pode fazer a tela piscar entre menu e
    /// atracao para sempre. Exigir a batida nova resolve os dois casos.
    bool mAlgumaAnterior = true;
};
