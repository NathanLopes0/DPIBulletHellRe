//
// A tela que mostra as maiores notas de cada materia.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Aluno/Exportacao.h"
#include "../Aluno/Ranking.h"
#include "../Motor/Game.h"

class Font;
class Actor;

/**
 * @class TelaDeRanking
 * @brief As maiores notas, uma materia por vez, em quatro listas: Normal,
 * Dificil, Normal em dupla e Dificil em dupla (Ranking::kListas).
 *
 * POR QUE ELA EXISTE. A curva da nota foi feita inteira para que a nota
 * ordenasse um ranking de verdade - era o argumento contra todo mundo tirar 100.
 * Ate aqui esse ranking so existia na planilha do professor, e quem joga no
 * gabinete nunca via onde estava.
 *
 * ESTA CENA NAO ORDENA NADA. A ordem, os empates e o corte do topo vivem em
 * Source/Aluno/Ranking.h, que e puro e testado. Aqui so ha leitura de manche e
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

private:

    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho,
                 int larguraMaxima, bool aEsquerda = false);

    /// Reescreve as linhas a partir da materia selecionada e da lista.
    void Redesenhar();

    /// A lista em exibicao (Ranking::kListas).
    [[nodiscard]] const Ranking::Lista& ListaAtual() const { return Ranking::kListas[mLista]; }
    [[nodiscard]] bool EmDupla() const { return ListaAtual().modo == Progresso::Modo::Dupla; }

    std::unique_ptr<Font> mFonte;

    /// A turma inteira, lida uma vez. Ver o comentario de classe.
    std::vector<Exportacao::FichaDeAluno> mFichas;

    /// O que mostrar e para onde voltar, posto por quem pediu a tela.
    /// Ver Game::PedidoDeRanking.
    Game::PedidoDeRanking mPedido{};

    /**
     * QUAL DAS QUATRO LISTAS esta aberta: o indice em Ranking::kListas.
     *
     * E ESTADO DA TELA, e nao do pedido: a lista nao e algo que a selecao de
     * fases ou o menu pedem, e quem troca e o jogador olhando. Pondo em
     * PedidoDeRanking, os dois lugares que montam o pedido teriam de saber de
     * um campo que nao lhes diz respeito.
     *
     * COMECA NA LISTA DA PARTIDA: quem acabou de jogar em dupla no Dificil quer
     * ver a lista da dupla no Dificil. Nao e um campo novo - sai de
     * QuantosJogadores() e de NivelDeDificuldade().
     */
    int mLista = 0;

    /// As quatro abas, na ordem de kListas: a moldura e o nome de cada uma.
    std::vector<class DrawCaixaComponent*> mAbas;
    std::vector<class DrawTextComponent*> mNomesDasAbas;

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

    Actor* mRodapeAtor{};

    bool mVoltarAnterior = true;
    bool mAlternarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
};
