//
// O piloto da simulacao: quem joga no lugar do aluno quando o jogo roda sozinho.
//
#pragma once

#include "../../Motor/Math.h"

class Player;
class RigidBodyComponent;

/// O piloto de Simulacao.h (--piloto segue|parado|vaivem|esquiva), no lugar do manche. Mora fora do Player
/// para que o aluno nao precise conhecer o Hugo: e o piloto que sabe ir ao alvo do objetivo do TCC.
class PilotoDoAluno {
public:
    /// Dirige o aluno neste quadro: poe a velocidade no corpo e diz se ele anda. Devolve false quando
    /// a simulacao nao tem piloto, e o manche vale.
    bool Dirigir(Player& aluno, RigidBodyComponent& corpo, bool& andando);

private:
    /// O piloto que desvia (Simulacao::Desviar): os tiros do chefe que machucam, vistos daqui.
    bool Desviar(Player& aluno, RigidBodyComponent& corpo, bool& andando, float alvoX, float alvoY);
    /// O mesmo piloto no jogo de plataforma (Simulacao::Saltar): anda pelo chao rumo a 'alvo' e pula o
    /// que vem.
    bool Saltar(Player& aluno, RigidBodyComponent& corpo, bool& andando, Vector2 alvo);

    /// O sentido do vai e vem, e a altura em que o piloto que segue mora quando nada o chama.
    float mSentidoDoVaiVem = 1.0f;
    float mCasa = -1.0f;
    /// O rumo do piloto que desvia no quadro anterior, e o jeito dele no jogo de plataforma.
    int mRumoX = 0;
    int mRumoY = 0;
    int mAnda = 0;
    bool mPula = false;
};
