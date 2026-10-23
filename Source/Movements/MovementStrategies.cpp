//
// Created by gensh on 24/11/2025.
//

#include "MovementStrategies.h"
#include "Escada.h"
#include "../Attacks/Logica.h"
#include "../Attacks/Relogio.h"
#include "../Components/AIComponents/FSMComponent.h"
#include "../Scenes/Battle/Battle.h"
#include "../Actors/Player/Player.h"

// =========================================================
// RANDOM WANDER STRATEGY
// =========================================================

RandomWanderStrategy::RandomWanderStrategy(float interval, float speed, float margin)
    : mState(Idle), mChangeInterval(interval), mMoveSpeed(speed),
      mMargin(margin), mWaitTimer(0.0f), mIsMoving(false)
{
}
void RandomWanderStrategy::Initialize(Boss* boss) {
    PickNewTarget(boss);
}
void RandomWanderStrategy::Update(Boss* boss, float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;

    mHoverTimer += deltaTime;

    if (mState == Moving) {
        Vector2 toTarget = mTargetPos - boss->GetPosition();
        float dist = toTarget.Length();

        if (dist < 10.0f) {
            mState = Idle;

            // Ex: Se configurou 3.0s, ele espera entre 2.4s e 3.6s.
            float minWait = mChangeInterval * 0.8f;
            float maxWait = mChangeInterval * 1.2f;
            mWaitTimer = Random::GetFloatRange(minWait, maxWait);

            rb->SetVelocity(Vector2::Zero);
        } else {
            toTarget.Normalize();
            float slowFactor = Math::Min(1.0f, dist / 150.0f);
            rb->SetVelocity(toTarget * (mMoveSpeed * slowFactor));
        }
    }
    else if (mState == Idle) {
        mWaitTimer -= deltaTime;
        const float hoverVy = std::cos(mHoverTimer * HOVER_FREQ) * HOVER_AMP;
        rb->SetVelocity(Vector2(0.0f, hoverVy));
        if (mWaitTimer <= 0.0f) {
            PickNewTarget(boss);
        }
    }
}
void RandomWanderStrategy::PickNewTarget(Boss* boss) {
    auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());

    mTargetPos = Vector2(
        Random::GetFloatRange(mMargin, w - mMargin),
        Random::GetFloatRange(mMargin, h / 2.5f)
    );
    mState = Moving;
}

// =========================================================
// HOVER ABOVE PLAYER STRATEGY
// =========================================================

HoverAbovePlayerStrategy::HoverAbovePlayerStrategy(float speed, float yLevel)
    : mSpeed(speed), mPreferredY(yLevel)
{
}

void HoverAbovePlayerStrategy::Update(Boss* boss, float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();

    mHoverTimer += deltaTime;

    float targetX = boss->GetPosition().x;

    if (const auto battle = dynamic_cast<Battle*>(boss->GetScene())) {
        if (const auto player = battle->Alvo()) {
            targetX = player->GetPosition().x;
        }
    }

    Vector2 currentPos = boss->GetPosition();
    float hoverOffset = Math::Sin(mHoverTimer * 2.f) * 15.f;

    Vector2 desiredPos = Vector2(targetX, mPreferredY + hoverOffset);

    Vector2 toTarget = desiredPos - currentPos;

    Vector2 desiredVel = toTarget;
    if(desiredVel.Length() > 1.0f) desiredVel.Normalize();
    desiredVel *= mSpeed;

    Vector2 currentVel = rb->GetVelocity();
    Vector2 newVel = Vector2::Lerp(currentVel, desiredVel, 5.0f * deltaTime); // Aumentei para 5.0f pra ficar mais ágil

    rb->SetVelocity(newVel);
}

// =========================================================
// GO TO CENTER STRATEGY
// =========================================================

void GoToCenterStrategy::Initialize(Boss* boss) {
    mReached = false;
}

void GoToCenterStrategy::Update(Boss* boss, float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (mReached) {
        rb->SetVelocity(Vector2::Zero);
        return;
    }

    auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());
    auto center = Vector2(w / 2.0f, h / 5.0f);

    Vector2 toTarget = center - boss->GetPosition();
    float dist = toTarget.Length();

    if (dist < 5.0f) {
        mReached = true;
        rb->SetVelocity(Vector2::Zero);
    } else {
        toTarget.Normalize();
        float slowFactor = Math::Min(1.0f, dist / 100.0f);
        rb->SetVelocity(toTarget * (200.0f * slowFactor));
    }
}
// =========================================================
// ORBITAR  -  o gesto da estrela de Kleene
// =========================================================

OrbitarStrategy::OrbitarStrategy(const float raio, const float voltasPorSegundo)
    : mRaio(raio), mVoltasPorSegundo(voltasPorSegundo)
{
}

void OrbitarStrategy::Initialize(Boss* boss) {

    const auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    const auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());

    // O CENTRO E DA TELA, E NAO ONDE O CHEFE ESTAVA. Capturar a posicao de
    // entrada faria a orbita nascer torta - deslocada para o lado em que a fase
    // anterior tiver largado o chefe - e metade do anel sairia da tela.
    mCentro = Vector2(w / 2.0f, h * 0.30f);

    // COMECA NO ANGULO EM QUE O CHEFE JA ESTA, para a primeira volta nao
    // comecar com um salto atravessando a tela.
    const Vector2 doCentro = boss->GetPosition() - mCentro;
    mAngulo = Math::NearZero(doCentro.Length()) ? 0.0f
                                                : Math::Atan2(doCentro.y, doCentro.x);
}

void OrbitarStrategy::Update(Boss* boss, const float deltaTime) {

    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;

    mAngulo += 2.0f * Math::Pi * mVoltasPorSegundo * deltaTime;

    const Vector2 alvo = mCentro + Vector2(Math::Cos(mAngulo), Math::Sin(mAngulo)) * mRaio;

    // PERSEGUE O PONTO DA ORBITA em vez de receber a velocidade tangente. Com a
    // tangente, qualquer erro de integracao se acumula e o chefe sai em
    // espiral; perseguindo, ele volta sozinho para o anel.
    rb->SetVelocity((alvo - boss->GetPosition()) * 6.0f);
}

// =========================================================
// VARREDURA  -  o gesto de a^n b^n e da fita de Turing
// =========================================================

VarreduraStrategy::VarreduraStrategy(const float velocidade, const float margem)
    : mVelocidade(velocidade), mMargem(margem)
{
}

void VarreduraStrategy::Initialize(Boss* boss) {

    const auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());
    mAltura = h * 0.18f;

    // Comeca indo para o lado MAIS LONGE: assim a primeira passada e a longa, e
    // o jogador ve a varredura inteira antes de ela inverter.
    const auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    mDirecao = boss->GetPosition().x < w / 2.0f ? 1.0f : -1.0f;
}

void VarreduraStrategy::Update(Boss* boss, const float deltaTime) {

    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;

    const auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    const float x = boss->GetPosition().x;

    // A VIRADA E POR POSICAO, e nao por um cronometro: assim a varredura cobre
    // a largura inteira seja qual for a velocidade, e mexer na velocidade nao
    // obriga a reacertar um tempo junto.
    if (x > w - mMargem) mDirecao = -1.0f;
    else if (x < mMargem) mDirecao = 1.0f;

    // O y e corrigido junto, e nao so o x: sem isto o chefe desce de vez com o
    // empurrao de qualquer coisa e a varredura deixa de ser uma linha.
    const float correcaoY = (mAltura - boss->GetPosition().y) * 3.0f;
    rb->SetVelocity(Vector2(mVelocidade * mDirecao, correcaoY));
}

// =========================================================
// MERGULHO  -  o gesto da pilha
// =========================================================

MergulhoStrategy::MergulhoStrategy(const float velocidade, const float profundidade)
    : mVelocidade(velocidade), mProfundidade(profundidade)
{
}

void MergulhoStrategy::Initialize(Boss* boss) {
    const auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());
    mTopo = h * 0.16f;
    mDirecao = 1.0f;   // comeca empilhando, isto e, descendo
}

void MergulhoStrategy::Update(Boss* boss, const float deltaTime) {

    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;

    const float y = boss->GetPosition().y;

    if (y > mTopo + mProfundidade) mDirecao = -1.0f;   // fundo: desempilha
    else if (y < mTopo)            mDirecao = 1.0f;    // topo: empilha de novo

    // E DERIVA DE LADO ENQUANTO MERGULHA, em vez de ficar preso ao centro.
    //
    // A primeira versao puxava o x de volta ao meio, e com isso o chefe
    // empilhava sempre na mesma coluna. Medido, a fase dava ZERO acerto em
    // dezessete segundos contra um jogador que o seguisse - e o motivo nao era
    // a quantidade de simbolos (subi de 10 para 18 sem mudar nada):
    //
    // CircleSpreadAttack distribui os angulos em i * 360/n A PARTIR DE ZERO,
    // sempre. As frestas do anel ficam nas MESMAS direcoes em todo disparo, e
    // com o chefe numa coluna fixa o jogador que estivesse numa fresta ficava
    // nela para sempre. Com 18 simbolos o passo e 20 graus e nao ha projetil
    // nenhum a 90 - exatamente para baixo, que e onde o jogador estava.
    //
    // A deriva resolve sem mexer no motor: o anel continua com as mesmas
    // frestas, mas elas passam a varrer o campo porque a ORIGEM anda. E enche a
    // largura, que e o outro motivo de a fase parecer parada.
    const auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    const float x = boss->GetPosition().x;

    if (x > w - kMargemDaDeriva)      mDeriva = -1.0f;
    else if (x < kMargemDaDeriva)     mDeriva = 1.0f;

    rb->SetVelocity(Vector2(kVelocidadeDaDeriva * mDeriva, mVelocidade * mDirecao));
}

EscadaStrategy::EscadaStrategy(const float segundosPorDegrau, const float degraus)
    : mSegundosPorDegrau(segundosPorDegrau), mDegraus(static_cast<int>(degraus))
{
}

void EscadaStrategy::Initialize(Boss* boss) {
    (void)boss;
    mTempo = 0.0f;
}

void EscadaStrategy::Update(Boss* boss, const float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;
    mTempo += deltaTime;

    Escada::Geometria g;
    g.largura = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    const Vector2 alvo = Escada::Degrau(Escada::DegrauNoTempo(mTempo, mSegundosPorDegrau, mDegraus),
                                        mDegraus, g);

    // VAI ATE O DEGRAU E PARA NELE. O ultimo passo e so o que falta, para nao
    // tremer em volta do ponto.
    const Vector2 falta = alvo - boss->GetPosition();
    const float d = falta.Length();
    const float passo = kVelocidadeEntreDegraus * deltaTime;
    if (d <= passo || deltaTime <= 0.0f) {
        boss->SetPosition(alvo);
        rb->SetVelocity(Vector2::Zero);
        return;
    }
    rb->SetVelocity(falta * (kVelocidadeEntreDegraus / d));
}

// =========================================================
// DESLOCAMENTO STRATEGY
// =========================================================

DeslocamentoStrategy::DeslocamentoStrategy(const float segundosPorPasso, const float colunas)
    : mSegundosPorPasso(segundosPorPasso), mColunas(static_cast<int>(colunas))
{
}

void DeslocamentoStrategy::Initialize(Boss* boss) {
    mTempo = 0.0f;
    mAltura = boss->GetPosition().y;
    const auto largura = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    mInicio = Logica::Coluna(boss->GetPosition().x, largura, mColunas);
}

void DeslocamentoStrategy::Update(Boss* boss, const float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb || mColunas <= 0) return;
    mTempo += deltaTime;

    const auto largura = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    const int coluna = Logica::ColunaDoDeslocamento(mTempo, mSegundosPorPasso, kColunasDaBorda,
                                                    mColunas - 1 - kColunasDaBorda, mInicio);
    const Vector2 alvo(Logica::CentroDaColuna(coluna, largura, mColunas), mAltura);

    // VAI ATE A COLUNA E PARA NELA, como a escada.
    const Vector2 falta = alvo - boss->GetPosition();
    const float d = falta.Length();
    const float passo = kVelocidadeDoPasso * deltaTime;
    if (d <= passo || deltaTime <= 0.0f) {
        boss->SetPosition(alvo);
        rb->SetVelocity(Vector2::Zero);
        return;
    }
    rb->SetVelocity(falta * (kVelocidadeDoPasso / d));
}

// =========================================================
// ONDA QUADRADA STRATEGY  -  o gesto do clock
// =========================================================

OndaQuadradaStrategy::OndaQuadradaStrategy(const float amplitude, const float velocidade)
    : mAmplitude(amplitude), mVelocidade(velocidade)
{
}

void OndaQuadradaStrategy::Initialize(Boss* boss) {
    const auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());
    const auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    mAltura = h * 0.18f;
    mDirecao = boss->GetPosition().x < w / 2.0f ? 1.0f : -1.0f;
}

void OndaQuadradaStrategy::Update(Boss* boss, const float deltaTime) {
    (void)deltaTime;
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;

    // O CLOCK E O DO PLACAR, e o tempo e o da fase: o mesmo relogio que faz os
    // tiros andarem (Ricardo::AtualizarRelogio).
    const float f = Relogio::LerFrequencia(boss->AnuncioDaFase(boss->FaseAtual())).value_or(1.0f);
    const auto* fsm = boss->GetComponent<FSMComponent>();
    const float t = fsm ? fsm->GetStateTime() : 0.0f;
    const float alvoY = mAltura + (Relogio::Alto(t, f) ? -0.5f : 0.5f) * mAmplitude;

    const auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    const float x = boss->GetPosition().x;
    if (x > w - kMargem) mDirecao = -1.0f;
    else if (x < kMargem) mDirecao = 1.0f;

    rb->SetVelocity(Vector2(mVelocidade * mDirecao, (alvoY - boss->GetPosition().y) * kRapidezDaBorda));
}

// =========================================================
// CONDUZIDO STRATEGY  -  vai aonde o professor diz
// =========================================================

ConduzidoStrategy::ConduzidoStrategy(const float velocidade)
    : mVelocidade(velocidade)
{
}

void ConduzidoStrategy::Update(Boss* boss, const float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;
    const auto alvo = boss->AlvoDoMovimento();
    if (!alvo || mVelocidade <= 0.0f) {
        rb->SetVelocity(Vector2::Zero);
        return;
    }

    // VAI ATE O ALVO E PARA NELE, como o deslocamento: sem passar do ponto.
    const Vector2 falta = *alvo - boss->GetPosition();
    const float d = falta.Length();
    if (d <= mVelocidade * deltaTime || deltaTime <= 0.0f) {
        boss->SetPosition(*alvo);
        rb->SetVelocity(Vector2::Zero);
        return;
    }
    rb->SetVelocity(falta * (mVelocidade / d));
}
