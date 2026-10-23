//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#include "Vladimir.h"

#include "../../../Attacks/Alfabeto.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

Vladimir::Vladimir(Scene* scene) : Boss(scene) {
}

void Vladimir::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

std::string Vladimir::AvisoDoIntercambio() const {
    return Alfabeto::kAvisoDaTroca;
}

void Vladimir::OnIntercambio() {
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return;

    // TODA LETRA EM VOO TROCA DE LETRA E DE COMPORTAMENTO, no quadro em que os
    // alunos trocam de lugar. O "a" ganha o Tracking do "b", e o "b" perde o
    // dele e segue reto no rumo em que estava - o modulo nao muda nos dois
    // casos, entao nenhuma letra acelera de surpresa.
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (!p || p->GetState() != ActorState::Active) continue;
        auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (!anim || !Alfabeto::Troca(anim->GetAnimation(), p->Posto(), p->Enfeite())) continue;

        const std::string nova = Alfabeto::Trocar(anim->GetAnimation());
        anim->SetAnimation(nova);
        if (nova == Alfabeto::kB) {
            p->insertMotion<TrackingBehavior>(kAtrasoDaTroca, kForcaDaTroca, kDuracaoDaTroca);
        } else {
            p->DescartarMotion();
        }
    }
}

void Vladimir::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    // NADA AQUI, E DE PROPOSITO. CustomizeAttackParams e o unico pedaco do
    // balanceamento que ainda vive em C++, e e onde o angulo de um ataque pode
    // ser sobrescrito em tempo de execucao. Os padroes do Vladimir sao
    // deterministicos - uma palavra tem a forma que tem -, entao nenhum deles
    // precisa de sorteio no codigo: tudo cabe em fases.json e regras.json.
    Boss::CustomizeAttackParams(params, stateName);
}
