//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"

#include <algorithm>
#include <limits>

#include "../../../Game.h"
#include "../../../Random.h"
#include "../../../Attacks/Ancora.h"
#include "../../../Actors/Player/Player.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"


Salles::Salles(Scene *scene) : Boss(scene)
{

}

void Salles::OnUpdate(float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarCapivara(deltaTime);
    AtualizarPilha(deltaTime);
}

std::optional<Vector2> Salles::AlvoDoMovimento() const {
    return mAlvo;
}

std::vector<BossProjectile*> Salles::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

void Salles::DerrubarPilha() {
    // A PILHA CAI ABERTA EM LEQUE, cada bloco num angulo: a do topo no meio, e
    // as outras para os lados - uma chuva, e nao uma coluna so.
    const int n = static_cast<int>(mPilha.size());
    for (int k = 0; k < n; ++k) {
        BossProjectile* b = mPilha[static_cast<size_t>(k)];
        b->DefinirPosto("");
        const float abertura = n > 1 ? (static_cast<float>(k) / static_cast<float>(n - 1) - 0.5f) * 50.0f : 0.0f;
        const float a = Math::ToRadians(90.0f + abertura);
        if (auto* rb = b->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2(Math::Cos(a), Math::Sin(a)) * 260.0f);
    }
    mPilha.clear();
    mValores.clear();
}

void Salles::AtualizarPilha(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool naPilha = fsm && fsm->GetStateName() == Pilha::kFaseDaPilha && !EhClone();

    // OS BLOCOS QUE SAIRAM NUM POP ESTOURAM na hora deles, dentro ou fora da
    // fase: um pop do fim da fase ainda esta voando.
    for (auto& p : mPopados) {
        if (p.bloco->GetState() != ActorState::Active) { p.idade = -1.0f; continue; }
        p.idade += deltaTime;
        if (p.idade < Pilha::kEstouraEm) continue;
        if (p.valor > 0) {
            mEstourando = true;
            mDisparoDoEstouro = p.bloco->GetPosition();
            mLascas = p.valor;
            Disparar(Pilha::kGatilhoDoEstouro);
            mEstourando = false;
        }
        p.bloco->Kill();
        p.idade = -1.0f;
    }
    mPopados.erase(std::remove_if(mPopados.begin(), mPopados.end(), [](const Popado& p) { return p.idade < 0.0f; }),
                   mPopados.end());

    if (!naPilha) {
        // A FASE ACABOU: a pilha cai, e o professor deixa de ter coluna.
        if (mNaPilha) {
            DerrubarPilha();
            if (!mAnuncioDaPilha.empty()) RegistrarAnuncio(Pilha::kFaseDaPilha, mAnuncioDaPilha);
            mAlvo.reset();
            mNaPilha = false;
        }
        return;
    }
    if (!mNaPilha) {
        mNaPilha = true;
        mPilha.clear();
        mValores.clear();
        mProximaOperacao = Pilha::kPrimeira;
        mProximaMudanca = kParadoNaColuna;
        mColuna = 1;
        mAlvo = Vector2(kColunasDaPilha[1], kAlturaDaPilha);
        if (mAnuncioDaPilha.empty()) mAnuncioDaPilha = AnuncioDaFase(Pilha::kFaseDaPilha);
    }
    const float agora = fsm->GetStateTime();

    // QUEM SAIU DA PILHA SAI DA CONTA: o bloco que o aluno derrubou a tiro.
    for (size_t k = 0; k < mPilha.size();) {
        if (mPilha[k]->GetState() != ActorState::Active || mPilha[k]->Posto() != Pilha::kPostoDaPilha) {
            mPilha.erase(mPilha.begin() + static_cast<std::ptrdiff_t>(k));
            mValores.erase(mValores.begin() + static_cast<std::ptrdiff_t>(k));
        } else {
            ++k;
        }
    }

    // DE COLUNA EM COLUNA, parando em cada uma: parado, ele leva tiro; e
    // andando, a pilha pendurada varre o campo.
    if (agora >= mProximaMudanca) {
        mProximaMudanca += kParadoNaColuna;
        const int salto = Random::GetIntRange(1, 2);
        mColuna = (mColuna + salto) % static_cast<int>(kColunasDaPilha.size());
        mAlvo = Vector2(kColunasDaPilha[static_cast<size_t>(mColuna)], kAlturaDaPilha);
    }

    // A OPERACAO DO PROGRAMA.
    if (agora >= mProximaOperacao) {
        mProximaOperacao += Pilha::kPasso;
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        const Pilha::Operacao op = Pilha::Proxima(static_cast<int>(mPilha.size()), sorteio);
        const int topo = mValores.empty() ? 0 : mValores.back();

        if (op.tipo == Pilha::Tipo::Push) {
            mEmpurrando = true;
            mEmpurrado = nullptr;
            Disparar(Pilha::kGatilhoDoPush);
            mEmpurrando = false;
            if (mEmpurrado) {
                if (auto* anim = mEmpurrado->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(Pilha::CaraDoBloco(op.valor));
                mPilha.push_back(mEmpurrado);
                mValores.push_back(op.valor);
            }
        } else if (op.tipo == Pilha::Tipo::Pop && !mPilha.empty()) {
            // O TOPO SAI MIRADO NO ALUNO, e estoura no caminho.
            BossProjectile* b = mPilha.back();
            b->DefinirPosto("");
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - b->GetPosition();
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            if (auto* rb = b->GetComponent<RigidBodyComponent>()) rb->SetVelocity(d * Pilha::kVelocidadeDoPop);
            mPopados.push_back(Popado{b, 0.0f, mValores.back()});
            mPilha.pop_back();
            mValores.pop_back();
        } else if (op.tipo == Pilha::Tipo::Overflow) {
            DerrubarPilha();
        }
        RegistrarAnuncio(Pilha::kFaseDaPilha, Pilha::Placar(op, topo));
    }

    // CADA BLOCO NO LUGAR DELE, embaixo do professor: o 0 colado nele, o topo
    // la embaixo. Parados - sao armadura, e nao tiro.
    for (size_t k = 0; k < mPilha.size(); ++k) {
        BossProjectile* b = mPilha[k];
        const Vector2 lugar = GetPosition() + Vector2(0.0f, Pilha::AbaixoDoProfessor(static_cast<int>(k)));
        b->SetPosition(Ancora::Aproximar(b->GetPosition(), lugar, kRapidezDoBloco, deltaTime));
        if (auto* rb = b->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    }
}

void Salles::AtualizarCapivara(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || EhClone()) return;
    const std::string& fase = fsm->GetStateName();
    if (fase.empty()) return;

    // O AVISO NO PLACAR SAI SOZINHO, no tempo dele - na fase em que foi dado.
    if (mAvisoDaCapivara > 0.0f) {
        mAvisoDaCapivara -= deltaTime;
        if (mAvisoDaCapivara <= 0.0f) RegistrarAnuncio(mFaseDaCapivara, mAnuncioAntesDaCapivara);
    }

    // UMA FASE NOVA, UM SORTEIO NOVO: se ela vem nesta, quando e de que lado.
    if (fase != mFaseDaCapivara) {
        if (mAvisoDaCapivara > 0.0f) RegistrarAnuncio(mFaseDaCapivara, mAnuncioAntesDaCapivara);
        mAvisoDaCapivara = 0.0f;
        mFaseDaCapivara = fase;
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        mVisita = Capivara::Sortear(sorteio);
        mVeio = false;
    }
    if (!mVisita.vem || mVeio || fsm->GetStateTime() < mVisita.quando) return;

    // ELA APARECE, e o placar avisa.
    mVeio = true;
    mDisparandoCapivara = true;
    Disparar(Capivara::kGatilho);
    mDisparandoCapivara = false;
    mAnuncioAntesDaCapivara = AnuncioDaFase(fase);
    RegistrarAnuncio(fase, Capivara::kAnuncio);
    mAvisoDaCapivara = Capivara::kAnuncioNaTela;
}

void Salles::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                  const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);

    // O BLOCO QUE O PUSH CRIOU: e ele que entra na pilha.
    if (mEmpurrando) {
        for (auto& p : projectiles) {
            if (auto* b = dynamic_cast<BossProjectile*>(p.get()); b && b->Posto() == Pilha::kPostoDaPilha) mEmpurrado = b;
        }
        return;
    }
    if (!mDisparandoCapivara) return;

    // ELA ANDA OLHANDO PARA ONDE VAI.
    for (auto& p : projectiles) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) {
            anim->SetAnimation(mVisita.daEsquerda ? Capivara::kAnimacaoDireita : Capivara::kAnimacaoEsquerda);
        }
    }
}

void Salles::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // A CAPIVARA ENTRA POR UMA BORDA E ATRAVESSA ATE A OUTRA, reta, na altura
    // dela - e nao sai do professor nem no angulo da fase.
    if (mDisparandoCapivara) {
        const float largura = (mScene && mScene->GetGame())
                                  ? static_cast<float>(mScene->GetGame()->GetWindowWidth()) : 1200.0f;
        params.firePosition = Vector2(mVisita.daEsquerda ? -Capivara::kForaDaTela : largura + Capivara::kForaDaTela,
                                      Capivara::kAltura);
        params.centralAngle = mVisita.daEsquerda ? 0.0f : 180.0f;
        return;
    }

    // O POP ESTOURA ONDE O BLOCO ESTA, em tantas lascas quanto o numero dele.
    // O total e escrito inteiro a cada disparo (params e o mesmo objeto - ver
    // Boss.h). E o push nasce no professor, parado: quem o leva ao lugar e a
    // pilha.
    if (stateName == Pilha::kFaseDaPilha) {
        if (mEstourando) {
            // MIRADO NO ALUNO, com uma lasca no meio do leque quando o numero e
            // par (ver Ricardo::CustomizeAttackParams).
            params.firePosition = mDisparoDoEstouro;
            params.numProjectiles = mLascas;
            Vector2 d(0.0f, 1.0f);
            if (const auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->Alvo()) {
                d = batalha->Alvo()->GetPosition() - mDisparoDoEstouro;
                if (d.LengthSq() > 0.0f) d.Normalize();
            }
            const bool par = mLascas > 1 && mLascas % 2 == 0;
            const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(mLascas - 1) : 0.0f;
            params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
        }
        return;
    }

    // Mesmo conserto de Andre::CustomizeAttackParams: Math::Acos(dir.x) so
    // devolve [0, 180] e perdia o sinal de y, fazendo o boss mirar para baixo
    // quando o player estava acima. Calculado uma vez, em vez de tres.
    const Vector2 dirToPlayer = GetDirectionToPlayer();
    const float angleToPlayer = Math::ToDegrees(Math::Atan2(dirToPlayer.y, dirToPlayer.x));

    if (stateName == "StateOne") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }

    }

    if (stateName == "StateTwo") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }
    }

    if (stateName == "StateThree") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {

            // A BUSCA DESCE POR UM RAMO, E NAO POR CIMA DA RAIZ.
            //
            // Uma arvore binaria tem numero PAR de folhas - 16, aqui - e elas
            // se abrem simetricamente em torno do eixo do tiro. Logo existe
            // sempre um corredor vazio EM CIMA do eixo, da largura de um
            // espacamento. Mirar o eixo no jogador punha esse corredor
            // exatamente nele: a arvore se abria em volta e passava sem
            // encostar, por mais que ele ficasse parado. Da para ver numa foto
            // da fase - quatro nos longe a esquerda, quatro longe a direita,
            // nada no meio.
            //
            // O desvio poe o jogador SOBRE um ramo em vez de sobre a raiz, que
            // e o que uma busca faz de verdade: na raiz ela vai para a esquerda
            // ou para a direita, nunca fica em cima. O sinal alterna para o
            // desvio nao virar um vies fixo, que o jogador aprenderia a
            // compensar ficando um pouco de lado.
            mLadoDaBusca = -mLadoDaBusca;
            params.centralAngle = angleToPlayer + mLadoDaBusca * kDesvioDaBusca;
        }
        else {
            params.centralAngle = Random::GetFloatRange(0.f, 180.f);
        }
    }

    if (stateName == "StateFinal") {
        params.centralAngle = Random::GetFloatRange(0.f, 180.f);
    }
}




