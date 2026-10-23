//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#include "Vladimir.h"

#include <limits>

#include "../../../Random.h"
#include "../../../Attacks/Alfabeto.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"

Vladimir::Vladimir(Scene* scene) : Boss(scene) {
}

void Vladimir::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarAutomato();
}

std::string Vladimir::AvisoDoIntercambio() const {
    return Alfabeto::kAvisoDaTroca;
}

std::optional<Vector2> Vladimir::AlvoDoMovimento() const {
    return mAlvo;
}

std::string Vladimir::CaraDoEstado(const int q, const bool aceso) {
    return "Q" + std::to_string(q) + (aceso ? "Aceso" : "");
}

Vector2 Vladimir::CentroDoEstado(const int q) {
    const auto i = static_cast<size_t>(q < 0 || q >= Automato::kEstados ? 0 : q);
    return Vector2(Automato::kXDosEstados[i], Automato::kYDosEstados[i]);
}

std::vector<BossProjectile*> Vladimir::NoPosto(const char* posto) const {
    std::vector<BossProjectile*> achados;
    const auto* batalha = dynamic_cast<Battle*>(mScene);
    if (!batalha || !batalha->GetProjectileManager()) return achados;
    for (const auto& p : batalha->GetProjectileManager()->GetBossProjectiles()) {
        if (p && p->GetState() == ActorState::Active && p->Posto() == posto) achados.push_back(p.get());
    }
    return achados;
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

    // E O QUE O AUTOMATO AINDA VAI LER TAMBEM TROCA: o placar muda, e com ele
    // o caminho do professor. O que ja foi lido fica como foi.
    if (mNoAutomato && mMomento == Momento::Lendo) {
        Automato::Trocar(mLeitura);
        RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
    }
}

void Vladimir::Formar(const char* gatilho, const Vector2 onde, std::string cara) {
    mFormando = true;
    mDisparoDoFormando = onde;
    mCaraDoFormando = std::move(cara);
    Disparar(gatilho);
    mFormando = false;
}

void Vladimir::AtualizarAutomato() {
    const auto* fsm = GetComponent<FSMComponent>();
    const bool noAutomato = fsm && fsm->GetStateName() == Automato::kFaseDoAutomato && !EhClone();

    if (!noAutomato) {
        // A FASE ACABOU: o automato se desfaz, o placar volta ao que o arquivo
        // diz, e o professor deixa de ter para onde ir.
        if (mNoAutomato) {
            for (BossProjectile* p : NoPosto(Automato::kPostoDosEstados)) p->Kill();
            if (!mAnuncioDoAutomato.empty()) RegistrarAnuncio(Automato::kFaseDoAutomato, mAnuncioDoAutomato);
            mNoAutomato = false;
            mAlvo.reset();
        }
        return;
    }
    if (!mNoAutomato) {
        mNoAutomato = true;
        mAutomatoFormado = false;
        mMomento = Momento::Anuncio;
        mProximoPasso = kPrimeiraPalavra;
        mLeitura = Automato::Leitura{};
        if (mAnuncioDoAutomato.empty()) mAnuncioDoAutomato = AnuncioDaFase(Automato::kFaseDoAutomato);
        RegistrarAnuncio(Automato::kFaseDoAutomato, mAnuncioDoAutomato);
        mAlvo = CentroDoEstado(Automato::kInicial);
    }
    const float agora = fsm->GetStateTime();

    if (!mAutomatoFormado && agora >= kFormaOAutomato) {
        Formar(Automato::kGatilhoDoDiagrama, Vector2(Automato::kXDoDiagrama, Automato::kYDoDiagrama), "");
        for (int q = 0; q < Automato::kEstados; ++q) {
            Formar(Automato::kGatilhoDosEstados, CentroDoEstado(q), CaraDoEstado(q, false));
        }
        mAutomatoFormado = true;
    }

    if (agora >= mProximoPasso) {
        mProximoPasso += (mMomento == Momento::Lendo && Automato::Terminou(mLeitura)) ? kVeredito : kPasso;

        if (mMomento != Momento::Lendo) {
            // UMA PALAVRA NOVA, sempre do estado inicial: o professor volta ao
            // q0 sem atirar, e o placar mostra o que ele vai ler.
            const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
            mLeitura = Automato::Leitura{Automato::Sortear(sorteio), 0, Automato::kInicial};
            mMomento = Momento::Lendo;
            mAlvo = CentroDoEstado(Automato::kInicial);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else if (!Automato::Terminou(mLeitura)) {
            // UM SIMBOLO: a letra sai do estado em que ele esta, e ele voa.
            const Automato::Passo passo = Automato::Ler(mLeitura);
            Disparar(passo.letra == 'a' ? Automato::kGatilhoDoA : Automato::kGatilhoDoB);
            mAlvo = CentroDoEstado(passo.para);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Configuracao(mLeitura));
        } else {
            // O VEREDITO: aceita, e o estado de aceitacao estoura num anel; ou
            // rejeita, e saem perseguidores de onde ele parou.
            Disparar(Automato::Aceita(mLeitura.estado) ? Automato::kGatilhoDeAceitar
                                                       : Automato::kGatilhoDeRejeitar);
            RegistrarAnuncio(Automato::kFaseDoAutomato, Automato::Veredito(mLeitura));
            mMomento = Momento::Veredito;
        }
    }

    // O ESTADO EM QUE ELE ESTA FICA ACESO - lendo, ou no veredito.
    const bool lendo = mMomento != Momento::Anuncio;
    for (BossProjectile* p : NoPosto(Automato::kPostoDosEstados)) {
        auto* anim = p->GetComponent<DrawAnimatedComponent>();
        if (!anim || anim->GetAnimation() == Automato::kCaraDoDiagrama) continue;
        for (int q = 0; q < Automato::kEstados; ++q) {
            if ((p->GetPosition() - CentroDoEstado(q)).LengthSq() > 1.0f) continue;
            const std::string cara = CaraDoEstado(q, lendo && q == mLeitura.estado);
            if (anim->GetAnimation() != cara) anim->SetAnimation(cara);
        }
    }
}

void Vladimir::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                    const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mFormando) return;

    // O DIAGRAMA E OS ESTADOS SAO O MAPA, e nao tiro: ninguem se machuca neles,
    // e o tiro do aluno os atravessa.
    for (auto& p : projectiles) {
        auto* parte = dynamic_cast<BossProjectile*>(p.get());
        if (!parte) continue;
        parte->DefinirEnfeite(true);
        if (auto* anim = parte->GetComponent<DrawAnimatedComponent>(); anim && !mCaraDoFormando.empty()) {
            anim->SetAnimation(mCaraDoFormando);
        }
    }
}

void Vladimir::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    if (stateName != Automato::kFaseDoAutomato) return;

    // O MAPA NASCE ONDE FICA.
    if (mFormando) {
        params.firePosition = mDisparoDoFormando;
        return;
    }

    // A LETRA LIDA SAI MIRADA NO ALUNO, com um tiro no meio do leque: num leque
    // par o meio cai entre dois tiros, e quem fica parado ficaria na fresta
    // (ver Ricardo::CustomizeAttackParams). O anel da aceitacao ignora o angulo.
    const Vector2 d = GetDirectionToPlayer();
    const bool par = params.numProjectiles > 1 && params.numProjectiles % 2 == 0;
    const float meioPasso = par ? 0.5f * params.angle / static_cast<float>(params.numProjectiles - 1) : 0.0f;
    params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x)) + meioPasso;
}
