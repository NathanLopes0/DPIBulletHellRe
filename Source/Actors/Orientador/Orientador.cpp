//
// O orientador do TCC, em campo.
//

#include "Orientador.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "../../Escolta.h"
#include "../../Math.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Components/DrawComponents/DrawSpriteComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Scenes/Battle/ProjectileManager.h"

Orientador::Orientador(Scene* scene, std::string professor, const std::string& retrato)
    : Actor(scene),
      mProfessor(std::move(professor)),
      mMateria(Golpe::DoProfessor(mProfessor))
{
    SetScale(kEscala);
    AddComponent<DrawSpriteComponent>(retrato, kOrdemDeDesenho);
}

Battle* Orientador::Batalha() {
    return dynamic_cast<Battle*>(GetScene());
}

Vector2 Orientador::PostoAgora() {
    Battle* batalha = Batalha();
    if (!batalha) return GetPosition();

    std::vector<Vector2> alunos;
    for (const Player* p : batalha->Jogadores()) {
        if (p) alunos.push_back(p->GetPosition());
    }

    const SDL_FRect campo = batalha->GetPlayfieldBounds();
    const float esquerda = campo.x;
    const float direita = campo.x + campo.w;
    if (alunos.size() == 1) mLado = Escolta::Lado(alunos.front().x, mLado, esquerda, direita);
    return Escolta::Posto(alunos, mLado, esquerda, direita);
}

void Orientador::Pousar() {
    mPouso = PostoAgora();
    SetPosition(mPouso);
}

void Orientador::OnUpdate(const float deltaTime) {

    Battle* batalha = Batalha();
    if (!batalha) return;

    // ELE VOA SEMPRE, MAS SO ATIRA NA LUTA: na tela de carregamento o chefe
    // ainda nao comecou, e na de fim a nota ja foi fechada.
    mTempo += deltaTime;
    mPouso = Escolta::Seguir(mPouso, PostoAgora(), deltaTime);
    const float balanco = kBalanco * std::sin(mTempo * kBalancoPorSegundo * 2.0f * Math::Pi);
    SetPosition(mPouso + Vector2(0.0f, balanco));

    Guiar(*batalha, deltaTime);
    if (!batalha->EmCombate()) return;
    Golpear(*batalha, deltaTime);
}

void Orientador::Golpear(Battle& batalha, const float deltaTime) {

    const Boss* chefe = batalha.GetBoss();
    if (!chefe) return;
    if (!mSorteado) {
        mSorteado = true;
        mSorteio = Variacao::SorteioDa(batalha.SementeDaBatalha(), "orientador");
    }
    if (mEspera > 0.0f) {
        mEspera -= deltaTime;
        return;
    }
    const Vector2 alvo = chefe->GetPosition();

    // O OBSERVER: um tiro a cada kNotificacoes do aluno - da media, em dupla.
    if (mMateria == Golpe::Materia::MariaLucia) {
        int doAluno = 0, alunos = 0;
        for (const Player* p : batalha.Jogadores()) {
            if (!p) continue;
            doAluno += p->Disparos();
            ++alunos;
        }
        const int devidos = Golpe::Notificacoes(doAluno / std::max(1, alunos), mNotificacoes);
        for (int i = 0; i < devidos; ++i) {
            Disparar(batalha, Golpe::Tiro{}, GetPosition(), Golpe::Rumo(GetPosition(), alvo, 0.0f));
        }
        mNotificacoes += devidos;
        return;
    }

    // A RODADA: quando acaba, vem a proxima, e cada tiro sai na hora dele.
    mNaRodada += deltaTime;
    if (mRodada < 0 || mNaRodada >= mDaVez.duracao) {
        if (mRodada >= 0) mNaRodada -= mDaVez.duracao;
        mDaVez = Golpe::DaVez(mMateria, ++mRodada, mSorteio);
        mProximoTiro = 0;
    }
    while (mProximoTiro < mDaVez.tiros.size() && mNaRodada >= mDaVez.tiros[mProximoTiro].quando) {
        const Golpe::Tiro& tiro = mDaVez.tiros[mProximoTiro++];
        const Vector2 de = GetPosition() + Vector2(tiro.dx, tiro.dy);
        const Vector2 direcao = Golpe::Rumo(tiro.paralelo ? GetPosition() : de, alvo, tiro.desvio);
        Disparar(batalha, tiro, de, direcao);
    }
}

void Orientador::Disparar(Battle& batalha, const Golpe::Tiro& tiro, const Vector2& de, const Vector2& direcao) {

    ProjectileManager* tiros = batalha.GetProjectileManager();
    if (!tiros) return;

    auto p = std::make_unique<PlayerProjectile>(GetScene(), this);
    p->ActivateProjectile();   // nasce onde o orientador esta
    p->SetPosition(de);
    if (auto* rb = p->GetComponent<RigidBodyComponent>()) {
        rb->SetVelocity(tiro.espera > 0.0f ? Vector2::Zero : direcao * tiro.velocidade);
    }
    if (tiro.Guiado()) {
        Guiado g;
        g.id = p->Id();
        g.tiro = tiro;
        g.partiu = tiro.espera <= 0.0f;
        g.deLado = Vector2(-direcao.y, direcao.x);
        mGuiados.push_back(g);
    }
    tiros->AddPlayerProjectile(std::move(p));
    ++mDisparos;
}

void Orientador::Guiar(Battle& batalha, const float deltaTime) {

    if (mGuiados.empty()) return;
    ProjectileManager* tiros = batalha.GetProjectileManager();
    const Boss* chefe = batalha.GetBoss();
    if (!tiros || !chefe) {
        mGuiados.clear();
        return;
    }
    const Vector2 alvo = chefe->GetPosition();

    // O MANAGER APAGA O TIRO QUE MORRE: cada um e procurado pelo numero, a
    // cada quadro, e o que nao esta mais la sai da lista.
    std::unordered_map<std::uint64_t, PlayerProjectile*> meus;
    for (const auto& p : tiros->GetPlayerProjectiles()) {
        if (p && p->mOwner == this && p->GetState() == ActorState::Active) meus[p->Id()] = p.get();
    }

    struct Filho {
        Golpe::Tiro tiro;
        Vector2 de;
        Vector2 direcao;
    };
    std::vector<Filho> filhos;

    for (auto it = mGuiados.begin(); it != mGuiados.end();) {
        const auto achado = meus.find(it->id);
        if (achado == meus.end()) {
            it = mGuiados.erase(it);
            continue;
        }
        PlayerProjectile* p = achado->second;
        auto* rb = p->GetComponent<RigidBodyComponent>();
        Guiado& g = *it;
        const Golpe::Tiro& t = g.tiro;
        g.idade += deltaTime;

        // A ESPERA: parado junto do orientador - na pilha, na linha da
        // transacao -, e depois parte mirado no chefe.
        if (!g.partiu) {
            if (g.idade < t.espera) {
                p->SetPosition(GetPosition() + Vector2(t.dx, t.dy));
                if (rb) rb->SetVelocity(Vector2::Zero);
                ++it;
                continue;
            }
            g.partiu = true;
            const Vector2 direcao = Golpe::Rumo(p->GetPosition(), alvo, 0.0f);
            if (rb) rb->SetVelocity(direcao * t.velocidade);
            g.deLado = Vector2(-direcao.y, direcao.x);
        }
        const float voo = g.idade - std::max(0.0f, t.espera);

        // O PONTEIRO: curva na direcao do chefe.
        if (t.curva > 0.0f && voo < kCurvaAte && rb) {
            rb->SetVelocity(Golpe::Curvar(rb->GetVelocity(), alvo - p->GetPosition(), t.curva, deltaTime));
        }
        // O FETCH: a resposta volta, e aponta para o chefe de uma vez.
        if (t.reMira > 0.0f && !g.remirou && voo >= t.reMira && rb) {
            g.remirou = true;
            rb->SetVelocity(Golpe::Rumo(p->GetPosition(), alvo, 0.0f) * rb->GetVelocity().Length());
        }
        // O GRADIENTE: o desvio de lado, que se apaga.
        if (t.onda != 0.0f) {
            const float agora = Golpe::Oscilacao(voo, t.onda);
            p->SetPosition(p->GetPosition() + g.deLado * (agora - g.deslocado));
            g.deslocado = agora;
        }
        // A GRAMATICA: S -> S S. O tiro some e nascem dois, abrindo.
        if (t.divide > 0.0f && voo >= t.divide) {
            for (const float lado : {-0.5f, 0.5f}) {
                Golpe::Tiro filho;
                filho.velocidade = t.velocidade;
                if (t.geracoes > 0) {
                    filho.divide = t.divide;
                    filho.geracoes = t.geracoes - 1;
                }
                filhos.push_back({filho, p->GetPosition(),
                                  Golpe::Rumo(p->GetPosition(), alvo, lado * Golpe::kAberturaDosFilhos)});
            }
            p->DeactivateProjectile();
            it = mGuiados.erase(it);
            continue;
        }
        ++it;
    }

    // DEPOIS DO LACO: nascer agora mexeria no vetor do manager e na lista.
    for (const Filho& f : filhos) Disparar(batalha, f.tiro, f.de, f.direcao);
}
