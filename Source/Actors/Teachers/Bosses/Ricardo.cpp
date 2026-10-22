//
// O chefe da INF 250 - Organizacao de Computadores.
//

#include "Ricardo.h"

#include <algorithm>
#include <limits>

#include "../../../Game.h"
#include "../../../Random.h"
#include "../../../Attacks/AttackParameters/PalavraAttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/RigidBodyComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

Ricardo::Ricardo(Scene* scene) : Boss(scene)
{
}

void Ricardo::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarPortas();
    AtualizarRelogio();
}

std::vector<BossProjectile*> Ricardo::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
}

Logica::Porta Ricardo::PortaEm(const float t) const {
    if (const auto porta = Logica::LerPorta(AnuncioDaFaseEm(Logica::kFaseDasPortas, t), Logica::kBits)) {
        return *porta;
    }
    return mPorta;
}

void Ricardo::AtualizarPortas() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasPortas = fsm && fsm->GetStateName() == Logica::kFaseDasPortas && !EhClone();
    if (!mScene || !mScene->GetGame()) return;
    const auto largura = static_cast<float>(mScene->GetGame()->GetWindowWidth());

    if (nasPortas) {
        mPorta = PortaEm(fsm->GetStateTime());
        if (!mPortasFormadas && fsm->GetStateTime() >= kFormaAsPortas) {
            mDisparandoPortas = true;
            Disparar(Logica::kGatilhoDasPortas);
            mDisparandoPortas = false;
            mPortasFormadas = true;
        }
    }

    // A CARA DE CADA PORTA: a operacao da etapa e o B da coluna dela.
    const auto portas = NoPosto(Logica::kPostoDasPortas);
    for (BossProjectile* porta : portas) {
        const int coluna = Logica::Coluna(porta->GetPosition().x, largura, Logica::kBits);
        const std::string cara = Logica::AnimacaoDaPorta(mPorta, coluna, Logica::kBits);
        if (auto* anim = porta->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }

    // A TRAVESSIA. Todo bit que chegou a altura das portas sai pela porta da
    // coluna dele: o valor que entra e o dele (o UM e o bit que nao e enfeite),
    // o B e o da porta, e o que sai e a tabela-verdade. Depois ele deixa o
    // posto: ja passou, e nao passa de novo.
    if (!portas.empty()) {
        const float altura = portas.front()->GetPosition().y;
        for (BossProjectile* bit : NoPosto(Logica::kPostoDaPalavra)) {
            if (bit->GetPosition().y < altura) continue;
            const int coluna = Logica::Coluna(bit->GetPosition().x, largura, Logica::kBits);
            const bool sai = Logica::Saida(mPorta.operacao, !bit->Enfeite(), Logica::Bit(mPorta.b, coluna, Logica::kBits));
            if (auto* anim = bit->GetComponent<DrawAnimatedComponent>()) {
                anim->SetAnimation(sai ? Logica::kAnimacaoDoUm : Logica::kAnimacaoDoZero);
            }
            bit->DefinirEnfeite(!sai);
            bit->DefinirPosto("");
        }
    }

    // FORA DA FASE as portas ficam ate a ultima palavra passar por elas - a
    // que saiu no fim da fase ainda esta a caminho - e entao se desfazem.
    if (!nasPortas && mPortasFormadas && NoPosto(Logica::kPostoDaPalavra).empty()) {
        for (BossProjectile* porta : portas) porta->Kill();
        mPortasFormadas = false;
    }
}

void Ricardo::MoverRegistradores(const float velocidade) {
    for (BossProjectile* r : NoPosto(Relogio::kPostoDosRegistradores)) {
        const auto direcao = mDirecoes.find(r);
        if (direcao == mDirecoes.end()) continue;   // ainda nao acordou
        if (auto* rb = r->GetComponent<RigidBodyComponent>()) rb->SetVelocity(direcao->second * velocidade);
    }
}

void Ricardo::AtualizarRelogio() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noRelogio = fsm && fsm->GetStateName() == Relogio::kFaseDoRelogio && !EhClone();

    if (!noRelogio) {
        // O CLOCK PAROU: sem borda, o registrador vira fio, e o que estava
        // parado escorre na velocidade do ultimo clock - e sai da tela, em vez
        // de ficar congelado no meio da fase seguinte.
        if (mAntesNoRelogio >= 0.0f) {
            MoverRegistradores(Relogio::kPasso * mFrequencia);
            for (BossProjectile* r : NoPosto(Relogio::kPostoDosRegistradores)) r->DefinirPosto("");
            mDirecoes.clear();
            mAntesNoRelogio = -1.0f;
            mFimDoPasso = -1.0f;
        }
        return;
    }
    if (mAntesNoRelogio < 0.0f) {
        mAntesNoRelogio = 0.0f;
        mContador = 1;
    }

    const float agora = fsm->GetStateTime();
    mFrequencia = Relogio::LerFrequencia(AnuncioDaFase(Relogio::kFaseDoRelogio)).value_or(1.0f);

    // O REGISTRADOR QUE ACABOU DE ACORDAR anda na direcao do leque: ela e
    // anotada, e ele para ate a proxima borda. E o que ja nao esta em campo
    // sai da lista.
    for (auto it = mDirecoes.begin(); it != mDirecoes.end();) {
        if (it->first->GetState() != ActorState::Active || it->first->Posto() != Relogio::kPostoDosRegistradores) {
            it = mDirecoes.erase(it);
        } else {
            ++it;
        }
    }
    for (BossProjectile* r : NoPosto(Relogio::kPostoDosRegistradores)) {
        if (mDirecoes.count(r)) continue;
        auto* rb = r->GetComponent<RigidBodyComponent>();
        if (!rb || rb->GetVelocity().LengthSq() <= 0.0f) continue;
        Vector2 direcao = rb->GetVelocity();
        direcao.Normalize();
        mDirecoes[r] = direcao;
        rb->SetVelocity(Vector2::Zero);
    }

    // O SEGUNDO VIROU: o contador sai num leque, mirado no aluno.
    if (Relogio::Subiu(mAntesNoRelogio, agora, 1.0f)) Disparar(Relogio::kGatilhoDoContador);

    // A BORDA DE SUBIDA: todo registrador da um passo de kPasso pixels...
    if (Relogio::Subiu(mAntesNoRelogio, agora, mFrequencia)) {
        const float duracao = Relogio::DuracaoDoPasso(mFrequencia);
        MoverRegistradores(Relogio::kPasso / duracao);
        mFimDoPasso = agora + duracao;
    }
    // ...e para, ate a proxima.
    else if (mFimDoPasso >= 0.0f && agora >= mFimDoPasso) {
        MoverRegistradores(0.0f);
        mFimDoPasso = -1.0f;
    }
    mAntesNoRelogio = agora;
}

void Ricardo::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                   const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (stateName != Relogio::kFaseDoRelogio) return;

    // O LEQUE E O VALOR DO CONTADOR: um bit por tiro, o mais alto a esquerda de
    // quem o recebe. A WaveAttack abre o leque do angulo menor para o maior, e
    // com o leque descendo o angulo menor e a direita - por isso o tiro i leva
    // o bit da coluna n-1-i.
    const int n = static_cast<int>(projectiles.size());
    const std::uint32_t valor = Relogio::Johnson(mContador, n);
    for (int i = 0; i < n; ++i) {
        auto* bit = dynamic_cast<BossProjectile*>(projectiles[static_cast<size_t>(i)].get());
        if (!bit) continue;
        mDirecoes.erase(bit);   // um objeto do pool que ja foi registrador e outro agora
        const bool um = Logica::Bit(valor, n - 1 - i, n);
        if (auto* anim = bit->GetComponent<DrawAnimatedComponent>()) {
            anim->SetAnimation(um ? Logica::kAnimacaoDoUm : Logica::kAnimacaoDoZero);
        }
        bit->DefinirEnfeite(!um);
    }
    ++mContador;
}

void Ricardo::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // O LEQUE DO CONTADOR SAI MIRADO NO ALUNO, com UM BIT apontado para ele:
    // num leque de 8 o meio cai entre dois bits, e quem fica parado ficava
    // sempre na fresta - o piloto parado levou zero acertos na fase. Meio
    // passo de giro poe nele o bit da coluna 4. centralAngle e escrito inteiro
    // a cada disparo (params e o mesmo objeto toda vez - ver Boss.h).
    if (stateName == Relogio::kFaseDoRelogio) {
        const Vector2 d = GetDirectionToPlayer();
        const float meioPasso = params.numProjectiles > 1
                                    ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1)
                                    : 0.0f;
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
        return;
    }

    // A FILEIRA DE PORTAS nao e palavra: quem lhe da a cara e AtualizarPortas.
    if (mDisparandoPortas) return;
    if (stateName != Logica::kFaseDasPortas) return;
    auto* palavra = dynamic_cast<PalavraAttackParams*>(&params);
    if (!palavra) return;

    // A PALAVRA E SORTEADA PARA A PORTA QUE ELA VAI ATRAVESSAR - a da etapa em
    // que ela chega as portas, e nao a de agora: uma palavra que sai no fim de
    // uma etapa cruza a porta da seguinte. Justa (Logica::Justa): parede com
    // porta antes e depois, e a porta muda alguma coisa.
    const auto* fsm = GetComponent<FSMComponent>();
    float chegada = fsm ? fsm->GetStateTime() : 0.0f;
    const auto portas = NoPosto(Logica::kPostoDasPortas);
    const float de = palavra->altura >= 0.0f ? palavra->altura : params.firePosition.y;
    if (!portas.empty() && params.projectileSpeed > 0.0f) {
        chegada += std::max(0.0f, portas.front()->GetPosition().y - de) / params.projectileSpeed;
    }
    const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
    palavra->palavra = Logica::Sortear(PortaEm(chegada), params.numProjectiles, sorteio);
}
