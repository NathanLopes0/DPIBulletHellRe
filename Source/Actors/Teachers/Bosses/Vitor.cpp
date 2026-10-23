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
#include "../../../Attacks/Behaviors.h"
#include "../../../Components/RigidBodyComponent.h"

#include <algorithm>
#include <cmath>

Vitor::Vitor(Scene* scene) : Boss(scene) {
}

void Vitor::OnUpdate(const float deltaTime) {

    // NO FORK o corpo nao anda sozinho nem ataca: o relogio da fase corre, mas
    // os ataques e o movimento dele esperam a nuvem se desfazer. E ele continua
    // levando tiro - o colisor fica onde ele esta -, porque o fork nao e um
    // intervalo da batalha.
    if (mTempoNoFork >= 0.0f) {
        mTempoNoFork += deltaTime;
        if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        if (auto* d = GetComponent<DrawAnimatedComponent>()) {
            d->SetIsVisible(Nuvem::Visivel(kFork, mTempoNoFork, mNasceuDentro));
        }
        SetPosition(mSaiDe + (mVaiPara - mSaiDe) * Nuvem::Saida(kFork, mTempoNoFork));
        if (!mNasceuDentro) AtualizarANuvemDoFork();
        if (mTempoNoFork >= kFork.sai) SairDaNuvem();
        return;
    }

    Boss::OnUpdate(deltaTime);
    AtualizarNuvens(deltaTime);
}

void Vitor::EntrarNaNuvem(const Vector2 onde, const Vector2 destino, const bool nasceuDentro) {
    SetPosition(onde);
    mSaiDe = onde;
    mVaiPara = destino;
    mNasceuDentro = nasceuDentro;
    mTempoNoFork = 0.0f;
    if (auto* rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
    if (auto* d = GetComponent<DrawAnimatedComponent>()) d->SetIsVisible(Nuvem::Visivel(kFork, 0.0f, nasceuDentro));
}

void Vitor::SairDaNuvem() {
    mTempoNoFork = -1.0f;
    SetPosition(mVaiPara);
    if (auto* d = GetComponent<DrawAnimatedComponent>()) d->SetIsVisible(true);
    if (EhClone()) return;

    // A NUVEM SE DESFAZ, e o placar volta ao nome da fase: sairam dois.
    for (BossProjectile* nuvem : Nuvens()) nuvem->Kill();
    if (!mNomeDaFaseDoFork.empty()) RegistrarAnuncio(kFaseDoFork, mNomeDaFaseDoFork);
}

void Vitor::ComecarOFork() {
    auto* batalha = dynamic_cast<Battle*>(mScene);
    const auto nuvens = Nuvens();
    if (!batalha || nuvens.empty()) return;   // sem nuvem, o fork de sempre

    // AS DUAS NUVENS SE JUNTAM EM CIMA DELE (AtualizarANuvemDoFork as faz
    // crescer e processar). A Ancorar nova toma o lugar da que as prendia dos
    // lados, e o fim dela fica depois do fim do fork: quem as desfaz e
    // SairDaNuvem, e nao a queda.
    mEscalaDaNuvem = nuvens.front()->GetScale();
    for (BossProjectile* nuvem : nuvens) {
        nuvem->insertMotion<AncorarBehavior>(Vector2::Zero, kFork.sai + 0.5f, 200.0f);
    }

    // ELE ENTRA quando a nuvem chega, e fica onde esta. O OUTRO CORPO nasce
    // dentro dela - os dois processos de um fork saem do mesmo lugar - e,
    // quando ela abre, anda ate o lugar em que Battle::DividirChefe o pos, o
    // espelho do original: dali em diante a fase e a de sempre.
    const Vector2 aqui = GetPosition();
    EntrarNaNuvem(aqui, aqui, false);
    for (Boss* corpo : batalha->Corpos()) {
        if (corpo == this) continue;
        if (auto* outro = dynamic_cast<Vitor*>(corpo)) outro->EntrarNaNuvem(aqui, outro->GetPosition(), true);
    }

    if (mNomeDaFaseDoFork.empty()) mNomeDaFaseDoFork = AnuncioDaFase(kFaseDoFork);
    RegistrarAnuncio(kFaseDoFork, kNoFork);
}

void Vitor::AtualizarANuvemDoFork() {
    // A NUVEM CRESCE enquanto as duas se juntam, e cobre o professor quando ele
    // entra; processa ate abrir, e abre ociosa - o fork acabou.
    const float u = kFork.entra > 0.0f ? std::min(1.0f, mTempoNoFork / kFork.entra) : 1.0f;
    const float escala = mEscalaDaNuvem + (kEscalaDaNuvemJunta - mEscalaDaNuvem) * u;
    const char* cara = mTempoNoFork < kFork.abre ? Nuvem::kCaraProcessando : Nuvem::kCaraOciosa;
    for (BossProjectile* nuvem : Nuvens()) {
        nuvem->SetScale(escala);
        if (auto* anim = nuvem->GetComponent<DrawAnimatedComponent>(); anim && anim->GetAnimation() != cara) {
            anim->SetAnimation(cara);
        }
    }
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
            // A FASE ACABOU. Se a seguinte e o fork, as nuvens sao a porta dele;
            // se nao, elas se desfazem. O placar volta ao nome.
            if (!mNomeDaFaseDasNuvens.empty()) RegistrarAnuncio(kFaseDasNuvens, mNomeDaFaseDasNuvens);
            mNasNuvens = false;
            if (fsm && fsm->GetStateName() == kFaseDoFork && !EhClone()) {
                ComecarOFork();
            } else {
                for (BossProjectile* nuvem : Nuvens()) nuvem->Kill();
            }
        }
        return;
    }
    if (!mNasNuvens) {
        mNasNuvens = true;
        mNuvensFormadas = false;
        mTempoNasNuvens = 0.0f;
        mSemNuvensHa = 0.0f;
        mChuva.t = 0.0f;
        if (mNomeDaFaseDasNuvens.empty()) mNomeDaFaseDasNuvens = AnuncioDaFase(kFaseDasNuvens);
    }

    mTempoNasNuvens += deltaTime;
    if (!mNuvensFormadas) {
        if (mTempoNasNuvens < kFormaAsNuvens) return;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mNuvensFormadas = true;
        mChuva.t = 0.0f;
        return;     // as nuvens recem-disparadas so entram no campo no quadro seguinte
    }

    // A NUVEM FORA DO AR: o Ponto Extra levou as duas. O placar avisa, e elas
    // voltam kVoltaDasNuvens depois, com o ciclo do comeco.
    if (Nuvens().empty()) {
        mSemNuvensHa += deltaTime;
        if (AnuncioDaFase(kFaseDasNuvens) != kForaDoAr) RegistrarAnuncio(kFaseDasNuvens, kForaDoAr);
        if (mSemNuvensHa < Nuvem::kVoltaDasNuvens) return;
        DispararNuvem(Nuvem::kGatilhoDasNuvens);
        mChuva.t = 0.0f;
        mSemNuvensHa = 0.0f;
        return;
    }
    mSemNuvensHa = 0.0f;

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
