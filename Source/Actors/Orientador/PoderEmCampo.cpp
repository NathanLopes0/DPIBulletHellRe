//
// O poder do orientador em campo: um objeto por poder.
//

#include "PoderEmCampo.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_set>
#include <vector>

#include "Orientador.h"
#include "../Player/PlayerProjectile.h"
#include "../Teachers/Boss.h"
#include "../Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Scenes/Battle/ProjectileManager.h"

namespace {

    /// As folhas das figuras dos poderes, cada uma da batalha do professor.
    constexpr const char* kFolhaDaCapivara = "Teachers/Projectiles/DPIBHSallesCapivarona";
    constexpr const char* kFolhaDosAgentes = "Teachers/Projectiles/DPIBHJulioToken";
    constexpr const char* kFolhaDaBala = "Teachers/Projectiles/DPIBHMariaLuciaRequisito";
    constexpr int kQuadroDaBala = 44;

    /// O tiro do HLT: o tiro guardado ainda e o mesmo se a idade bate.
    constexpr float kFolgaDaIdade = 0.15f;

    /// As figuras de um poder (a capivara, os agentes) saem quando ele acaba.
    void Desfazer(std::vector<Actor*>& figuras) {
        for (Actor* f : figuras) if (f) f->SetState(ActorState::Destroy);
        figuras.clear();
    }

    // ---- ANDRE: for (;;) ------------------------------------------------------

    class Rajada final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2&, const Vector2&) override {
            // A rajada sai pelo Atualizar, um tiro por intervalo.
            mFalta = o.Descricao().tiros;
            mProximo = o.Tempo();
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            Boss* chefe = batalha.GetBoss();
            if (!ativo || !chefe) return;
            while (mFalta > 0 && o.Tempo() >= mProximo) {
                --mFalta;
                mProximo += o.Descricao().intervalo;
                const float varre = 10.0f * std::sin(static_cast<float>(mFalta) * 0.7f);
                o.Disparar(batalha, Golpe::Tiro{}, o.GetPosition(),
                           Golpe::Rumo(o.GetPosition(), chefe->GetPosition(), varre));
            }
        }
    private:
        int mFalta = 0;
        float mProximo = 0.0f;
    };

    // ---- SALLES: a capivara ---------------------------------------------------

    class Capivara final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2& aluno, const Vector2&) override {
            Actor* capivara = o.Figura(kFolhaDaCapivara, {0, 1, 2, 3}, 1.2f);
            if (auto* anim = capivara->GetComponent<DrawAnimatedComponent>()) anim->AddAnimation("Volta", {4, 5, 6, 7});
            capivara->SetPosition(Vector2(aluno.x, aluno.y + 10.0f));
            mFiguras.push_back(capivara);
        }
        void Atualizar(Orientador& o, Battle& batalha, const float deltaTime, const bool ativo) override {
            if (!ativo) Desfazer(mFiguras);
            if (!ativo || !batalha.GetBoss() || mFiguras.empty() || !mFiguras.front()) return;
            // A CAPIVARA ANDA EMBAIXO DO ALUNO e come o que encosta nela.
            const Vector2 aluno = o.PosicaoDoAluno();
            Actor* c = mFiguras.front();
            const float dx = aluno.x - c->GetPosition().x;
            const float passo = std::clamp(dx, -Poder::kRapidezDaCapivara * deltaTime,
                                           Poder::kRapidezDaCapivara * deltaTime);
            c->SetPosition(Vector2(c->GetPosition().x + passo, aluno.y + 10.0f));
            if (auto* anim = c->GetComponent<DrawAnimatedComponent>()) {
                const char* rumo = dx < -2.0f ? "Volta" : "Anda";
                if (anim->GetAnimation() != rumo) anim->SetAnimation(rumo);
            }
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                if (Poder::NoAlcance(c->GetPosition(), p->GetPosition(), o.Descricao().raio)) p->Kill();
            }
        }
    private:
        std::vector<Actor*> mFiguras;
    };

    // ---- RICARDO: HLT ---------------------------------------------------------

    class Congelar final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            mCongelados.clear();
            mJaCongelados.clear();
            // O CHEFE PARA JUNTO (Boss::Congelar): sem isso ele seguia atirando
            // parado, e o que nascia congelava em cima dele - e caia tudo de
            // uma vez quando o clock voltava.
            for (Boss* corpo : batalha.Corpos()) {
                if (corpo) corpo->Congelar(o.Descricao().duracao);
            }
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            // FIM DO EFEITO: o que congelou volta a andar, como estava.
            if (!ativo && !mCongelados.empty()) {
                for (const Congelado& c : mCongelados) {
                    if (!Mesmo(o, c)) continue;
                    if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(c.velocidade);
                }
                mCongelados.clear();
                mJaCongelados.clear();
            }
            if (!ativo || !batalha.GetBoss()) return;
            // HLT: todo tiro solto para onde esta - os que nascem tambem.
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                if (mJaCongelados.insert(p).second) {
                    auto* rb = p->GetComponent<RigidBodyComponent>();
                    mCongelados.push_back({p, p->GetPosition(), rb ? rb->GetVelocity() : Vector2::Zero,
                                           p->TempoDeVida(), o.Tempo()});
                }
            }
            for (const Congelado& c : mCongelados) {
                if (!Mesmo(o, c)) continue;
                c.p->SetPosition(c.posicao);
                if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            }
        }
    private:
        /// O que congelou, e como estava.
        struct Congelado {
            BossProjectile* p = nullptr;
            Vector2 posicao;
            Vector2 velocidade;
            float idade = 0.0f;
            float desde = 0.0f;
        };
        /// O POOL RECICLA O PROJETIL: o ponteiro guardado so e o mesmo tiro
        /// se ainda esta ativo e a idade dele bate com a de quando congelou.
        static bool Mesmo(const Orientador& o, const Congelado& c) {
            if (!c.p || c.p->GetState() != ActorState::Active) return false;
            return std::fabs(c.p->TempoDeVida() - (c.idade + (o.Tempo() - c.desde))) <= kFolgaDaIdade;
        }
        std::vector<Congelado> mCongelados;
        std::unordered_set<const BossProjectile*> mJaCongelados;
    };

    // ---- THIAGO: ROLLBACK -----------------------------------------------------

    class Rollback final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            // TODO TIRO DO HUGO VOLTA por onde veio.
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(rb->GetVelocity() * -1.0f);
            }
        }
    };

    // ---- LUIZINHO: free() -----------------------------------------------------

    class Liberar final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2& aluno, const Vector2&) override {
            // free(): o que esta perto do aluno some.
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                if (Poder::NoAlcance(aluno, p->GetPosition(), o.Descricao().raio)) p->Kill();
            }
            o.AbrirAnel();
        }
    };

    // ---- VLADIMIR: o automato rejeita -----------------------------------------

    class Converter final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2& aluno, const Vector2& alvo) override {
            // O AUTOMATO REJEITA o que esta perto, e cada um volta como tiro nosso.
            const int quadroDele = Poder::QuadroDaMateria(Golpe::Materia::Vladimir);
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                if (!Poder::NoAlcance(aluno, p->GetPosition(), o.Descricao().raio)) continue;
                const Vector2 de = p->GetPosition();
                p->Kill();
                o.Disparar(batalha, Golpe::Tiro{}, de, Golpe::Rumo(de, alvo, 0.0f), Poder::kFolhaDasMaterias,
                           quadroDele, 1.0f);
            }
            o.AbrirAnel();
        }
    };

    // ---- JULIO: agentes autonomos ---------------------------------------------

    class Agentes final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2&, const Vector2&) override {
            // OS AGENTES voam em volta do aluno enquanto o poder dura - e o que
            // mostra que os tiros estao cacando.
            mInicio = o.Tempo();
            for (int k = 0; k < o.Descricao().tiros; ++k) {
                mFiguras.push_back(o.Figura(kFolhaDosAgentes, {8, 9, 10, 11}, 1.3f));
            }
        }
        void Atualizar(Orientador& o, Battle& batalha, const float deltaTime, const bool ativo) override {
            if (!ativo) Desfazer(mFiguras);
            Boss* chefe = batalha.GetBoss();
            if (!ativo || !chefe) return;
            const Vector2 aluno = o.PosicaoDoAluno();
            const Vector2 alvo = chefe->GetPosition();
            const int n = static_cast<int>(mFiguras.size());
            for (int k = 0; k < n; ++k) {
                if (mFiguras[static_cast<size_t>(k)]) {
                    mFiguras[static_cast<size_t>(k)]->SetPosition(
                        Poder::Agente(aluno, k, n, o.Tempo() - mInicio, o.Descricao().raio));
                }
            }
            // ...e todo tiro do aluno e do orientador CACA UM ALVO: com o Hugo
            // blindado, o alvo do objetivo mais perto; sem alvo, o chefe. O
            // tiro curva (Poder::Guiar), e nao teleporta.
            const std::vector<Vector2> destrutiveis = chefe->AlvosDestrutiveis();
            ProjectileManager* tiros = batalha.GetProjectileManager();
            if (!tiros) return;
            for (const auto& p : tiros->GetPlayerProjectiles()) {
                if (!p || p->GetState() != ActorState::Active || p->Murchando()) continue;
                auto* rb = p->GetComponent<RigidBodyComponent>();
                if (!rb) continue;
                const Vector2 de = p->GetPosition();
                Vector2 caca = alvo;
                float perto = (alvo - de).LengthSq();
                if (!destrutiveis.empty()) perto = std::numeric_limits<float>::max();
                for (const Vector2& d : destrutiveis) {
                    if ((d - de).LengthSq() < perto) { perto = (d - de).LengthSq(); caca = d; }
                }
                for (Boss* corpo : batalha.Corpos()) {
                    if (!destrutiveis.empty() || !corpo) continue;
                    if ((corpo->GetPosition() - de).LengthSq() < perto) {
                        perto = (corpo->GetPosition() - de).LengthSq();
                        caca = corpo->GetPosition();
                    }
                }
                rb->SetVelocity(Poder::Guiar(rb->GetVelocity(), de, caca, Poder::kGiroDosAgentes, deltaTime));
            }
        }
    private:
        float mInicio = 0.0f;
        std::vector<Actor*> mFiguras;
    };

    // ---- MARIA LUCIA: a bala de prata -----------------------------------------

    class BalaDePrata final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2& alvo) override {
            // A BALA DE PRATA curva ate o chefe e, chegando, quebra o escudo.
            Golpe::Tiro t;
            t.velocidade = 650.0f;
            t.curva = 400.0f;
            t.curvaAte = 3.0f;
            PlayerProjectile* bala = o.Disparar(batalha, t, o.GetPosition(), Golpe::Rumo(o.GetPosition(), alvo, 0.0f),
                                                kFolhaDaBala, kQuadroDaBala, 2.2f);
            mBala = bala ? bala->Id() : 0;
        }
        void Atualizar(Orientador&, Battle& batalha, float, bool) override {
            // A BALA NAO TEM DURACAO: ela vale ate chegar, entao a conferencia
            // roda com o efeito ativo ou nao.
            Boss* chefe = batalha.GetBoss();
            if (mBala == 0 || !chefe) return;
            bool achou = false;
            if (ProjectileManager* tiros = batalha.GetProjectileManager()) {
                for (const auto& p : tiros->GetPlayerProjectiles()) {
                    // A BALA BARRADA PELO ESCUDO murcha (PlayerProjectile::Murchar): ja acertou.
                    if (!p || p->Id() != mBala || p->GetState() != ActorState::Active || p->Murchando()) continue;
                    achou = true;
                    // PERTO DO CHEFE, o escudo quebra - uma vez.
                    if ((p->GetPosition() - chefe->GetPosition()).Length() < 90.0f) {
                        chefe->QuebrarEscudo();
                        mBala = 0;
                    }
                }
            }
            // ACERTOU ANTES DE CHEGAR PERTO (o colisor do chefe e grande): quebra.
            if (!achou && mBala != 0) {
                chefe->QuebrarEscudo();
                mBala = 0;
            }
        }
    private:
        std::uint64_t mBala = 0;
    };

    // ---- LUCAS: Promise.all() -------------------------------------------------

    class PromiseAll final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            // DOZE PROMESSAS em volta dele, pendentes, que resolvem juntas - e
            // VOLTAM (PlayerProjectile::FazerPromessa): o valor de uma promessa
            // e o que ela devolve. Na ida e na volta desfazem os tiros do Hugo
            // que encostam.
            const Poder::Descricao& d = o.Descricao();
            const int quadroDele = Poder::QuadroDaMateria(Golpe::Materia::Lucas);
            for (int k = 0; k < d.tiros; ++k) {
                const float a = 2.0f * Math::Pi * static_cast<float>(k) / static_cast<float>(d.tiros);
                Golpe::Tiro t;
                t.dx = Math::Cos(a) * d.raio;
                t.dy = Math::Sin(a) * d.raio;
                t.espera = d.duracao;
                if (PlayerProjectile* p = o.Disparar(batalha, t, o.GetPosition() + Vector2(t.dx, t.dy),
                                                     Vector2(0.0f, -1.0f), Poder::kFolhaDasMaterias,
                                                     quadroDele, 1.0f)) {
                    p->FazerPromessa();
                }
            }
        }
    };

    // ---- VITOR: socket() ------------------------------------------------------

    class Rede final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            // As paredes dos lados viram rede (Battle::AbrirRede).
            batalha.AbrirRede(o.Descricao().duracao);
        }
    };
}

std::unique_ptr<PoderEmCampo> PoderEmCampo::Do(const Poder::Tipo t) {
    switch (t) {
        case Poder::Tipo::Rajada:      return std::make_unique<Rajada>();
        case Poder::Tipo::Capivara:    return std::make_unique<Capivara>();
        case Poder::Tipo::Congelar:    return std::make_unique<Congelar>();
        case Poder::Tipo::Rollback:    return std::make_unique<Rollback>();
        case Poder::Tipo::Liberar:     return std::make_unique<Liberar>();
        case Poder::Tipo::Converter:   return std::make_unique<Converter>();
        case Poder::Tipo::Agentes:     return std::make_unique<Agentes>();
        case Poder::Tipo::BalaDePrata: return std::make_unique<BalaDePrata>();
        case Poder::Tipo::PromiseAll:  return std::make_unique<PromiseAll>();
        case Poder::Tipo::Rede:        return std::make_unique<Rede>();
        case Poder::Tipo::Nenhum:      break;
    }
    return nullptr;
}
