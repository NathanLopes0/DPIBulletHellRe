//
// As colisoes de um quadro de luta.
//

#include "Colisoes.h"

#include "EfeitosDaBatalha.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Actors/Teachers/ExtraPointItem.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Interface/Devolutiva.h"
#include "../../Regras/Destrutivel.h"
#include "../../Regras/PontoExtra.h"
#include "../../Regras/Raspao.h"

namespace {

/// O colisor de um jogador, ou nullptr se nao houver jogador ou colisor.
const CircleColliderComponent* ColisorDe(const Player* jogador) {
    return jogador ? jogador->GetComponent<CircleColliderComponent>() : nullptr;
}

} // namespace

Colisoes::Colisoes(const std::vector<Player*>& jogadores, const std::vector<Boss*>& corpos,
                   const std::vector<ExtraPointItem*>& pontosExtras, EfeitosDaBatalha& efeitos, Regras& regras)
    : mJogadores(jogadores), mCorpos(corpos), mPontosExtras(pontosExtras), mEfeitos(efeitos), mRegras(regras) {}

void Colisoes::Verificar(Boss* chefe, const std::vector<std::unique_ptr<BossProjectile>>& doChefe,
                         const std::vector<std::unique_ptr<PlayerProjectile>>& dosAlunos) {
    if (mJogadores.empty() || !chefe) return;
    if (!chefe->GetComponent<CircleColliderComponent>()) return;

    // A ordem das regras importa: cada uma ve o que a anterior ja derrubou.
    Abrigo(doChefe);
    Escudo(doChefe);
    TiroDoChefeNoAluno(doChefe);
    TiroDoAlunoNosProjeteis(chefe, dosAlunos, doChefe);
    TiroDoAlunoNoChefe(dosAlunos);
    Contato();
    PontosExtras();
}

void Colisoes::Abrigo(const std::vector<std::unique_ptr<BossProjectile>>& doChefe) {
    for (const auto& abrigo : doChefe) {
        if (abrigo->GetState() != ActorState::Active || !abrigo->Abrigo()) continue;
        const auto* colisorDoAbrigo = abrigo->GetComponent<CircleColliderComponent>();
        if (!colisorDoAbrigo) continue;
        for (const auto& projetil : doChefe) {
            if (projetil->GetState() != ActorState::Active) continue;
            if (projetil->Abrigo() || projetil->Enfeite() || !projetil->Posto().empty()) continue;
            const auto* outro = projetil->GetComponent<CircleColliderComponent>();
            if (outro && colisorDoAbrigo->Intersect(*outro)) projetil->Kill();
        }
    }
}

void Colisoes::Escudo(const std::vector<std::unique_ptr<BossProjectile>>& doChefe) {
    for (Player* jogador : mJogadores) {
        if (!jogador || jogador->TempoDeImunidade() <= 0.0f) continue;
        const auto* colisor = ColisorDe(jogador);
        const Vector2 centro = colisor ? colisor->GetCenter() : jogador->GetPosition();
        for (const auto& projetil : doChefe) {
            if (projetil->GetState() != ActorState::Active) continue;
            if (projetil->Abrigo() || projetil->Enfeite() || !projetil->Posto().empty()) continue;
            const auto* outro = projetil->GetComponent<CircleColliderComponent>();
            if (outro && PontoExtra::NoEscudo(centro, outro->GetCenter(), outro->GetRadius())) projetil->Kill();
        }
    }
}

void Colisoes::TiroDoChefeNoAluno(const std::vector<std::unique_ptr<BossProjectile>>& doChefe) {
    for (Player* jogador : mJogadores) {
        const auto* colisor = ColisorDe(jogador);
        if (!colisor) continue;

        for (const auto& projetil : doChefe) {
            if (projetil->GetState() != ActorState::Active) continue;
            if (projetil->Enfeite()) continue;   // o cursor da sprint e a mao, nao um tiro
            if (projetil->Abrigo()) continue;    // a capivara nao machuca ninguem
            const auto* colisorDoProjetil = projetil->GetComponent<CircleColliderComponent>();
            if (!colisorDoProjetil) continue;

            // O mesmo tiro nao acerta duas vezes seguidas (Acerto.h).
            if (colisor->Intersect(*colisorDoProjetil) && !jogador->EstaInvencivel() &&
                !jogador->JaFoiAcertadoPor(projetil.get(), projetil->TempoDeVida())) {
                jogador->DefinirInvencivel(true);
                jogador->FoiAcertadoPor(projetil.get(), projetil->TempoDeVida());
                jogador->OnCollision(projetil.get());
                projetil->OnCollision(jogador);

                // Nem todo acerto e a mesma coisa.
                if (projetil->Acorrenta()) {
                    jogador->Acorrentar(projetil->SegundosDeCorrente(), projetil->FatorDaCorrente());
                }
                mEfeitos.Faiscar(colisor->GetCenter(), Hud::Faisca::Dano);
                mRegras.AlunoAcertado(*jogador, projetil->Tipo(), projetil->MultiplicadorDaPerda(), true);
            } else {
                Raspar(*jogador, *colisor, *projetil, *colisorDoProjetil);
            }
        }
    }
}

void Colisoes::Raspar(Player& aluno, const CircleColliderComponent& colisor, BossProjectile& projetil,
                      const CircleColliderComponent& colisorDoProjetil) {
    // O que esta parado num posto (a pilha, a tabela) e o que nem anda nao e um desvio; e quem esta
    // invencivel nao arrisca nada.
    if (aluno.EstaInvencivel() || !projetil.Posto().empty()) return;
    // A conta da faixa antes de procurar o corpo rigido: quase todo projetil passa longe.
    if (!Raspao::NaFaixa(colisor.GetCenter(), colisor.GetRadius(), colisorDoProjetil.GetCenter(),
                         colisorDoProjetil.GetRadius())) return;
    const auto* rb = projetil.GetComponent<RigidBodyComponent>();
    if (!rb || rb->GetVelocity().Length() < Raspao::kRapidezMinima) return;
    if (aluno.JaRaspou(&projetil, projetil.TempoDeVida())) return;

    aluno.Raspou(&projetil, projetil.TempoDeVida());
    mRegras.AlunoRaspou(aluno, Raspao::OndeFaisca(colisor.GetCenter(), colisorDoProjetil.GetCenter()));
}

void Colisoes::TiroDoAlunoNosProjeteis(Boss* chefe, const std::vector<std::unique_ptr<PlayerProjectile>>& dosAlunos,
                                       const std::vector<std::unique_ptr<BossProjectile>>& doChefe) {
    for (const auto& tiro : dosAlunos) {
        if (tiro->GetState() != ActorState::Active) continue;
        if (!tiro->GetComponent<CircleColliderComponent>()) continue;

        // A promessa do Lucas (Promise.all, no TCC) desfaz e segue.
        if (tiro->Promessa()) {
            for (const auto& projetil : doChefe) {
                if (projetil->GetState() != ActorState::Active) continue;
                if (projetil->Enfeite() || projetil->Abrigo() || projetil->Blindado() || !projetil->Posto().empty()) continue;
                const auto* alvo = projetil->GetComponent<CircleColliderComponent>();
                if (!alvo || !tiro->Alcanca(*alvo)) continue;
                if (!tiro->DesfazPelaPrimeiraVez(projetil.get())) continue;

                if (projetil->Derrubavel()) {
                    if (projetil->Golpear()) Derrubar(chefe, *tiro, *projetil);
                } else {
                    mEfeitos.Faiscar(projetil->GetPosition(), Hud::Faisca::Dissolvido);
                    projetil->Kill();
                }
            }
            continue;
        }

        for (const auto& projetil : doChefe) {
            if (projetil->GetState() != ActorState::Active) continue;
            if (!projetil->Derrubavel() && !projetil->Blindado() && !projetil->Abrigo()) continue;
            if (projetil->Enfeite()) continue;   // o tiro passa pelo cursor
            const auto* alvo = projetil->GetComponent<CircleColliderComponent>();
            if (!alvo || !tiro->Alcanca(*alvo)) continue;

            if (!Destrutivel::Pode(projetil->Derrubavel(), projetil->Blindado(), projetil->Abrigo(), projetil->Enfeite())) {
                tiro->Murchar();
                break;
            }
            tiro->OnCollision(projetil.get());
            // A resistencia: so o ultimo golpe derruba.
            if (projetil->Golpear()) {
                Derrubar(chefe, *tiro, *projetil);
            } else if (chefe) {
                mEfeitos.Faiscar(tiro->GetPosition(), Hud::Faisca::Acerto);
                chefe->OnProjetilBarrado();
            }
            break;   // um tiro derruba um processo so, ou some numa historia so
        }
    }
}

void Colisoes::Derrubar(Boss* chefe, PlayerProjectile& tiro, BossProjectile& projetil) {
    // A cobertura tem a faisca dela.
    if (!mRegras.ProjetilDerrubado(tiro, projetil)) mEfeitos.Faiscar(projetil.GetPosition(), Hud::Faisca::Derrubada);
    projetil.OnCollision(&tiro);
    if (chefe) {
        chefe->OnProjetilDerrubado();
        chefe->OnDerrubou(&projetil);
    }
}

void Colisoes::TiroDoAlunoNoChefe(const std::vector<std::unique_ptr<PlayerProjectile>>& dosAlunos) {
    for (const auto& tiro : dosAlunos) {
        if (tiro->GetState() != ActorState::Active) continue;
        if (!tiro->GetComponent<CircleColliderComponent>()) continue;

        for (Boss* corpo : mCorpos) {
            const auto* colisorDoCorpo = corpo ? corpo->GetComponent<CircleColliderComponent>() : nullptr;
            if (!colisorDoCorpo || !tiro->Alcanca(*colisorDoCorpo)) continue;

            // A promessa que ja volta passa pelo Hugo: ela ja resolveu.
            if (tiro->Promessa() && tiro->Voltando()) continue;

            corpo->OnCollision(tiro.get());
            // O chefe blindado nao ouve a pergunta.
            const bool ouviu = corpo->AceitaDano();
            if (tiro->Promessa()) tiro->Voltar();
            else if (ouviu) tiro->OnCollision(corpo);
            else tiro->Murchar();

            mRegras.ProfessorAcertado(*tiro, *colisorDoCorpo, ouviu);

            // Um tiro acerta um corpo so.
            break;
        }
    }
}

void Colisoes::Contato() {
    for (Player* jogador : mJogadores) {
        const auto* colisor = ColisorDe(jogador);
        if (!colisor) continue;

        for (Boss* corpo : mCorpos) {
            const auto* colisorDoCorpo = corpo ? corpo->GetComponent<CircleColliderComponent>() : nullptr;
            if (!colisorDoCorpo) continue;

            if (colisor->Intersect(*colisorDoCorpo) && !jogador->EstaInvencivel() &&
                !jogador->JaFoiAcertadoPor(corpo, 0.0f)) {
                jogador->DefinirInvencivel(true);
                jogador->FoiAcertadoPor(corpo, 0.0f);
                jogador->OnCollision(corpo);
                corpo->OnCollision(jogador);
                mRegras.AlunoAcertado(*jogador, Devolutiva::kEsbarrao, 1.0f, false);
            }
        }
    }
}

void Colisoes::PontosExtras() {
    bool primeiro = true;
    for (Player* jogador : mJogadores) {
        const auto* colisor = ColisorDe(jogador);
        if (!colisor) continue;

        for (ExtraPointItem* item : mPontosExtras) {
            if (item->GetState() != ActorState::Active) continue;
            const auto* colisorDoItem = item->GetComponent<CircleColliderComponent>();
            if (!colisorDoItem || !colisor->Intersect(*colisorDoItem)) continue;

            jogador->GanharPontoExtra();
            mEfeitos.Faiscar(colisor->GetCenter(), Hud::Faisca::Coleta);
            mRegras.PontoExtraPego(*jogador, primeiro);
            primeiro = false;
            item->SetState(ActorState::Destroy);
        }
    }
}
