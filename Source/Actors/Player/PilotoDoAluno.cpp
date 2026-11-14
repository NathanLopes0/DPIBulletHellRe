//
// O piloto da simulacao.
//

#include "PilotoDoAluno.h"

#include <cmath>
#include <vector>

#include "Player.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Motor/Game.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../Teachers/Boss.h"
#include "../Teachers/Bosses/BossesProjectiles/BossProjectile.h"
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
    // lugar que a etapa pede (em x e em y). O que desvia quer o mesmo lugar, mas olha os tiros antes.
    if (piloto == Simulacao::Piloto::Segue || piloto == Simulacao::Piloto::Esquiva) {
        if (mCasa < 0.0f) mCasa = aluno.GetPosition().y;
        float alvo = aluno.GetPosition().x, alvoY = mCasa;
        if (const auto b = dynamic_cast<Battle*>(aluno.GetScene())) {
            if (const auto chefe = b->Chefe()) alvo = chefe->GetPosition().x;
            if (auto* hugo = dynamic_cast<Hugo*>(b->Chefe())) {
                if (auto o = hugo->ObjetivoAtual()) alvo = o->x;
                if (auto o = hugo->LugarDeIr()) { alvo = o->x; alvoY = o->y; }
            }
        }
        if (piloto == Simulacao::Piloto::Esquiva) return Desviar(aluno, corpo, andando, alvo, alvoY);
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

bool PilotoDoAluno::Desviar(Player& aluno, RigidBodyComponent& corpo, bool& andando, const float alvoX, const float alvoY) {
    auto* batalha = dynamic_cast<Battle*>(aluno.GetScene());
    const Vector2 eu = aluno.GetPosition();
    std::vector<Simulacao::Ameaca> ameacas;
    Simulacao::Faixa faixa{eu.x - 1.0f, eu.y - 1.0f, eu.x + 1.0f, eu.y + 1.0f};
    if (batalha) {
        const SDL_FRect c = batalha->GetPlayfieldBounds();
        // Nao sobe ate o professor: do terco de cima para baixo.
        faixa = Simulacao::Faixa{c.x + 16.0f, c.y + c.h * 0.3f, c.x + c.w - 16.0f, c.y + c.h - 16.0f};
        if (auto* gerente = batalha->GerenteDeProjeteis()) {
            for (const auto& p : gerente->GetBossProjectiles()) {
                if (!p || p->GetState() != ActorState::Active) continue;
                // O que nao machuca: o enfeite, o abrigo e o tiro da classe do aluno.
                if (p->Enfeite() || p->Abrigo()) continue;
                if (p->Classe() >= 0 && p->Classe() == aluno.Classe()) continue;
                const Vector2 onde = p->GetPosition();
                if ((onde - eu).Length() > Simulacao::kAlcanceDoOlhar) continue;
                Simulacao::Ameaca a;
                a.x = onde.x;
                a.y = onde.y;
                if (const auto* rb = p->GetComponent<RigidBodyComponent>()) {
                    a.vx = rb->GetVelocity().x;
                    a.vy = rb->GetVelocity().y;
                }
                if (const auto* colisor = p->GetComponent<CircleColliderComponent>()) a.raio = colisor->GetRadius();
                ameacas.push_back(a);
            }
        }
    }
    float raio = 6.0f;
    if (const auto* colisor = aluno.GetComponent<CircleColliderComponent>()) raio = colisor->GetRadius();
    const Simulacao::Rumo r = Simulacao::Desviar(eu.x, eu.y, raio, aluno.VelocidadeDeAndar(), ameacas, alvoX, alvoY, faixa,
                                                 Simulacao::Rumo{mRumoX, mRumoY});
    mRumoX = r.dx;
    mRumoY = r.dy;
    const float n = (r.dx != 0 && r.dy != 0) ? 0.70710678f : 1.0f;
    corpo.SetVelocity(Vector2(static_cast<float>(r.dx), static_cast<float>(r.dy)) * (n * aluno.VelocidadeDeAndar()));
    andando = r.dx != 0 || r.dy != 0;
    return true;
}
