//
// A tela em que o aluno digita a matricula.
//

#pragma once

#include <memory>
#include <string>

#include "Scene.h"

class Font;
class Actor;

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
 */
class Identificacao : public Scene {
public:
    explicit Identificacao(Game* game);
    ~Identificacao() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    void CriarTextos();

    /// Atualiza o que esta escrito na tela a partir de mDigitado e mErro.
    void Redesenhar() const;

    /// @brief Quais teclas foram APERTADAS AGORA (estavam soltas no quadro anterior).
    ///
    /// Digitar precisa de borda, e nao de estado: com o estado puro, segurar o 8
    /// por meio segundo encheria o campo de oitos. O jogo todo le o teclado por
    /// estado (a selecao de fase usa um temporizador para contornar isso), mas
    /// para digitar um temporizador fica lento demais.
    void LerTeclado(const Uint8* keyState);

    std::unique_ptr<Font> mFonte;

    /// O que o aluno digitou ate agora. So digito, no maximo seis - garantido por
    /// Matricula::Digitar, nao por esta classe.
    std::string mDigitado;

    /// A frase de erro em exibicao, vazia quando nao ha erro.
    std::string mErro;

    /// Estado do teclado no quadro anterior, para detectar borda.
    bool mDigitoAnterior[10]{};
    bool mEnterAnterior{};
    bool mApagarAnterior{};
    bool mVisitanteAnterior{};

    Actor* mTituloAtor{};
    Actor* mCampoAtor{};
    Actor* mErroAtor{};
    Actor* mAjudaAtor{};
};
