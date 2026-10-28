//
// Created by nslop on 03/10/2024.
//
#include <algorithm>
#include <vector>

#include "Player.h"
#include "../../Painel.h"
#include "../../Poder.h"

#include "PlayerProjectile.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

#include "../../CaminhosArquivo.h"
#include "../../ComporPersonagem.h"
#include "../../Game.h"
#include "../../PersonagensArquivo.h"

#define NUM_PROJECTILES_VECTOR 500
#define ATK_TIMER_START_FULL PontoExtra::kEntreTiros   // o Ponto Extra paga por eles
#define PLAYER_SPEED_START 300.0f
#define BASE_SHOOT_FORWARD_SPEED 800.0f
#define BLINK_INVINCIBLE_FREQUENCY 0.1f

Player::Player(Scene* scene, const size_t quem) :
    // NA ORDEM EM QUE OS MEMBROS SAO DECLARADOS - mQuem e declarado depois de
    // mMoving, e e nessa ordem que o compilador inicializa de qualquer jeito.
    Actor(scene),
    playerSpeed(PLAYER_SPEED_START),
    atkTimer(ATK_TIMER_START_FULL),
    invincibleTimer(0.0f),
    isInvincible(false),
    mBlinkTimer(BLINK_INVINCIBLE_FREQUENCY),
    mMoving(false),
    mQuem(quem)

{

    // A APARENCIA VEM DO PERFIL, e nao de um caminho fixo. A personagem e
    // montada das camadas escolhidas (ou da padrao, para quem nao escolheu) e
    // vira UMA textura, que daqui para baixo e uma sprite animada como outra
    // qualquer - ver Source/ComporPersonagem.h.
    std::string spritePath = Caminhos::Asset("Player/DPIBHPlayer.png");
    std::string dataPath   = Caminhos::Asset("Player/DPIBHPlayer.json");

    Game* game = GetScene()->GetGame();
    const Personagens::Composta composta =
        // A APARENCIA DA SESSAO DESTE ALUNO. Cada jogador entra com a
        // personagem do proprio perfil (requisitos-coop, D6).
        Personagens::Compor(game, Personagens::Carregado(), game->AparenciaDe(mQuem));

    if (composta.ok) {
        spritePath = composta.chaveDaTextura;   // ja esta no cache de texturas
        dataPath   = composta.atlas;
    }
    else {
        // SOBRA O SPRITE ANTIGO. Um catalogo faltando ou uma arte ilegivel nao
        // podem impedir alguem de jogar; o motivo ja saiu no log de quem tentou
        // compor.
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

    // A linha 'keyState = SDL_GetKeyboardState(nullptr);' que existia aqui
    // sobrescrevia o parametro recebido, ignorando o estado que a Scene tinha
    // acabado de passar. Funcionava por coincidencia (era o mesmo ponteiro),
    // mas escondia de quem le a funcao qual e a fonte real do input.

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
    // CADA UM SO ANDA O RELOGIO DELE; QUEM PINTA E UM SO.
    //
    // Antes cada um destes tres chamava SetColor, e a ultima chamada do quadro
    // ganhava. Isso escondeu um bug por um commit inteiro: UpdateOverload
    // pintava de BRANCO no ramo "else", ou seja em todo quadro em que o aluno
    // nao estava sobrecarregado - e como ele rodava depois de UpdateCorrente,
    // apagava a cor da corrente antes de alguem ver. O efeito existia, mexia na
    // velocidade, e era invisivel.
    //
    // Com um pintor so a prioridade vira uma coisa escrita, e nao uma
    // consequencia da ordem das chamadas.
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
    // O TEMPO DEPENDE DO MODO (Dificuldade::Invencibilidade): no Dificil e bem
    // mais curto, e a rajada que vem atras do primeiro tiro tambem acerta.
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

    // O MANCHE DESTE ALUNO. Era W A S D ou as setas, as duas coisas movendo o
    // mesmo jogador - e por isso a posicao 2 do gabinete ficava sem manche.
    // As setas sao do jogador 1, o W A S D e do jogador 2; ver Painel.h.
    const Painel::Jogador kQuem = MeuPainel();

    // A VELOCIDADE E LIDA COM O FATOR, e playerSpeed fica intacto. Ver
    // mCorrenteTimer em Player.h: guardar a velocidade antiga para devolver
    // depois seria um estado a mais para errar.
    const float velocidade = VelocidadeDeAndar();

    // ------ verificação de movimento CIMA/BAIXO ------
    if(Painel::Apertado(keyState, kQuem, Painel::Direcao::Cima))
        newYSpeed -= velocidade;
    else if(Painel::Apertado(keyState, kQuem, Painel::Direcao::Baixo))
        newYSpeed += velocidade;

    // ------ verificação de movimento ESQUERDA/DIREITA ------
    if(Painel::Apertado(keyState, kQuem, Painel::Direcao::Esquerda))
        newXSpeed -= velocidade;
    else if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Direita))
        newXSpeed += velocidade;

    //vendo se está movendo ou não para mudar a variável mMoving (pra mudar a animação de acordo)
    if(newXSpeed == 0 && newYSpeed == 0) mMoving = false;
    else mMoving = true;

    GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(newXSpeed, newYSpeed));
}
void Player::ShootInput(const Uint8 *keyState) {

    // BOTAO 1 do painel DESTE ALUNO. Ver Painel.h: a tecla mora num lugar so.
    if(Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Um) && mOverloadTimer <= 0.0f) {
        Shoot();
    }

}
void Player::SpecialInput(const Uint8 *keyState) {

    // BOTAO 2 do painel DESTE ALUNO.
    if(Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Dois) && mExtraPoints > 0 && mEsperaDoPontoExtra <= 0.0f) {
        mExtraPoints--;
        const auto battle = dynamic_cast<Battle*>(GetScene());
        // SO O SOLO SOBRECARREGA. Em dupla o presente vai para o outro, e quem
        // gastou continua atirando - e espera o escudo do outro acabar.
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

//ShootInput Subfunction
void Player::Shoot() {


    if (atkTimer > 0) return;

    auto battle = dynamic_cast<Battle*>(GetScene());
    if (!battle) return;

    auto projManager = battle->GetProjectileManager();
    if (!projManager) return;

    // TODO - MATANDO o Object Pooling - mudar pra chamada de função em ProjectileManager
    auto projectile = std::make_unique<PlayerProjectile>(GetScene(), this);

    // Posiciona o projétil na frente do jogador
    projectile->SetPosition(GetPosition());

    // Define velocidade inicial (sobe)
    projectile->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0.0f, -BASE_SHOOT_FORWARD_SPEED));

    // Ativa e torna visível
    projectile->ActivateProjectile();

    // Entrega ownership ao manager
    projManager->AddPlayerProjectile(std::move(projectile));
    ++mDisparos;

    // Toca audio
    const auto audioHandler = mScene->GetGame()->GetAudio()->PlaySound("plst00.wav", false);
    mScene->GetGame()->GetAudio()->SetSoundVolume(audioHandler, 10);

    atkTimer = ATK_TIMER_START_FULL;
}

//OnUpdate Subfunction
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

    // A REDE DO VITOR (o socket(), como orientador): com as paredes abertas,
    // quem passa de um lado entra pelo outro, com a sobra do passo.
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

    // SO O RELOGIO E O PULSO. Quem pinta e AtualizarCorDoEstado.
    mEsperaDoPontoExtra = std::max(0.0f, mEsperaDoPontoExtra - deltaTime);

    if (mOverloadTimer <= 0.0f) {
        mPulseTimer = 0.0f;
        return;
    }


    mOverloadTimer -= deltaTime;

    // Quanto MENOR o timer (mais perto de acabar), MAIOR a frequencia: comeca
    // em 5 e sobe 15 por segundo passado.
    const float freq = 5.0f + (OVERLOAD_DURATION - mOverloadTimer) * 15.0f;
    mPulseTimer += freq * deltaTime;
}

void Player::Proteger(const float segundos) {

    // O MAIOR DOS DOIS, e nao a soma. Ver o cabecalho: renova, nao acumula.
    // Usar o maior tambem impede que um presente curto ENCURTE uma protecao
    // mais longa que ja estivesse valendo.
    if (segundos > mImunidadeTimer) mImunidadeTimer = segundos;
}

void Player::Acorrentar(const float segundos, const float fatorDeVelocidade) {

    // O MAIOR DOS DOIS, como em Proteger: renova, nao acumula, e uma corrente
    // curta nao encurta uma longa que ja estava valendo.
    if (segundos > mCorrenteTimer) {
        mCorrenteTimer = segundos;

        // O FATOR ACOMPANHA O TIMER VENCEDOR. Se o que vale e o da corrente
        // nova, o quanto ela pesa tambem e dela - guardar o fator antigo faria
        // uma corrente leve e longa anular uma pesada e curta pela metade.
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

    // FEEDBACK FORTE, E A PRIMEIRA VERSAO ERROU A MAO. Ela escurecia de 120 a
    // 155 com um azul de leve, "porque peso se le apagado, nao agitado". Na
    // foto o aluno preso ficava praticamente igual ao normal - e um efeito que
    // muda a velocidade PRECISA se anunciar, senao o jogador culpa o controle.
    //
    // Agora sao tres coisas ao mesmo tempo, e so as tres juntas se leem de
    // longe no meio de uma tela cheia:
    //
    //   ESCURECE DE VERDADE   o corpo cai para perto de 40% do brilho
    //   PUXA PARA O FERRO     o azul fica bem acima do vermelho e do verde
    //   PULSA RAPIDO NO FIM   a frequencia sobe conforme a corrente acaba, e e
    //                         o aviso de "ja vai soltar" - mesma ideia da
    //                         imunidade, que acelera quando esta para expirar
    const float fracaoRestante = mCorrenteTimer / kDuracaoTipicaDaCorrente;
    mPulsoDaCorrente += (4.0f + (1.0f - fracaoRestante) * 14.0f) * deltaTime;

    // Quem pinta e AtualizarCorDoEstado; aqui so anda o pulso.
}

void Player::UpdateImunidade(const float deltaTime) {

    // SO O RELOGIO E O PULSO. Quem pinta e AtualizarCorDoEstado.
    if (mImunidadeTimer <= 0.0f) return;

    mImunidadeTimer -= deltaTime;
    if (mImunidadeTimer <= 0.0f) {
        mImunidadeTimer = 0.0f;
        mPulsoDaImunidade = 0.0f;
        return;
    }

    // O MESMO PULSO DO OVERLOAD, e as duas se distinguem pela COR e nao pelo
    // ritmo - que e o que da para ler no meio de uma tela cheia de bala.
    const float freq = 5.0f + (IMUNIDADE_DO_PRESENTE - mImunidadeTimer) * 15.0f;
    mPulsoDaImunidade += freq * deltaTime;
}

void Player::AtualizarCorDoEstado() {

    const auto dc = GetComponent<DrawAnimatedComponent>();
    if (!dc) return;

    // A ORDEM DOS IFS E A PRIORIDADE, e esta escrita de proposito - ver o
    // cabecalho. O primeiro que casar pinta e sai.

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
        // FERRO, ESCURO E PULSANDO. Ver UpdateCorrente para o porque dos tres
        // efeitos juntos.
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
