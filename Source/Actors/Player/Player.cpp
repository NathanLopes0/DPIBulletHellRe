//
// Created by nslop on 03/10/2024.
//
#include <algorithm>
#include <vector>

#include "Player.h"
#include "../../Entrada/Painel.h"
#include "../../Regras/Poder.h"

#include "PlayerProjectile.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

#include "../../Motor/CaminhosArquivo.h"
#include "../../Personagem/ComporPersonagem.h"
#include "../../Motor/Game.h"
#include "../../Personagem/PersonagensArquivo.h"

#define NUM_PROJECTILES_VECTOR 500
#define ATK_TIMER_START_FULL PontoExtra::kEntreTiros   // o Ponto Extra paga por eles
#define PLAYER_SPEED_START 300.0f
#define BASE_SHOOT_FORWARD_SPEED 800.0f
#define BLINK_INVINCIBLE_FREQUENCY 0.1f

Player::Player(Scene* scene, const size_t quem) :
    // Na ordem em que os membros sao declarados - mQuem e declarado depois de mMoving, e e nessa
    // ordem que o compilador inicializa de qualquer jeito.
    Actor(scene),
    playerSpeed(PLAYER_SPEED_START),
    atkTimer(ATK_TIMER_START_FULL),
    invincibleTimer(0.0f),
    isInvincible(false),
    mBlinkTimer(BLINK_INVINCIBLE_FREQUENCY),
    mMoving(false),
    mQuem(quem)

{

    // A aparencia vem do perfil, e nao de um caminho fixo.
    std::string spritePath = Caminhos::Asset("Player/DPIBHPlayer.png");
    std::string dataPath   = Caminhos::Asset("Player/DPIBHPlayer.json");

    Game* game = GetScene()->GetGame();
    const Personagens::Composta composta =
        // A aparencia da sessao deste aluno.
        Personagens::Compor(game, Personagens::Carregado(), game->AparenciaDe(mQuem));

    if (composta.ok) {
        spritePath = composta.chaveDaTextura;   // ja esta no cache de texturas
        dataPath   = composta.atlas;
    }
    else {
        // Um catalogo faltando ou uma arte ilegivel nao podem impedir alguem de jogar.
        SDL_Log("PLAYER: a personagem nao pode ser composta; usando o sprite antigo.");
    }

    auto drawComp = AddComponent<DrawAnimatedComponent>(spritePath, dataPath);
    drawComp->AddAnimation("Moving", {0,1,2,3});
    drawComp->AddAnimation("Idle", {1});
    drawComp->SetAnimation("Idle");

    AddComponent<RigidBodyComponent>();
    auto colliderComp = AddComponent<CircleColliderComponent>(static_cast<float>(drawComp->GetSpriteWidth()) / 10.f);
    colliderComp->SetTag(ColliderTag::Player);

}

void Player::OnProcessInput(const Uint8 *keyState) {

    MoveInput(keyState);
    ShootInput(keyState);
    SpecialInput(keyState);

}

void Player::OnUpdate(float deltaTime) {

    InvincibleUpdate(deltaTime);
    mAcertos.Avancar(deltaTime);
    HandleAnimation();
    DecreaseAtkTimer(deltaTime);
    BorderLimitCheck();
    // Cada um so anda o relogio dele; quem pinta e um so.
    UpdateCorrente(deltaTime);
    UpdateOverload(deltaTime);
    UpdateImunidade(deltaTime);
    AtualizarCorDoEstado();

}

void Player::InvincibleUpdate(float deltaTime) {
    if (!isInvincible) return;

    if (invincibleTimer == 0.0f) {
        mBlinkTimer = BLINK_INVINCIBLE_FREQUENCY;
    }

    invincibleTimer += deltaTime;
    // O tempo depende do modo (Dificuldade::Invencibilidade).
    const Dificuldade::Nivel nivel = (mScene && mScene->GetGame()) ? mScene->GetGame()->NivelDeDificuldade()
                                                                   : Dificuldade::Nivel::Normal;
    if (invincibleTimer >= Dificuldade::Invencibilidade(nivel)) {
        isInvincible = false;
        invincibleTimer = 0.0f;
        GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
        mBlinkTimer = BLINK_INVINCIBLE_FREQUENCY;
        return;
    }

    auto blinking = GetComponent<DrawAnimatedComponent>()->GetIsVisible();
    mBlinkTimer += deltaTime;
    if (mBlinkTimer >= BLINK_INVINCIBLE_FREQUENCY) {
        mBlinkTimer = 0.0f;
        GetComponent<DrawAnimatedComponent>()->SetIsVisible(!blinking);
        blinking = !blinking;
    }

}
void Player::MoveInput(const Uint8 *keyState) {

    float newXSpeed = 0;
    float newYSpeed = 0;

    // O manche deste aluno.
    const Painel::Jogador kQuem = MeuPainel();

    // A velocidade e lida com o fator, e playerSpeed fica intacto.
    const float velocidade = VelocidadeDeAndar();

    // ---- Verificação de movimento cima/baixo ----
    if(Painel::Apertado(keyState, kQuem, Painel::Direcao::Cima))
        newYSpeed -= velocidade;
    else if(Painel::Apertado(keyState, kQuem, Painel::Direcao::Baixo))
        newYSpeed += velocidade;

    // ---- Verificação de movimento esquerda/direita ----
    if(Painel::Apertado(keyState, kQuem, Painel::Direcao::Esquerda))
        newXSpeed -= velocidade;
    else if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Direita))
        newXSpeed += velocidade;

    // Vendo se está movendo ou não para mudar a variável mMoving (pra mudar a animação de acordo)
    if(newXSpeed == 0 && newYSpeed == 0) mMoving = false;
    else mMoving = true;

    GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(newXSpeed, newYSpeed));
}
void Player::ShootInput(const Uint8 *keyState) {

    // Botao 1 do painel deste aluno.
    if(Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Um) && mOverloadTimer <= 0.0f) {
        Shoot();
    }

}
void Player::SpecialInput(const Uint8 *keyState) {

    // Botao 2 do painel deste aluno.
    if(Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Dois) && mExtraPoints > 0 && mEsperaDoPontoExtra <= 0.0f) {
        mExtraPoints--;
        const auto battle = dynamic_cast<Battle*>(GetScene());
        // So o solo sobrecarrega.
        const bool sozinho = !battle || !battle->OutroJogador(this);
        mEsperaDoPontoExtra = sozinho ? OVERLOAD_DURATION : IMUNIDADE_DO_PRESENTE;
        if (sozinho) mOverloadTimer = OVERLOAD_DURATION;
        if (battle) battle->OnPlayerUsedExtraPoint(this);
    }

}

void Player::HandleAnimation() {

    if(mMoving) GetComponent<DrawAnimatedComponent>()->SetAnimation("Moving");
    else GetComponent<DrawAnimatedComponent>()->SetAnimation("Idle");

}

// ShootInput Subfunction.
void Player::Shoot() {

    if (atkTimer > 0) return;

    auto battle = dynamic_cast<Battle*>(GetScene());
    if (!battle) return;

    auto projManager = battle->GetProjectileManager();
    if (!projManager) return;

    // TODO - MATANDO o Object Pooling - mudar pra chamada de função em ProjectileManager
    auto projectile = std::make_unique<PlayerProjectile>(GetScene(), this, mQuem);

    // Posiciona o projétil na frente do jogador.
    projectile->SetPosition(GetPosition());

    // Define velocidade inicial (sobe)
    projectile->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0.0f, -BASE_SHOOT_FORWARD_SPEED));

    // Ativa e torna visível.
    projectile->ActivateProjectile();

    // Entrega ownership ao manager.
    projManager->AddPlayerProjectile(std::move(projectile));
    ++mDisparos;

    // Toca audio.
    const auto audioHandler = mScene->GetGame()->GetAudio()->PlaySound("plst00.wav", false);
    mScene->GetGame()->GetAudio()->SetSoundVolume(audioHandler, 10);

    atkTimer = ATK_TIMER_START_FULL;
}

// OnUpdate Subfunction.
void Player::DecreaseAtkTimer(float deltaTime) {
    if(atkTimer > 0)
        atkTimer -= deltaTime;
}

void Player::BorderLimitCheck() {
    SDL_FRect bounds = mScene->GetPlayfieldBounds();
    auto pWidth = GetComponent<DrawAnimatedComponent>()->GetSpriteWidth();
    auto pHeight = GetComponent<DrawAnimatedComponent>()->GetSpriteHeight();

    const float esquerda = bounds.x + static_cast<float>(pWidth);
    const float direita = bounds.x + bounds.w - static_cast<float>(pWidth);
    float x = GetPosition().x;

    // A rede do Vitor (o socket(), como orientador).
    if (auto* batalha = dynamic_cast<Battle*>(mScene); batalha && batalha->RedeAberta() &&
        (x < esquerda || x > direita)) {
        const Vector2 de(x < esquerda ? esquerda : direita, GetPosition().y);
        x = Poder::Atravessar(x, esquerda, direita);
        batalha->Atravessou(de, Vector2(x, GetPosition().y));
    }

    SetPosition(Vector2(Math::Clamp(x, esquerda, direita),
            Math::Clamp(GetPosition().y, bounds.y + pHeight, bounds.y + bounds.h - pHeight)));
}

void Player::UpdateOverload(float deltaTime) {

    // So o relogio e o pulso.
    mEsperaDoPontoExtra = std::max(0.0f, mEsperaDoPontoExtra - deltaTime);

    if (mOverloadTimer <= 0.0f) {
        mPulseTimer = 0.0f;
        return;
    }

    mOverloadTimer -= deltaTime;

    // Quanto menor o timer (mais perto de acabar), maior a frequencia.
    const float freq = 5.0f + (OVERLOAD_DURATION - mOverloadTimer) * 15.0f;
    mPulseTimer += freq * deltaTime;
}

void Player::Proteger(const float segundos) {

    // O maior dos dois, e nao a soma.
    if (segundos > mImunidadeTimer) mImunidadeTimer = segundos;
}

void Player::Acorrentar(const float segundos, const float fatorDeVelocidade) {

    // O maior dos dois, como em Proteger.
    if (segundos > mCorrenteTimer) {
        mCorrenteTimer = segundos;

        // O fator acompanha o timer vencedor.
        mFatorDaCorrente = fatorDeVelocidade;
    }
}

void Player::UpdateCorrente(const float deltaTime) {

    if (mCorrenteTimer <= 0.0f) return;

    mCorrenteTimer -= deltaTime;

    if (mCorrenteTimer <= 0.0f) {
        mCorrenteTimer = 0.0f;
        mFatorDaCorrente = 1.0f;
        mPulsoDaCorrente = 0.0f;
        return;
    }

    // Ela escurecia de 120 a 155 com um azul de leve, "porque peso se le apagado, nao agitado".
    const float fracaoRestante = mCorrenteTimer / kDuracaoTipicaDaCorrente;
    mPulsoDaCorrente += (4.0f + (1.0f - fracaoRestante) * 14.0f) * deltaTime;

    // Quem pinta e AtualizarCorDoEstado; aqui so anda o pulso.
}

void Player::UpdateImunidade(const float deltaTime) {

    // So o relogio e o pulso.
    if (mImunidadeTimer <= 0.0f) return;

    mImunidadeTimer -= deltaTime;
    if (mImunidadeTimer <= 0.0f) {
        mImunidadeTimer = 0.0f;
        mPulsoDaImunidade = 0.0f;
        return;
    }

    // O mesmo pulso do overload, e as duas se distinguem pela cor e nao pelo ritmo.
    const float freq = 5.0f + (IMUNIDADE_DO_PRESENTE - mImunidadeTimer) * 15.0f;
    mPulsoDaImunidade += freq * deltaTime;
}

void Player::AtualizarCorDoEstado() {

    const auto dc = GetComponent<DrawAnimatedComponent>();
    if (!dc) return;

    // A ordem dos ifs e a prioridade, e esta escrita - ver o cabecalho.

    if (mImunidadeTimer > 0.0f) {
        const auto rg = static_cast<Uint8>((Math::Sin(mPulsoDaImunidade) + 1.0f) * 127.5f);
        dc->SetColor(rg, rg, 255);
        return;
    }

    if (mOverloadTimer > 0.0f) {
        const auto gb = static_cast<Uint8>((Math::Sin(mPulseTimer) + 1.0f) * 127.5f);
        dc->SetColor(255, gb, gb);
        return;
    }

    if (mCorrenteTimer > 0.0f) {
        // Ferro, escuro e pulsando.
        const float osc = (Math::Sin(mPulsoDaCorrente) + 1.0f) * 0.5f;
        const auto escuro = static_cast<Uint8>(70.0f + osc * 45.0f);
        const auto azul   = static_cast<Uint8>(150.0f + osc * 60.0f);
        dc->SetColor(escuro, escuro, azul);
        return;
    }

    dc->SetColor(255, 255, 255);
}

void Player::AddExtraPoint() {
    mExtraPoints = Math::Clamp(mExtraPoints + 1, 0, 3);
}
