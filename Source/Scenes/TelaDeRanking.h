//
// A tela que mostra as maiores notas de cada materia.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Exportacao.h"
#include "../Game.h"

class Font;
class Actor;

/**
 * @class TelaDeRanking
 * @brief As maiores notas, uma materia por vez.
 *
 * POR QUE ELA EXISTE. A curva da nota foi feita inteira para que a nota
 * ordenasse um ranking de verdade - era o argumento contra todo mundo tirar 100.
 * Ate aqui esse ranking so existia na planilha do professor, e quem joga no
 * gabinete nunca via onde estava.
 *
 * ESTA CENA NAO ORDENA NADA. A ordem, os empates e o corte do topo vivem em
 * Source/Ranking.h, que e puro e testado. Aqui so ha leitura de manche e
 * desenho - a mesma divisao da tela de matricula.
 *
 * AS FICHAS SAO LIDAS UMA VEZ, no Load. Reler a cada quadro custaria um passeio
 * pelo disco inteiro por quadro, e o ranking nao muda enquanto ele esta aberto:
 * quem jogou acabou de sair da batalha.
 */
class TelaDeRanking : public Scene {
public:
    explicit TelaDeRanking(Game* game);
    ~TelaDeRanking() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:

    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho,
                 int larguraMaxima, bool aEsquerda = false);

    /// Reescreve as linhas a partir da materia selecionada e do modo.
    void Redesenhar();

    std::unique_ptr<Font> mFonte;

    /// A turma inteira, lida uma vez. Ver o comentario de classe.
    std::vector<Exportacao::FichaDeAluno> mFichas;

    /// O que mostrar e para onde voltar, posto por quem pediu a tela.
    /// Ver Game::PedidoDeRanking.
    Game::PedidoDeRanking mPedido{};

    /**
     * Mostrando o ranking de DUPLAS, e nao o de solo.
     *
     * E ESTADO DA TELA, e nao do pedido: o modo nao e algo que a selecao de
     * fases ou o menu pedem, e quem alterna e o jogador olhando a lista. Pondo
     * em PedidoDeRanking, os dois lugares que montam o pedido teriam de saber
     * de um campo que nao lhes diz respeito.
     *
     * COMECA NO MODO EM QUE SE ESTA JOGANDO: quem acabou de jogar em dupla quer
     * ver a lista de duplas. Nao e um campo novo - sai de QuantosJogadores().
     */
    bool mDuplas = false;

    Actor* mTituloAtor{};
    Actor* mSubtituloAtor{};
    Actor* mVazioAtor{};
    Actor* mSuaPosicaoAtor{};

    /// Uma linha de texto por posicao do ranking, criadas uma vez e reescritas.
    /// Criar e destruir atores a cada troca de materia deixaria lixo na cena.
    std::vector<Actor*> mPosicaoAtores;
    std::vector<Actor*> mMatriculaAtores;

    /// A dupla inteira ("89384 + 90112") numa caixa so, no lugar da coluna de
    /// matricula. Uma caixa propria porque duas matriculas e um "+" nao cabem
    /// na largura de uma, e porque assim as colunas de nota e de materias
    /// jogadas nao se movem quando o jogador alterna de lista.
    std::vector<Actor*> mDuplaAtores;
    std::vector<Actor*> mJogadasAtores;
    std::vector<Actor*> mNotaAtores;

    /// O rodape e reescrito na troca de lista: ele anuncia PARA ONDE o BOTAO 3
    /// leva, e nao onde se esta.
    Actor* mRodapeAtor{};

    bool mVoltarAnterior = true;
    bool mAlternarAnterior = true;
};
