//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#include "Vitor.h"

#include "../../../Attacks/AttackParameters/AttackParams.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

#include <cmath>

Vitor::Vitor(Scene* scene) : Boss(scene) {
}

void Vitor::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarNuvens(deltaTime);
}

std::vector<BossProjectile*> Vitor::Nuvens() const {
    std::vector<BossProjectile*> nuvens;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return nuvens;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == Nuvem::kPostoDasNuvens) {
            nuvens.push_back(p.get());
        }
    }
    return nuvens;
}

void Vitor::DispararNuvem(const char* gatilho) {
    mDisparandoNuvem = true;
    Disparar(gatilho);
    mDisparandoNuvem = false;
}

void Vitor::AtualizarNuvens(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool nasNuvens = fsm && fsm->GetStateName() == kFaseDasNuvens && !EhClone();

    if (!nasNuvens) {
        if (mNasNuvens) {
            // A FASE ACABOU: as nuvens se desfazem, e o placar volta ao nome.
            for (BossProjectile* nuvem : Nuvens()) nuvem->Kill();
            if (!mNomeDaFaseDasNuvens.empty()) RegistrarAnuncio(kFaseDasNuvens, mNomeDaFaseDasNuvens);
            mNasNuvens = false;
        }
        return;
    }
    if (!mNasNuvens) {
        mNasNuvens = true;
        mNuvensFormadas = false;
        mTempoNasNuvens = 0.0f;
        mChuva.t = 0.0f;
        if (mNomeDaFaseDasNuvens.empty()) mNomeDaFaseDasNuvens = AnuncioDaFase(kFaseDasNuvens);
    }

    mTempoNasNuvens += deltaTime;
    if (!mNuvensFormadas) {
        if (mTempoNasNuvens < kFormaAsNuvens) return;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mNuvensFormadas = true;
        mChuva.t = 0.0f;
    }

    // O CICLO: processa, e no fim do processamento chove.
    if (Nuvem::Avancar(mChuva, deltaTime)) DispararNuvem(Nuvem::kGatilhoDaChuva);

    const bool processando = Nuvem::Processando(mChuva);
    const char* texto = processando ? kProcessando : kEnviando;
    if (AnuncioDaFase(kFaseDasNuvens) != texto) RegistrarAnuncio(kFaseDasNuvens, texto);

    const char* cara = processando ? Nuvem::kCaraProcessando : Nuvem::kCaraOciosa;
    for (BossProjectile* nuvem : Nuvens()) {
        if (auto* anim = nuvem->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }
}

void Vitor::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    // AS NUVENS NAO SAO O ANEL: nem a formacao delas nem a chuva mexem na
    // fresta. Sem isto cada chuva empurraria a fresta um passo, e a sequencia
    // de portas do anel mudaria por causa de uma coisa que nao e anel.
    if (mDisparandoNuvem) return;

    // A FRESTA ANDA, MAS SO PELA METADE DE BAIXO DA TELA.
    //
    // E a unica coisa do Vitor que nao cabe em dados: "para onde a abertura
    // aponta DESTA vez" depende da rajada anterior, e fases.json nao tem
    // memoria. O TAMANHO da fresta continua no arquivo ("angulo": 330 deixa 30
    // graus abertos); daqui sai so a DIRECAO.
    //
    // O clone da fase 3 tambem passa por aqui: ele roda os ataques de
    // StateTwo, e e outro Vitor, com a propria fresta comecando do meio.
    if (stateName == "StateTwo") {
        const float fresta = kFrestaMaisADireita + mDesvioDaFresta;

        // O leque e desenhado EM TORNO de centralAngle, entao a abertura fica
        // oposta a ele: somar 180 poe a fresta onde a variavel diz. E escrever
        // o valor inteiro, e nao somar sobre o disparo anterior - params e o
        // mesmo objeto a cada disparo (ver Boss.h).
        params.centralAngle = fresta + 180.0f;

        mDesvioDaFresta = std::fmod(mDesvioDaFresta + kPassoDaFresta, kJanelaDaFresta);
    }
}
