//
// O piloto da simulacao.
//

#include "PilotoDoAluno.h"

#include "Player.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Motor/Game.h"
#include "../../Scenes/Battle/Battle.h"
#include "../Teachers/Boss.h"
#include "../Teachers/Bosses/Hugo.h"

bool PilotoDoAluno::Dirigir(Player& aluno, RigidBodyComponent& corpo, bool& andando) {
    const Simulacao::Piloto piloto = aluno.GetScene()->GetGame()->ConfiguracaoDaSimulacao().piloto;
    if (piloto == Simulacao::Piloto::Nenhum) return false;

    // O vai e vem varre a largura sem mirar: a mira dele e ruim de um jeito parecido com o de gente.
    if (piloto == Simulacao::Piloto::VaiVem) {
        mSentidoDoVaiVem = Simulacao::SentidoDoVaiVem(aluno.GetPosition().x, mSentidoDoVaiVem);
        corpo.SetVelocity(Vector2(mSentidoDoVaiVem * aluno.VelocidadeDeAndar(), 0));
        andando = true;
        return true;
    }

    // O que segue casa o x com o do chefe; no TCC, com o Hugo blindado, vai ao alvo do objetivo e ao
    // lugar que a etapa pede (em x e em y).
    if (piloto == Simulacao::Piloto::Segue) {
        if (mCasa < 0.0f) mCasa = aluno.GetPosition().y;
        float alvo = aluno.GetPosition().x, alvoY = mCasa;
        if (const auto b = dynamic_cast<Battle*>(aluno.GetScene())) {
            if (const auto chefe = b->Chefe()) alvo = chefe->GetPosition().x;
            if (auto* hugo = dynamic_cast<Hugo*>(b->Chefe())) {
                if (auto o = hugo->ObjetivoAtual()) alvo = o->x;
                if (auto o = hugo->LugarDeIr()) { alvo = o->x; alvoY = o->y; }
            }
        }
        const float v = static_cast<float>(Simulacao::Direcao(aluno.GetPosition().x, alvo)) * aluno.VelocidadeDeAndar();
        const float vy = static_cast<float>(Simulacao::Direcao(aluno.GetPosition().y, alvoY)) * aluno.VelocidadeDeAndar();
        corpo.SetVelocity(Vector2(v, vy));
        andando = v != 0.0f;
        return true;
    }

    // Parado.
    corpo.SetVelocity(Vector2(0, 0));
    andando = false;
    return true;
}
