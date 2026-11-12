//
// A tela do fim da luta: o resumo, a devolutiva, os alunos e o toca aqui.
//

#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#include <SDL.h>

#include "LeitorDoMenu.h"
#include "../../Interface/Comunicacao.h"
#include "../../Interface/Devolutiva.h"
#include "../../Interface/Resultado.h"
#include "../../Motor/Math.h"

class Actor;
class Boss;
class DrawLinhasComponent;
class Font;
class Game;
class PainelDeMenu;
class Scene;

/// O painel do resultado (Resultado.h): o resumo, o desenho do que mais acertou o aluno (a
/// devolutiva) e os alunos dos lados do titulo. Na dupla aprovada, o toca aqui (Comunicacao.h): os
/// dois apertam o Botao 3 juntos, os avatares pulam e um estouro dourado abre entre eles. Escolhida
/// uma opcao - ou passado o tempo, no gabinete -, pede a troca de cena.
class TelaDoResultado {
public:
    /// 'jogadores': quantos alunos lutaram. 'toque': o experimento mostra o toca aqui
    /// (Experimento.h). A medida conta sempre, e chega a batalha pelo 'aoTocarAqui'.
    TelaDoResultado(Scene& cena, Game& jogo, Font* fonte, std::size_t jogadores, bool toque,
                    std::function<void()> aoTocarAqui);
    ~TelaDoResultado();

    /// Monta e mostra o painel do resumo. 'chefe': de onde sai o desenho de cada causa da devolutiva.
    void Mostrar(const Resultado::Resumo& resumo, Boss* chefe);

    /// Um quadro com o painel na tela: o relogio dele, o pulo e o estouro, e a volta sozinha.
    void Avancar(float deltaTime);

    /// Le o menu e o toca aqui.
    void Processar(const Uint8* keyState);

    /// Segundos desde que o painel apareceu.
    [[nodiscard]] float Tempo() const { return mTempo; }

private:
    /// O painel, do tamanho do resumo. 'comToque': o rodape ensina o toca aqui.
    void CriarPainel(int linhas, bool comToque);
    /// A personagem dos alunos dos dois lados do titulo - no solo, a mesma; na dupla, cada aluno do
    /// seu lado: na formatura, de capelo, e na dupla aprovada, para o toca aqui.
    void MostrarAlunos();
    /// O que mais acertou o aluno, na linha da devolutiva: o desenho de cada projetil e quantas vezes.
    void MostrarCausas(const std::vector<Devolutiva::Causa>& causas, const Vector2& onde, Boss* chefe);
    /// Os dois Botoes 3 juntos sao o toca aqui.
    void LerToque(const Uint8* keyState);
    void TocarAqui();
    void AtualizarToque(float deltaTime);
    void Escolher();

    Scene& mCena;
    Game& mJogo;
    Font* mFonte;
    std::size_t mJogadores;
    bool mToque;
    std::function<void()> mAoTocarAqui;

    std::unique_ptr<PainelDeMenu> mPainel;
    LeitorDoMenu mLeitor;
    int mSelecionada = 0;
    float mTempo = 0.0f;

    struct Avatar {
        Actor* ator = nullptr;
        Vector2 base;
        float pulo = Comunicacao::kDuracaoDoPulo;   ///< parado: o pulo ja acabou
    };
    std::vector<Avatar> mAvatares;
    bool mPodeTocar = false;
    std::array<bool, 2> mTresAntes{};
    std::array<float, 2> mUltimoTres{Comunicacao::kNuncaApertou, Comunicacao::kNuncaApertou};
    float mUltimoToque = Comunicacao::kNuncaApertou;
    float mEstouro = Comunicacao::kDuracaoDoEstouro;   ///< parado: o estouro ja apagou
    DrawLinhasComponent* mDesenhoDoToque = nullptr;

    /// Por cima de tudo: a faixa da nota vai ate 304.
    static constexpr int kOrdem = 900;
    /// O lado do desenho de cada projetil na devolutiva: o de uma linha do resultado.
    static constexpr float kLadoDoIcone = 34.0f;
    /// Quem se formou, dos lados do titulo: dentro da placa do resultado e acima das linhas.
    static constexpr float kAfastamentoDoFormando = 200.0f;
    static constexpr float kEscalaDoFormando = 1.5f;
    /// O resultado e mais largo que a pausa: a linha da devolutiva traz o nome da fase.
    static constexpr int kLargura = 760;
};
