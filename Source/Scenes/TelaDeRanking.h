//
// A tela que mostra as maiores notas de cada materia.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Exportacao.h"

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

    /// Reescreve as linhas a partir da materia selecionada.
    void Redesenhar();

    std::unique_ptr<Font> mFonte;

    /// A turma inteira, lida uma vez. Ver o comentario de classe.
    std::vector<Exportacao::FichaDeAluno> mFichas;

    /// Qual materia esta em exibicao. Indice na lista de materias.
    int mMateria = 0;

    Actor* mTituloAtor{};
    Actor* mVazioAtor{};
    Actor* mSuaPosicaoAtor{};

    /// Uma linha de texto por posicao do ranking, criadas uma vez e reescritas.
    /// Criar e destruir atores a cada troca de materia deixaria lixo na cena.
    std::vector<Actor*> mPosicaoAtores;
    std::vector<Actor*> mMatriculaAtores;
    std::vector<Actor*> mNotaAtores;

    float mPassoTimer = 0.0f;
    bool mVoltarAnterior = true;
};
