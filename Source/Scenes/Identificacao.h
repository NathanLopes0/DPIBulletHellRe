//
// A tela em que o aluno digita a matricula.
//

#pragma once

#include <memory>
#include <string>

#include <vector>

#include "Scene.h"
#include "../Entrada/Borda.h"
#include "../Motor/Game.h"
#include "../Entrada/Navegacao.h"
#include "../Entrada/Painel.h"
#include "../Entrada/Teclado.h"

class Font;
class Actor;
class DrawCaixaComponent;

/// Pergunta a matricula do aluno, ou deixa entrar como visitante. Esta cena nao decide nada sobre
/// matricula.
class Identificacao : public Scene {
public:
    /// Segundos entre dois passos com o manche segurado.
    static constexpr float kPassoDoManche = 0.18f;

    explicit Identificacao(Game* game);
    ~Identificacao() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    void CriarTextos();

    /// Desenha o teclado digital: uma caixa e um rotulo por tecla.
    void CriarTeclado();

    /// Acende a tecla sob a selecao e apaga a anterior.
    void PintarSelecao() const;

    /// Executa a tecla selecionada. E o unico lugar que decide o que cada acao faz, entao o botao
    /// do gabinete e o enter do teclado chegam aqui iguais.
    void Acionar(const Teclado::Tecla& tecla);

    /// A navegacao do teclado digital, com o atraso de repeticao.
    void LerManche(const Uint8* keyState, float deltaTime);

    /// Cria um ator de texto centrado horizontalmente, no tamanho natural dele.
    Actor* Texto(const std::string& inicial, float y, int tamanho,
                 int larguraMaxima, int alturaMaxima);

    /// Atualiza o que esta escrito na tela a partir de mDigitado e mErro.
    void Redesenhar() const;

    /// Digitar precisa de borda, e nao de estado: com o estado puro, segurar o 8 por meio segundo
    /// encheria o campo de oitos.
    void LerTeclado(const Uint8* keyState);

    /// O que fazer quando a matricula digitada e valida. Separa a regra de fluxo (criar ou
    /// carregar) da leitura de teclado.
    void Confirmar(const std::string& canonica);

    std::unique_ptr<Font> mFonte;

    /// Qual posicao esta digitando, e se ha outra. Esta tela roda duas vezes no co-op, e quem diz
    /// de quem e a vez e o Game.
    size_t mQuem = 0;
    bool mEmDupla = false;

    /// O painel da vez. E por isso que o jogador 2 digita com o manche dele, e nao esperando o
    /// jogador 1 digitar por ele.
    [[nodiscard]] Painel::Jogador PainelDaVez() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    /// So digito, no maximo seis - garantido por Matricula::Digitar, nao por esta classe.
    std::string mDigitado;

    /// A frase de erro em exibicao, vazia quando nao ha erro.
    std::string mErro;

    /// Estado do teclado no quadro anterior, para detectar borda.
    Borda mBordaDigito[10]{};

    /// Em false, o primeiro quadro validaria a matricula vazia e a tela ja abriria com a frase de
    /// erro. E o mesmo vazamento que a selecao de fase teve.
    Borda mBordaEnter{true};
    Borda mBordaApagar;
    Borda mBordaVisitante;
    Borda mBordaVoltar{true};

    // ---- O teclado digital ----
    Navegacao::Grade mGradeDoTeclado;
    size_t mTeclaSelecionada = 0;
    std::vector<DrawCaixaComponent*> mCaixasDasTeclas;

    /// Atraso entre dois passos da selecao, para segurar o manche nao atravessar a grade. Mesmo
    /// recurso da selecao de fases, pelo mesmo motivo.
    float mPassoTimer = kPassoDoManche;

    Borda mBordaAcionar{true};

    Actor* mTituloAtor{};
    Actor* mCampoAtor{};
    Actor* mErroAtor{};

    /// A moldura do campo. Guardada para mudar de cor quando ha erro.
    DrawCaixaComponent* mCaixaDesenho{};

    /// O cursor pisca para a tela nao parecer travada enquanto ninguem digita.
    float mPiscaTimer = 0.0f;
    bool mCursorAceso = true;
};
