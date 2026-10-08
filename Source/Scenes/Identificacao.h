//
// A tela em que o aluno digita a matricula.
//

#pragma once

#include <memory>
#include <string>

#include <vector>

#include "Scene.h"
#include "../Navegacao.h"
#include "../Teclado.h"

class Font;
class Actor;
class DrawCaixaComponent;

/**
 * @class Identificacao
 * @brief Pergunta a matricula do aluno, ou deixa entrar como visitante.
 *
 * ESTA CENA NAO DECIDE NADA SOBRE MATRICULA. Toda a regra - o que e um digito
 * valido, quantos cabem, o que e uma matricula aceitavel, qual frase mostrar para
 * cada erro - vive em Source/Matricula.h, que e puro e coberto por testes. Aqui
 * so ha leitura de teclado e desenho.
 *
 * E a mesma divisao que separa os leitores de dados das pontes, e o motivo e o
 * mesmo: a regra assim pode ser testada sem subir uma janela.
 *
 * ENTRAR SEM SE IDENTIFICAR E PERMITIDO, por decisao de projeto: o login existe
 * para o professor saber quem jogou, nao para barrar ninguem. Quem entra como
 * visitante joga igual, mas o progresso dele nao e gravado - e a tela diz isso
 * antes, nao depois.
 *
 * DOIS MODOS, UMA TELA. Novo Jogo recusa matricula que JA tem perfil; Carregar
 * Perfil recusa matricula que NAO tem. E a mesma regra vista dos dois lados, e
 * o motivo dela e a maquina compartilhada do departamento: quem digita a
 * matricula de outro por engano nao pode apagar o progresso dele, e quem erra um
 * digito so redigita. Ver Documentacao/requisitos-perfil-e-personagem.md, D2/D3.
 */
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

    /// Executa a tecla selecionada. E o unico lugar que decide o que cada acao
    /// faz, entao o botao do gabinete e o ENTER do teclado chegam aqui iguais.
    void Acionar(const Teclado::Tecla& tecla);

    /// A navegacao do teclado digital, com o atraso de repeticao.
    void LerManche(const Uint8* keyState, float deltaTime);

    /// Cria um ator de texto centrado horizontalmente, no tamanho natural dele.
    /// A largura e a altura dadas sao LIMITE, nao destino - ver
    /// DrawTextComponent::SetAjustarAoTexto.
    Actor* Texto(const std::string& inicial, float y, int tamanho,
                 int larguraMaxima, int alturaMaxima);

    /// Atualiza o que esta escrito na tela a partir de mDigitado e mErro.
    void Redesenhar() const;

    /// @brief Quais teclas foram APERTADAS AGORA (estavam soltas no quadro anterior).
    ///
    /// Digitar precisa de borda, e nao de estado: com o estado puro, segurar o 8
    /// por meio segundo encheria o campo de oitos. O jogo todo le o teclado por
    /// estado (a selecao de fase usa um temporizador para contornar isso), mas
    /// para digitar um temporizador fica lento demais.
    void LerTeclado(const Uint8* keyState);

    /// O que fazer quando a matricula digitada e valida. Separa a REGRA de fluxo
    /// (criar ou carregar) da leitura de teclado.
    void Confirmar(const std::string& canonica);

    std::unique_ptr<Font> mFonte;

    /// A que viemos. Vem do Game, posto pelo menu.
    bool mModoNovo = true;

    /// O que o aluno digitou ate agora. So digito, no maximo seis - garantido por
    /// Matricula::Digitar, nao por esta classe.
    std::string mDigitado;

    /// A frase de erro em exibicao, vazia quando nao ha erro.
    std::string mErro;

    /// Estado do teclado no quadro anterior, para detectar borda.
    bool mDigitoAnterior[10]{};

    /// Comeca em TRUE, e e obrigatorio: o menu agora confirma com ENTER, entao
    /// quem escolhe "Novo Jogo" chega aqui com o ENTER ainda apertado. Em FALSE,
    /// o primeiro quadro validaria a matricula vazia e a tela ja abriria com a
    /// frase de erro. E o mesmo vazamento que a selecao de fase teve.
    bool mEnterAnterior = true;
    bool mApagarAnterior{};
    bool mVisitanteAnterior{};
    bool mVoltarAnterior = true;

    // --- O teclado digital ---
    //
    // O GABINETE NAO TEM TECLADO. A matricula e digitada escolhendo numeros numa
    // grade com o manche e confirmando no botao. O teclado fisico continua
    // funcionando, para desenvolver sem o gabinete.
    Navegacao::Grade mGradeDoTeclado;
    size_t mTeclaSelecionada = 0;
    std::vector<DrawCaixaComponent*> mCaixasDasTeclas;

    /// Atraso entre dois passos da selecao, para segurar o manche nao atravessar
    /// a grade. Mesmo recurso da selecao de fases, pelo mesmo motivo.
    ///
    /// COMECA PRONTO, e nao em zero: zerado, o primeiro toque no manche e
    /// engolido enquanto o temporizador carrega, e a tela parece nao responder
    /// justamente no instante em que a pessoa a esta conhecendo. A selecao de
    /// fases ja faz assim pelo mesmo motivo.
    float mPassoTimer = kPassoDoManche;

    bool mAcionarAnterior = true;   ///< ver mEnterAnterior: chega-se aqui com o botao apertado

    Actor* mTituloAtor{};
    Actor* mCampoAtor{};
    Actor* mErroAtor{};

    /// A moldura do campo. Guardada para mudar de cor quando ha erro.
    DrawCaixaComponent* mCaixaDesenho{};

    /// O cursor pisca para a tela nao parecer travada enquanto ninguem digita.
    float mPiscaTimer = 0.0f;
    bool mCursorAceso = true;
};
