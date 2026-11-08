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

/// As maiores notas, uma materia por vez, em quatro listas: Normal, Dificil, Normal em dupla e
/// Dificil em dupla (Ranking::kListas).
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

    /// A turma inteira, lida uma vez.
    std::vector<Exportacao::FichaDeAluno> mFichas;

    /// O que mostrar e para onde voltar, posto por quem pediu a tela.
    Game::PedidoDeRanking mPedido{};

    /// Qual das quatro listas esta aberta: o indice em Ranking::kListas.
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

    std::vector<Actor*> mDuplaAtores;
    std::vector<Actor*> mJogadasAtores;
    std::vector<Actor*> mNotaAtores;

    Actor* mRodapeAtor{};

    bool mVoltarAnterior = true;
    bool mAlternarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
};
