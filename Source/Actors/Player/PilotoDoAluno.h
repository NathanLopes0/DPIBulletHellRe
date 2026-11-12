//
// O piloto da simulacao: quem joga no lugar do aluno quando o jogo roda sozinho.
//
#pragma once

class Player;
class RigidBodyComponent;

/// O piloto de Simulacao.h (--piloto segue|parado|vaivem), no lugar do manche. Mora fora do Player
/// para que o aluno nao precise conhecer o Hugo: e o piloto que sabe ir ao alvo do objetivo do TCC.
class PilotoDoAluno {
public:
    /// Dirige o aluno neste quadro: poe a velocidade no corpo e diz se ele anda. Devolve false quando
    /// a simulacao nao tem piloto, e o manche vale.
    bool Dirigir(Player& aluno, RigidBodyComponent& corpo, bool& andando);

private:
    /// O sentido do vai e vem, e a altura em que o piloto que segue mora quando nada o chama.
    float mSentidoDoVaiVem = 1.0f;
    float mCasa = -1.0f;
};
