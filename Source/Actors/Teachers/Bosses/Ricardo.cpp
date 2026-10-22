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

void Ricardo::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

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
