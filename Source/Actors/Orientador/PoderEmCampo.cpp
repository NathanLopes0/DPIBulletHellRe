//
// O poder do orientador em campo: um objeto por poder.
//

#include "PoderEmCampo.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Orientador.h"
#include "../Player/Player.h"
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

    /// O tiro do hlt: o tiro guardado ainda e o mesmo se a idade bate.
    constexpr float kFolgaDaIdade = 0.15f;

    /// As figuras de um poder (a capivara, os agentes) saem quando ele acaba.
    void Desfazer(std::vector<Actor*>& figuras) {
        for (Actor* f : figuras) if (f) f->SetState(ActorState::Destroy);
        figuras.clear();
    }

    // ---- Andre: os lacos aninhados ----

    class Matriz final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2&, const Vector2&) override {
            mProxima = 0;
            mInicio = o.Tempo();
        }
        void Atualizar(Orientador& o, Battle& batalha, float, bool) override {
            // for (i) for (j): a matriz sai linha por linha, centrada no aluno - quem esta embaixo do
            // Hugo acerta a matriz inteira.
            const int total = o.Descricao().tiros;
            if (mProxima < 0 || mProxima >= total || !batalha.Chefe()) return;
            const Vector2 aluno = o.PosicaoDoAluno();
            while (mProxima < total) {
                const Poder::Celula c = Poder::CelulaDaMatriz(mProxima);
                if (o.Tempo() - mInicio < c.quando) break;
                ++mProxima;
                if (c.j == 0) {
                    o.Rotular("i = " + std::to_string(c.i) + ":  for (j)  " +
                              std::to_string(Poder::kColunasDaMatriz) + " tiros");
                }
                Golpe::Tiro tiro;
                tiro.velocidade = Poder::kVelocidadeDaMatriz;
                o.Disparar(batalha, tiro, Vector2(aluno.x + c.dx, aluno.y - kAcimaDoAluno), Vector2(0.0f, -1.0f));
            }
        }
    private:
        static constexpr float kAcimaDoAluno = 34.0f;
        int mProxima = -1;
        float mInicio = 0.0f;
    };

    // ---- Salles: a pilha ----

    class Pilha final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2& aluno, const Vector2&) override {
            Desfazer(mFiguras);
            Actor* capivara = o.Figura(kFolhaDaCapivara, {0, 1, 2, 3}, 1.2f);
            if (auto* anim = capivara->GetComponent<DrawAnimatedComponent>()) anim->AddAnimation("Volta", {4, 5, 6, 7});
            capivara->SetPosition(Vector2(aluno.x, aluno.y + 10.0f));
            mFiguras.push_back(capivara);
            mPilha = 0;
            mTransbordou = false;
        }
        void Atualizar(Orientador& o, Battle& batalha, const float deltaTime, const bool ativo) override {
            if (mFiguras.empty() || !mFiguras.front()) return;
            Boss* chefe = batalha.Chefe();
            if (!chefe) {
                Desfazer(mFiguras);
                return;
            }
            // A capivara anda embaixo do aluno.
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

            const int antes = mPilha;
            if (ativo) {
                // Push: o que encosta nela entra na pilha. Cheia, e stack overflow - ela devolve ja.
                for (BossProjectile* p : o.TirosSoltos(batalha)) {
                    if (mPilha >= Poder::kMaiorPilha) break;
                    if (!Poder::NoAlcance(c->GetPosition(), p->GetPosition(), o.Descricao().raio)) continue;
                    p->Kill();
                    ++mPilha;
                }
                if (mPilha >= Poder::kMaiorPilha && !mTransbordou) {
                    mTransbordou = true;
                    o.Rotular("stack overflow: pop()");
                } else if (mPilha != antes && !mTransbordou) {
                    o.Rotular("push(tiro)   pilha: " + std::to_string(mPilha));
                }
            }
            if (mPilha == 0) mTransbordou = false;
            if (mPilha > 0 && (!ativo || mTransbordou)) {
                // Pop: o ultimo que entrou sai primeiro, e vai no Hugo.
                if (o.Tempo() >= mProximoPop) {
                    mProximoPop = o.Tempo() + o.Descricao().intervalo;
                    const Vector2 topo = Topo(*c, mPilha - 1);
                    --mPilha;
                    o.Disparar(batalha, Golpe::Tiro{}, topo, Golpe::Rumo(topo, chefe->GetPosition(), 0.0f));
                    if (!ativo) o.Rotular("pop() no Hugo   pilha: " + std::to_string(mPilha));
                }
            } else if (!ativo) {
                Desfazer(mFiguras);
                return;
            }

            // A pilha, ao lado dela: um bloco por tiro.
            for (int b = 0; b < mPilha; ++b) {
                const Vector2 m = Topo(*c, b);
                const Vector2 a(m.x - kLarguraDoBloco / 2.0f, m.y - kAlturaDoBloco / 2.0f);
                const Vector2 z(m.x + kLarguraDoBloco / 2.0f, m.y + kAlturaDoBloco / 2.0f);
                o.Tracar(a, Vector2(z.x, a.y), kCorDaPilha);
                o.Tracar(Vector2(z.x, a.y), z, kCorDaPilha);
                o.Tracar(z, Vector2(a.x, z.y), kCorDaPilha);
                o.Tracar(Vector2(a.x, z.y), a, kCorDaPilha);
                o.Tracar(Vector2(a.x, m.y), Vector2(z.x, m.y), kCorDaPilha);
            }
        }
    private:
        static constexpr float kLarguraDoBloco = 24.0f;
        static constexpr float kAlturaDoBloco = 9.0f;
        static constexpr float kAoLado = 58.0f;
        static constexpr SDL_Color kCorDaPilha{255, 196, 92, 255};

        /// O meio do bloco 'b' da pilha (0 e o de baixo).
        static Vector2 Topo(const Actor& capivara, const int b) {
            return Vector2(capivara.GetPosition().x + kAoLado,
                           capivara.GetPosition().y + 6.0f - static_cast<float>(b) * (kAlturaDoBloco + 2.0f));
        }

        std::vector<Actor*> mFiguras;
        int mPilha = 0;
        bool mTransbordou = false;
        float mProximoPop = 0.0f;
    };

    // ---- Ricardo: hlt ----

    class Congelar final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            mCongelados.clear();
            mJaCongelados.clear();
            // O chefe para junto (Boss::Congelar).
            for (Boss* corpo : batalha.Corpos()) {
                if (corpo) corpo->Congelar(o.Descricao().duracao);
            }
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            if (!ativo && !mCongelados.empty()) {
                for (const Congelado& c : mCongelados) {
                    if (!Mesmo(o, c)) continue;
                    if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(c.velocidade);
                }
                mCongelados.clear();
                mJaCongelados.clear();
            }
            if (!ativo || !batalha.Chefe()) return;
            // Hlt: todo tiro solto para onde esta - os que nascem tambem.
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
        struct Congelado {
            BossProjectile* p = nullptr;
            Vector2 posicao;
            Vector2 velocidade;
            float idade = 0.0f;
            float desde = 0.0f;
        };
        /// O pool recicla o projetil: o ponteiro guardado so e o mesmo tiro se ainda esta ativo e a
        /// idade dele bate com a de quando congelou.
        static bool Mesmo(const Orientador& o, const Congelado& c) {
            if (!c.p || c.p->GetState() != ActorState::Active) return false;
            return std::fabs(c.p->TempoDeVida() - (c.idade + (o.Tempo() - c.desde))) <= kFolgaDaIdade;
        }
        std::vector<Congelado> mCongelados;
        std::unordered_set<const BossProjectile*> mJaCongelados;
    };

    // ---- Thiago: a transacao ----

    class Transacao final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            // BEGIN: a nota de agora e o ponto de volta. Se a fase que decide acabar antes do fim, a
            // transacao fecha la (Battle::FecharPendencias): a nota que decide ja e a certa.
            mMarca = batalha.MarcarNota();
            mAberta = true;
            o.Rotular("BEGIN   nota salva: " + Nota2(mMarca.nota));
            batalha.DeixarPendencia([this, &o, &batalha] { Fechar(o, batalha); });
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            if (!mAberta) return;
            if (!ativo) {
                batalha.DeixarPendencia(nullptr);
                Fechar(o, batalha);
                return;
            }
            // O bloco aberto: uma moldura tracejada em volta de cada aluno.
            for (const Player* aluno : batalha.Jogadores()) {
                if (!aluno) continue;
                const Vector2 p = aluno->GetPosition();
                const float fase = std::fmod(o.Tempo() * 40.0f, 2.0f * kTraco);
                for (float d = -kMeiaMoldura - fase; d < kMeiaMoldura; d += 2.0f * kTraco) {
                    const float a = std::max(-kMeiaMoldura, d), z = std::min(kMeiaMoldura, d + kTraco);
                    if (z <= a) continue;
                    o.Tracar(p + Vector2(a, -kMeiaMoldura), p + Vector2(z, -kMeiaMoldura), kCorDaTransacao);
                    o.Tracar(p + Vector2(a, kMeiaMoldura), p + Vector2(z, kMeiaMoldura), kCorDaTransacao);
                    o.Tracar(p + Vector2(-kMeiaMoldura, a), p + Vector2(-kMeiaMoldura, z), kCorDaTransacao);
                    o.Tracar(p + Vector2(kMeiaMoldura, a), p + Vector2(kMeiaMoldura, z), kCorDaTransacao);
                }
            }
        }
    private:
        static constexpr float kMeiaMoldura = 44.0f;
        static constexpr float kTraco = 8.0f;
        static constexpr SDL_Color kCorDaTransacao{120, 220, 255, 230};

        static std::string Nota2(const float nota) {
            char texto[16];
            std::snprintf(texto, sizeof(texto), "%.2f", static_cast<double>(nota));
            return texto;
        }

        /// O fim: ROLLBACK se a nota caiu (e os tiros do Hugo voltam por onde vieram), COMMIT se nao.
        void Fechar(Orientador& o, Battle& batalha) {
            if (!mAberta) return;
            mAberta = false;
            const float antes = batalha.NotaAtual();
            if (batalha.DesfazerNota(mMarca)) {
                o.Rotular("ROLLBACK: a nota voltou a " + Nota2(mMarca.nota));
                for (BossProjectile* p : o.TirosSoltos(batalha)) {
                    if (auto* rb = p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(rb->GetVelocity() * -1.0f);
                }
            } else {
                o.Rotular("COMMIT: a nota fica " + Nota2(antes));
            }
        }

        Nota::Conta::Marca mMarca;
        bool mAberta = false;
    };

    // ---- Luizinho: try / catch ----

    class TryCatch final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            mCapturas = 0;
            mPendentes.clear();
            batalha.CapturarAcertos(o.Descricao().duracao, [this](Player& aluno) {
                mPendentes.push_back(aluno.GetPosition());
            });
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            // Cada acerto capturado: o catch nao deixa a nota cair, e o finally responde com um leque.
            const Boss* chefe = batalha.Chefe();
            const int n = o.Descricao().tiros;
            for (const Vector2& de : mPendentes) {
                ++mCapturas;
                if (!chefe) continue;
                for (int k = 0; k < n; ++k) {
                    const float desvio = Poder::DesvioNoLeque(k, n, Poder::kAberturaDoFinally);
                    o.Disparar(batalha, Golpe::Tiro{}, de, Golpe::Rumo(de, chefe->GetPosition(), desvio),
                               Poder::kFolhaDasMaterias, Poder::QuadroDaMateria(Golpe::Materia::Luizinho), 1.0f);
                }
                o.Rotular("catch (acerto " + std::to_string(mCapturas) + ")   finally: leque");
            }
            mPendentes.clear();
            if (!ativo) return;
            // As chaves do bloco, em volta de cada aluno.
            for (const Player* aluno : batalha.Jogadores()) {
                if (!aluno) continue;
                const Vector2 p = aluno->GetPosition();
                for (const float lado : {-1.0f, 1.0f}) {
                    const float x = p.x + lado * kAoLado;
                    o.Tracar(Vector2(x - lado * 8.0f, p.y - kMeiaAltura), Vector2(x, p.y - kMeiaAltura + 8.0f), kCorDoTry);
                    o.Tracar(Vector2(x, p.y - kMeiaAltura + 8.0f), Vector2(x, p.y - 6.0f), kCorDoTry);
                    o.Tracar(Vector2(x, p.y - 6.0f), Vector2(x + lado * 7.0f, p.y), kCorDoTry);
                    o.Tracar(Vector2(x + lado * 7.0f, p.y), Vector2(x, p.y + 6.0f), kCorDoTry);
                    o.Tracar(Vector2(x, p.y + 6.0f), Vector2(x, p.y + kMeiaAltura - 8.0f), kCorDoTry);
                    o.Tracar(Vector2(x, p.y + kMeiaAltura - 8.0f), Vector2(x - lado * 8.0f, p.y + kMeiaAltura), kCorDoTry);
                }
            }
        }
    private:
        static constexpr float kAoLado = 40.0f;
        static constexpr float kMeiaAltura = 44.0f;
        static constexpr SDL_Color kCorDoTry{150, 235, 150, 240};

        int mCapturas = 0;
        std::vector<Vector2> mPendentes;
    };

    // ---- Vladimir: o automato rejeita ----

    class Converter final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2& aluno, const Vector2& alvo) override {
            // O automato rejeita o que esta perto, e cada um volta como tiro nosso.
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

    // ---- Arroyo: a busca binaria ----

    class BuscaBinaria final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2&, const Vector2&) override {
            mInicio = o.Tempo();
            mPasso = 0;
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            // A cada passo some a metade dos tiros do Hugo, a mais perto do aluno: n, n/2, n/4...
            if (!ativo || mPasso >= o.Descricao().tiros) return;
            if (o.Tempo() - mInicio < static_cast<float>(mPasso) * o.Descricao().intervalo) return;
            ++mPasso;
            std::vector<BossProjectile*> tiros = o.TirosSoltos(batalha);
            const Vector2 aluno = o.PosicaoDoAluno();
            std::sort(tiros.begin(), tiros.end(), [&](const BossProjectile* a, const BossProjectile* b) {
                return (a->GetPosition() - aluno).LengthSq() < (b->GetPosition() - aluno).LengthSq();
            });
            const int leva = Poder::LevaNoPasso(static_cast<int>(tiros.size()));
            for (int k = 0; k < leva; ++k) tiros[static_cast<size_t>(k)]->Kill();
            o.Rotular("passo " + std::to_string(mPasso) + ": " + std::to_string(tiros.size()) + " \xe2\x86\x92 " +
                      std::to_string(tiros.size() - static_cast<size_t>(leva)));
            o.AbrirAnel();
        }
    private:
        float mInicio = 0.0f;
        int mPasso = 0;
    };

    // ---- Daniel: as k-medias ----

    class KMedias final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            // Os tiros do Hugo em campo viram grupos, e cada grupo vira um tiro so, no centroide: o
            // mais perto dele fica, la, e os outros somem.
            std::vector<BossProjectile*> tiros = o.TirosSoltos(batalha);
            std::vector<Vector2> pontos;
            for (const BossProjectile* p : tiros) pontos.push_back(p->GetPosition());
            std::vector<int> grupos;
            const std::vector<Vector2> centros = Poder::KMedias(pontos, o.Descricao().tiros, grupos);
            std::vector<int> fica(centros.size(), -1);
            std::vector<float> perto(centros.size(), std::numeric_limits<float>::max());
            for (size_t k = 0; k < tiros.size(); ++k) {
                const int g = grupos[k];
                if (g < 0) continue;
                const float d = (pontos[k] - centros[static_cast<size_t>(g)]).LengthSq();
                if (d < perto[static_cast<size_t>(g)]) {
                    perto[static_cast<size_t>(g)] = d;
                    fica[static_cast<size_t>(g)] = static_cast<int>(k);
                }
            }
            for (size_t k = 0; k < tiros.size(); ++k) {
                const int g = grupos[k];
                if (g >= 0 && fica[static_cast<size_t>(g)] == static_cast<int>(k)) tiros[k]->SetPosition(centros[static_cast<size_t>(g)]);
                else tiros[k]->Kill();
            }
            // \xc3\xa9 e o e agudo, \xe2\x86\x92 a seta.
            o.Rotular("k-m\xc3\xa9" "dias: " + std::to_string(tiros.size()) + " tiros \xe2\x86\x92 " +
                      std::to_string(centros.size()));
            o.AbrirAnel();
        }
    };

    // ---- Goulart: o Round Robin ----

    class Rodizio final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2&, const Vector2&) override {
            mInicio = o.Tempo();
            mTiros.clear();
            mVistos.clear();
            mProximoGrupo = 0;
            mAnunciado = -1;
        }
        void Atualizar(Orientador& o, Battle& batalha, float, const bool ativo) override {
            mTiros.erase(std::remove_if(mTiros.begin(), mTiros.end(), [&o](const Tiro& c) { return !Mesmo(o, c); }),
                         mTiros.end());
            if (!ativo) {
                // Acabou o rodizio: quem esperava a vez volta a andar.
                for (Tiro& c : mTiros) {
                    if (c.parado) if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(c.velocidade);
                }
                mTiros.clear();
                return;
            }
            // Cada tiro novo do Hugo entra num grupo, em rodizio.
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                if (!mVistos.insert(p).second) continue;
                auto* rb = p->GetComponent<RigidBodyComponent>();
                mTiros.push_back({p, mProximoGrupo++ % Poder::kGruposDoRodizio, p->GetPosition(),
                                  rb ? rb->GetVelocity() : Vector2::Zero, false, p->TempoDeVida(), o.Tempo()});
            }
            // So o grupo da vez anda; os outros esperam onde estao.
            const int daVez = Poder::GrupoDaVez(o.Tempo() - mInicio);
            for (Tiro& c : mTiros) {
                auto* rb = c.p->GetComponent<RigidBodyComponent>();
                if (c.grupo == daVez) {
                    if (c.parado && rb) rb->SetVelocity(c.velocidade);
                    c.parado = false;
                    continue;
                }
                if (!c.parado) {
                    c.parado = true;
                    c.posicao = c.p->GetPosition();
                    if (rb) c.velocidade = rb->GetVelocity();
                }
                c.p->SetPosition(c.posicao);
                if (rb) rb->SetVelocity(Vector2::Zero);
            }
            if (daVez != mAnunciado) {
                mAnunciado = daVez;
                o.Rotular("quantum: o grupo " + std::to_string(daVez + 1) + " de " +
                          std::to_string(Poder::kGruposDoRodizio) + " anda");
            }
        }
    private:
        struct Tiro {
            BossProjectile* p = nullptr;
            int grupo = 0;
            Vector2 posicao;
            Vector2 velocidade;
            bool parado = false;
            float idade = 0.0f;
            float desde = 0.0f;
        };
        /// O pool recicla o projetil: o guardado so e o mesmo tiro se esta ativo e a idade bate.
        static bool Mesmo(const Orientador& o, const Tiro& c) {
            if (!c.p || c.p->GetState() != ActorState::Active) return false;
            return std::fabs(c.p->TempoDeVida() - (c.idade + (o.Tempo() - c.desde))) <= kFolgaDaIdade;
        }
        float mInicio = 0.0f;
        int mProximoGrupo = 0;
        int mAnunciado = -1;
        std::vector<Tiro> mTiros;
        std::unordered_set<const BossProjectile*> mVistos;
    };

    // ---- Julio: agentes autonomos ----

    class Agentes final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle&, const Vector2&, const Vector2&) override {
            // Os agentes voam em volta do aluno enquanto o poder dura.
            mInicio = o.Tempo();
            for (int k = 0; k < o.Descricao().tiros; ++k) {
                mFiguras.push_back(o.Figura(kFolhaDosAgentes, {8, 9, 10, 11}, 1.3f));
            }
        }
        void Atualizar(Orientador& o, Battle& batalha, const float deltaTime, const bool ativo) override {
            if (!ativo) Desfazer(mFiguras);
            Boss* chefe = batalha.Chefe();
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
            // ...e todo tiro do aluno e do orientador caca um alvo.
            const std::vector<Vector2> destrutiveis = chefe->AlvosDestrutiveis();
            ProjectileManager* tiros = batalha.GerenteDeProjeteis();
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

    // ---- Maria Lucia: a bala de prata ----

    class BalaDePrata final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2& alvo) override {
            // A bala de prata curva ate o chefe e, chegando, quebra o escudo.
            Golpe::Tiro t;
            t.velocidade = 650.0f;
            t.curva = 400.0f;
            t.curvaAte = 3.0f;
            PlayerProjectile* bala = o.Disparar(batalha, t, o.GetPosition(), Golpe::Rumo(o.GetPosition(), alvo, 0.0f),
                                                kFolhaDaBala, kQuadroDaBala, 2.2f);
            mBala = bala ? bala->Id() : 0;
        }
        void Atualizar(Orientador&, Battle& batalha, float, bool) override {
            Boss* chefe = batalha.Chefe();
            if (mBala == 0 || !chefe) return;
            bool achou = false;
            if (ProjectileManager* tiros = batalha.GerenteDeProjeteis()) {
                for (const auto& p : tiros->GetPlayerProjectiles()) {
                    // A bala barrada pelo escudo murcha (PlayerProjectile::Murchar): ja acertou.
                    if (!p || p->Id() != mBala || p->GetState() != ActorState::Active || p->Murchando()) continue;
                    achou = true;
                    // Perto do chefe, o escudo quebra - uma vez.
                    if ((p->GetPosition() - chefe->GetPosition()).Length() < 90.0f) {
                        chefe->QuebrarEscudo();
                        mBala = 0;
                    }
                }
            }
            // Acertou antes de chegar perto (o colisor do chefe e grande): quebra.
            if (!achou && mBala != 0) {
                chefe->QuebrarEscudo();
                mBala = 0;
            }
        }
    private:
        std::uint64_t mBala = 0;
    };

    // ---- Lucas: Promise.all() ----

    class PromiseAll final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2&, const Vector2&) override {
            // Doze promessas em volta dele, pendentes, que resolvem juntas.
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

    // ---- Vitor: o semaforo ----

    class Mutex final : public PoderEmCampo {
    public:
        void Soltar(Orientador& o, Battle& batalha, const Vector2& aluno, const Vector2&) override {
            // P(s): a porta fica onde o aluno estava, de parede a parede.
            const SDL_FRect campo = batalha.GetPlayfieldBounds();
            mPorta = std::max(campo.y + kMaisAlta, aluno.y - Poder::kPortaAcimaDoAluno);
            mParados.clear();
            mVistos.clear();
            mUltimoY.clear();
            mProximaPassagem = o.Tempo() + o.Descricao().intervalo;
            mFechada = true;
            mAnunciados = -1;
        }
        void Atualizar(Orientador& o, Battle& batalha, const float deltaTime, const bool ativo) override {
            if (!mFechada) return;
            mParados.erase(std::remove_if(mParados.begin(), mParados.end(),
                                          [&o](const Parado& c) { return !Mesmo(o, c); }), mParados.end());
            if (!ativo) {
                // V(s): a trava abre, e quem ainda esperava morre de inanicao.
                for (const Parado& c : mParados) c.p->Kill();
                o.Rotular("V(s): " + std::to_string(mParados.size()) + " morreram de inanição");
                mParados.clear();
                mFechada = false;
                return;
            }

            // Quem chega na porta descendo para, e entra na fila.
            for (BossProjectile* p : o.TirosSoltos(batalha)) {
                const float y = p->GetPosition().y;
                const auto antes = mUltimoY.find(p);
                const float vy = (antes != mUltimoY.end() && deltaTime > 0.0f) ? (y - antes->second) / deltaTime : 0.0f;
                mUltimoY[p] = y;
                if (mVistos.count(p) || !Poder::NaPorta(y, vy, mPorta)) continue;
                mVistos.insert(p);
                auto* rb = p->GetComponent<RigidBodyComponent>();
                mParados.push_back({p, p->GetPosition(), rb ? rb->GetVelocity() : Vector2::Zero, p->TempoDeVida(),
                                    o.Tempo()});
            }

            // Um de cada vez: o primeiro da fila passa.
            if (o.Tempo() >= mProximaPassagem && !mParados.empty()) {
                const Parado c = mParados.front();
                mParados.erase(mParados.begin());
                if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(c.velocidade);
                mProximaPassagem = o.Tempo() + o.Descricao().intervalo;
            }
            for (const Parado& c : mParados) {
                c.p->SetPosition(c.posicao);
                if (auto* rb = c.p->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
            }

            if (static_cast<int>(mParados.size()) != mAnunciados) {
                mAnunciados = static_cast<int>(mParados.size());
                o.Rotular("P(s)   s = 0   fila: " + std::to_string(mAnunciados));
            }

            // A porta: a faixa da secao critica, como na fase dele.
            const SDL_FRect campo = batalha.GetPlayfieldBounds();
            for (const float dy : {0.0f, 5.0f}) {
                o.Tracar(Vector2(campo.x + 6.0f, mPorta + dy), Vector2(campo.x + campo.w - 6.0f, mPorta + dy), kCorDaPorta);
            }
        }
    private:
        /// A porta nunca fica colada no professor.
        static constexpr float kMaisAlta = 150.0f;
        static constexpr SDL_Color kCorDaPorta{255, 214, 72, 235};

        struct Parado {
            BossProjectile* p = nullptr;
            Vector2 posicao;
            Vector2 velocidade;
            float idade = 0.0f;
            float desde = 0.0f;
        };
        /// O pool recicla o projetil: o guardado so e o mesmo tiro se esta ativo e a idade bate.
        static bool Mesmo(const Orientador& o, const Parado& c) {
            if (!c.p || c.p->GetState() != ActorState::Active) return false;
            return std::fabs(c.p->TempoDeVida() - (c.idade + (o.Tempo() - c.desde))) <= kFolgaDaIdade;
        }

        float mPorta = 0.0f;
        bool mFechada = false;
        float mProximaPassagem = 0.0f;
        int mAnunciados = -1;
        std::vector<Parado> mParados;
        std::unordered_set<const BossProjectile*> mVistos;
        std::unordered_map<const BossProjectile*, float> mUltimoY;
    };
}

std::unique_ptr<PoderEmCampo> PoderEmCampo::Do(const Poder::Tipo t) {
    switch (t) {
        case Poder::Tipo::Matriz:      return std::make_unique<Matriz>();
        case Poder::Tipo::Pilha:       return std::make_unique<Pilha>();
        case Poder::Tipo::Congelar:    return std::make_unique<Congelar>();
        case Poder::Tipo::Transacao:   return std::make_unique<Transacao>();
        case Poder::Tipo::TryCatch:    return std::make_unique<TryCatch>();
        case Poder::Tipo::Converter:   return std::make_unique<Converter>();
        case Poder::Tipo::Agentes:     return std::make_unique<Agentes>();
        case Poder::Tipo::BalaDePrata: return std::make_unique<BalaDePrata>();
        case Poder::Tipo::PromiseAll:  return std::make_unique<PromiseAll>();
        case Poder::Tipo::Mutex:       return std::make_unique<Mutex>();
        case Poder::Tipo::BuscaBinaria: return std::make_unique<BuscaBinaria>();
        case Poder::Tipo::Rodizio:     return std::make_unique<Rodizio>();
        case Poder::Tipo::KMedias:     return std::make_unique<KMedias>();
        case Poder::Tipo::Nenhum:      break;
    }
    return nullptr;
}
