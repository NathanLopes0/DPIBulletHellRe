//
// O piloto da simulacao.
//

#include "PilotoDoAluno.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Player.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Motor/Game.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../Teachers/Boss.h"
#include "../Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Teachers/Bosses/Ferreira.h"
#include "../Teachers/Bosses/Hugo.h"

namespace {

    /// Os tiros do chefe que machucam este aluno, perto dele: o que o piloto que desvia olha.
    std::vector<Simulacao::Ameaca> AmeacasPerto(const Player& aluno, Battle* batalha) {
        std::vector<Simulacao::Ameaca> ameacas;
        ProjectileManager* gerente = batalha ? batalha->GerenteDeProjeteis() : nullptr;
        if (!gerente) return ameacas;
        const Vector2 eu = aluno.GetPosition();
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
        return ameacas;
    }

    /// Com a camera de eixos invertidos (a do Lucas Ferreira), o piloto pede o contrario: e quem ja
    /// se acostumou. A suavizacao ele nao compensa - escorrega como todo mundo.
    void InverterNaCamera(Player& aluno, RigidBodyComponent& corpo) {
        const auto* b = dynamic_cast<const Battle*>(aluno.GetScene());
        const auto* f = b ? dynamic_cast<const Ferreira*>(b->Chefe()) : nullptr;
        if (f && aluno.NaCamera() && f->CameraInvertida()) corpo.SetVelocity(corpo.GetVelocity() * -1.0f);
    }
}

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
        // No jogo de plataforma, os dois andam pelo chao rumo ao chefe: o que segue so anda (e e
        // pego pelo que vem); o que desvia pula.
        if (const auto chao = aluno.Chao()) {
            Vector2 alvo = aluno.GetPosition();
            if (const auto b = dynamic_cast<Battle*>(aluno.GetScene())) {
                if (const auto chefe = b->Chefe()) alvo = chefe->GetPosition();
            }
            if (piloto == Simulacao::Piloto::Esquiva) return Saltar(aluno, corpo, andando, alvo);
            const Vector2 t = Plataforma::AoLongo(*chao);
            const int d = Simulacao::Direcao(Vector2::Dot(aluno.GetPosition(), t), Vector2::Dot(alvo, t));
            corpo.SetVelocity(t * (static_cast<float>(d) * aluno.VelocidadeDeAndar()));
            andando = d != 0;
            return true;
        }
        if (mCasa < 0.0f) mCasa = aluno.GetPosition().y;
        float alvo = aluno.GetPosition().x, alvoY = mCasa;
        if (const auto b = dynamic_cast<Battle*>(aluno.GetScene())) {
            if (const auto chefe = b->Chefe()) alvo = chefe->GetPosition().x;
            if (auto* hugo = dynamic_cast<Hugo*>(b->Chefe())) {
                if (auto o = hugo->ObjetivoAtual()) alvo = o->x;
                if (auto o = hugo->LugarDeIr()) { alvo = o->x; alvoY = o->y; }
            }
        }
        if (piloto == Simulacao::Piloto::Esquiva) {
            Desviar(aluno, corpo, andando, alvo, alvoY);
            InverterNaCamera(aluno, corpo);
            return true;
        }
        const float v = static_cast<float>(Simulacao::Direcao(aluno.GetPosition().x, alvo)) * aluno.VelocidadeDeAndar();
        const float vy = static_cast<float>(Simulacao::Direcao(aluno.GetPosition().y, alvoY)) * aluno.VelocidadeDeAndar();
        corpo.SetVelocity(Vector2(v, vy));
        InverterNaCamera(aluno, corpo);
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
    const std::vector<Simulacao::Ameaca> ameacas = AmeacasPerto(aluno, batalha);
    Simulacao::Faixa faixa{eu.x - 1.0f, eu.y - 1.0f, eu.x + 1.0f, eu.y + 1.0f};
    if (batalha) {
        const SDL_FRect c = batalha->GetPlayfieldBounds();
        // Nao sobe ate o professor: do terco de cima para baixo.
        faixa = Simulacao::Faixa{c.x + 16.0f, c.y + c.h * 0.3f, c.x + c.w - 16.0f, c.y + c.h - 16.0f};
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

bool PilotoDoAluno::Saltar(Player& aluno, RigidBodyComponent& corpo, bool& andando, const Vector2 alvo) {
    const Plataforma::Lado l = *aluno.Chao();
    const Vector2 g = Plataforma::Direcao(l), t = Plataforma::AoLongo(l);
    auto* batalha = dynamic_cast<Battle*>(aluno.GetScene());
    const Vector2 eu = aluno.GetPosition();
    // Onde o centro dele pode ir: o campo menos um desenho de cada lado (Player::PrenderNoCampo).
    Simulacao::Chao chao{Vector2::Dot(eu, g), Vector2::Dot(eu, t) - 1.0f, Vector2::Dot(eu, t) + 1.0f};
    if (batalha) {
        const SDL_FRect c = batalha->GetPlayfieldBounds();
        const auto* desenho = aluno.GetComponent<DrawAnimatedComponent>();
        const float w = desenho ? static_cast<float>(desenho->GetSpriteWidth()) : 64.0f;
        const float h = desenho ? static_cast<float>(desenho->GetSpriteHeight()) : 64.0f;
        const Vector2 menor(c.x + w, c.y + h), maior(c.x + c.w - w, c.y + c.h - h);
        chao.limite = std::max(Vector2::Dot(menor, g), Vector2::Dot(maior, g));
        chao.de = std::min(Vector2::Dot(menor, t), Vector2::Dot(maior, t));
        chao.ate = std::max(Vector2::Dot(menor, t), Vector2::Dot(maior, t));
    }
    float raio = 6.0f;
    if (const auto* colisor = aluno.GetComponent<CircleColliderComponent>()) raio = colisor->GetRadius();
    const float rapidez = aluno.VelocidadeDeAndar();
    const Simulacao::Salto s = Simulacao::Saltar(eu, Vector2::Dot(corpo.GetVelocity(), g), aluno.NoChao(), l, chao, raio, rapidez,
                                                 AmeacasPerto(aluno, batalha), Vector2::Dot(alvo, t),
                                                 Simulacao::Salto{mAnda, mPula});
    mAnda = s.anda;
    mPula = s.pula;
    // A velocidade que ele pede e a intencao: o Player a le como o manche do jogo de plataforma.
    corpo.SetVelocity(t * (static_cast<float>(s.anda) * rapidez) + g * (s.pula ? -rapidez : 0.0f));
    andando = s.anda != 0;
    return true;
}
