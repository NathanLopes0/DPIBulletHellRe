//
// O que se desenha por cima dos alunos: o feixe e o escudo do Ponto Extra, o sinal, o valeu, a mira
// presa e a hitbox.
//

#pragma once

#include <cstddef>
#include <vector>

#include "../../Motor/Math.h"

class DrawLinhasComponent;
class Player;
class Scene;

/// Linhas na frente do aluno (100): o escudo e o que se le, e nao o sprite. So a dupla tem feixe,
/// escudo, sinal e valeu - sozinho, so com a hitbox ligada ha o que desenhar, e o desenho nem nasce.
class DesenhoSobreOsAlunos {
public:
    explicit DesenhoSobreOsAlunos(Scene& cena);

    /// O feixe do Ponto Extra da dupla, de quem gastou ate o parceiro (PontoExtra::FeixeEm).
    void Feixe(const Player* de, const Player* ate);
    /// O sinal do Botao 3: um anel em volta de quem chamou, na cor da posicao 'cor' (Comunicacao.h).
    void Sinal(const Player* quem, std::size_t cor);
    /// O valeu: o coracao sobe de quem foi coberto, na cor da posicao de quem cobriu.
    void Valeu(const Player* coberto, std::size_t cor);

    /// Um quadro: anda e redesenha tudo. 'foco': em quem a mira do professor esta presa (Papeis.h), -1
    /// se em ninguem. 'hitbox': a opcao da tela de Opcoes.
    void Atualizar(float deltaTime, const std::vector<Player*>& jogadores, int foco, bool hitbox);

    /// O relogio do pulso do escudo, que tambem pulsa o brilho dos projeteis a caminho de um aluno.
    [[nodiscard]] float Pulso() const { return mPulso; }

private:
    void DesenharComunicacao(float deltaTime);

    struct FeixeEmCampo {
        const Player* de = nullptr;
        const Player* ate = nullptr;
        float t = 0.0f;
    };
    struct SinalEmCampo {
        const Player* quem = nullptr;
        std::size_t cor = 0;             ///< a posicao de quem deu o sinal: a cor do anel
        float t = 0.0f;
    };
    struct ValeuEmCampo {
        const Player* coberto = nullptr;
        std::size_t cor = 0;             ///< a posicao de quem cobriu: a cor do coracao
        float t = 0.0f;
    };

    Scene& mCena;
    DrawLinhasComponent* mLinhas = nullptr;
    std::vector<FeixeEmCampo> mFeixes;
    std::vector<SinalEmCampo> mSinais;
    std::vector<ValeuEmCampo> mValeus;
    float mPulso = 0.0f;

    /// Na frente do aluno (100) e atras dos efeitos (104).
    static constexpr int kOrdem = 102;
    /// O "v" da mira fica isto acima do centro do aluno: logo acima da cabeca (o desenho tem 64 px).
    static constexpr float kAlturaDaMira = 44.0f;
};
