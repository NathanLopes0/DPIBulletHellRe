//
// O aluno.
//

#include <algorithm>

#include "Player.h"

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

namespace {

    /// O pulso da sobrecarga e o da imunidade: acelera ate o fim. As duas se distinguem pela cor, e
    /// nao pelo ritmo.
    float FrequenciaDoPulso(const float decorrido) {
        return 5.0f + decorrido * 15.0f;
    }

    /// O seno de um pulso, de 0 a 255.
    Uint8 Onda(const float pulso) {
        return static_cast<Uint8>((Math::Sin(pulso) + 1.0f) * 127.5f);
    }

}

Player::Player(Scene* scene, const size_t quem) : Actor(scene), mQuem(quem) {

    // A aparencia vem do perfil, e nao de um caminho fixo; o sprite antigo e o plano B.
    std::string folha = Caminhos::Asset("Player/DPIBHPlayer.png");
    std::string atlas = Caminhos::Asset("Player/DPIBHPlayer.json");
    const Personagens::Composta composta = MinhaPersonagem();
    if (composta.ok) {
        folha = composta.chaveDaTextura;   // ja esta no cache de texturas
        atlas = composta.atlas;
    } else {
        // Um catalogo faltando ou uma arte ilegivel nao podem impedir alguem de jogar.
        SDL_Log("PLAYER: a personagem nao pode ser composta; usando o sprite antigo.");
    }

    mDesenho = AddComponent<DrawAnimatedComponent>(folha, atlas);
    mDesenho->AddAnimation("Moving", {0, 1, 2, 3});
    mDesenho->AddAnimation("Idle", {1});
    mDesenho->SetAnimation("Idle");

    mCorpo = AddComponent<RigidBodyComponent>();
    auto* colisor = AddComponent<CircleColliderComponent>(static_cast<float>(mDesenho->GetSpriteWidth()) / 10.f);
    colisor->SetTag(ColliderTag::Player);
}

Personagens::Composta Player::MinhaPersonagem() {
    Game* game = GetScene()->GetGame();
    return Personagens::Compor(game, Personagens::Carregado(), game->AparenciaDe(mQuem), game->Formado(mQuem));
}

void Player::RecomporPersonagem() {
    const Personagens::Composta composta = MinhaPersonagem();
    if (!composta.ok) return;   // fica a de antes
    mDesenho->LoadSpriteSheet(composta.chaveDaTextura, composta.atlas);
    // Mesmo que a luta tenha acabado no meio de uma piscada.
    mDesenho->SetIsVisible(true);
}

void Player::DefinirInvencivel(const bool invencivel) {
    // Comecar a invencibilidade recomeca o pisca: a primeira troca e ja no quadro seguinte.
    if (invencivel && !mInvencivel) mTempoDePiscar = kPiscar;
    mInvencivel = invencivel;
}

void Player::OnProcessInput(const Uint8* keyState) {
    LerManche(keyState);
    LerBotaoDeTiro(keyState);
    LerBotaoDoPontoExtra(keyState);
    LerBotaoDoSinal(keyState);
}

void Player::OnUpdate(const float deltaTime) {
    AtualizarInvencibilidade(deltaTime);
    mAcertos.Avancar(deltaTime);
    mRaspoes.Avancar(deltaTime);
    AtualizarAnimacao();
    DescontarRecarga(deltaTime);
    PrenderNoCampo();
    // Cada um so anda o relogio dele; quem pinta e um so.
    AtualizarCorrente(deltaTime);
    AtualizarSobrecarga(deltaTime);
    AtualizarImunidade(deltaTime);
    AtualizarCorDoEstado();
}

// ---- A entrada ----

void Player::LerManche(const Uint8* keyState) {

    if (mPiloto.Dirigir(*this, *mCorpo, mAndando)) return;

    // A velocidade e lida com o fator da corrente; kVelocidade fica intacta.
    const float velocidade = VelocidadeDeAndar();
    const Painel::Jogador eu = MeuPainel();

    float vx = 0.0f;
    float vy = 0.0f;
    if (Painel::Apertado(keyState, eu, Painel::Direcao::Cima)) vy -= velocidade;
    else if (Painel::Apertado(keyState, eu, Painel::Direcao::Baixo)) vy += velocidade;
    if (Painel::Apertado(keyState, eu, Painel::Direcao::Esquerda)) vx -= velocidade;
    else if (Painel::Apertado(keyState, eu, Painel::Direcao::Direita)) vx += velocidade;

    mAndando = vx != 0.0f || vy != 0.0f;
    mCorpo->SetVelocity(Vector2(vx, vy));
}

void Player::LerBotaoDeTiro(const Uint8* keyState) {
    // A sobrecarga trava o tiro. O piloto da simulacao atira sempre que pode (--atira).
    if (mTempoDeSobrecarga > 0.0f) return;
    const bool atira = GetScene()->GetGame()->ConfiguracaoDaSimulacao().atira ||
                       Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Um);
    if (atira) Atirar();
}

void Player::Atirar() {

    if (mRecargaDoTiro > 0) return;

    auto* battle = dynamic_cast<Battle*>(GetScene());
    if (!battle) return;
    auto* projeteis = battle->GerenteDeProjeteis();
    if (!projeteis) return;

    // TODO: o tiro do aluno ainda nao usa o pool de projeteis (ProjectilePool.h), como os do chefe.
    auto tiro = std::make_unique<PlayerProjectile>(GetScene(), this, mQuem);
    tiro->SetPosition(GetPosition());
    tiro->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0.0f, -PlayerProjectile::kVelocidade));
    tiro->ActivateProjectile();
    projeteis->AddPlayerProjectile(std::move(tiro));
    ++mDisparos;

    auto* audio = mScene->GetGame()->GetAudio();
    audio->SetSoundVolume(audio->PlaySound("plst00.wav", false), kVolumeDoTiro);

    mRecargaDoTiro = kEntreTiros;
}

void Player::LerBotaoDoPontoExtra(const Uint8* keyState) {

    if (!Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Dois)) return;
    if (mPontosExtras <= 0 || mEsperaDoPontoExtra > 0.0f) return;

    mPontosExtras--;
    auto* battle = dynamic_cast<Battle*>(GetScene());
    // So o solo sobrecarrega; na dupla o Ponto Extra protege o parceiro.
    const bool sozinho = !battle || !battle->OutroJogador(this);
    mEsperaDoPontoExtra = sozinho ? PontoExtra::kSobrecarga : PontoExtra::kImunidadeDoPresente;
    if (sozinho) mTempoDeSobrecarga = PontoExtra::kSobrecarga;
    if (battle) battle->AoGastarPontoExtra(this);
}

void Player::LerBotaoDoSinal(const Uint8* keyState) {
    const bool apertado = Painel::Apertado(keyState, MeuPainel(), Painel::Botao::Tres);
    if (apertado && !mSinalAntes) {
        if (auto* battle = dynamic_cast<Battle*>(GetScene())) battle->Sinalizar(*this);
    }
    mSinalAntes = apertado;
}

// ---- O quadro ----

void Player::AtualizarInvencibilidade(const float deltaTime) {
    if (!mInvencivel) return;

    // O tempo depende do modo (Dificuldade::Invencibilidade).
    mTempoInvencivel += deltaTime;
    const Dificuldade::Nivel nivel = (mScene && mScene->GetGame()) ? mScene->GetGame()->NivelDeDificuldade()
                                                                   : Dificuldade::Nivel::Normal;
    if (mTempoInvencivel >= Dificuldade::Invencibilidade(nivel)) {
        mInvencivel = false;
        mTempoInvencivel = 0.0f;
        mTempoDePiscar = kPiscar;
        mDesenho->SetIsVisible(true);
        return;
    }

    mTempoDePiscar += deltaTime;
    if (mTempoDePiscar >= kPiscar) {
        mTempoDePiscar = 0.0f;
        mDesenho->SetIsVisible(!mDesenho->GetIsVisible());
    }
}

void Player::AtualizarAnimacao() {
    mDesenho->SetAnimation(mAndando ? "Moving" : "Idle");
}

void Player::DescontarRecarga(const float deltaTime) {
    if (mRecargaDoTiro > 0) mRecargaDoTiro -= deltaTime;
}

void Player::PrenderNoCampo() {
    const SDL_FRect campo = mScene->GetPlayfieldBounds();
    const auto largura = static_cast<float>(mDesenho->GetSpriteWidth());
    const auto altura = static_cast<float>(mDesenho->GetSpriteHeight());
    SetPosition(Vector2(Math::Clamp(GetPosition().x, campo.x + largura, campo.x + campo.w - largura),
                        Math::Clamp(GetPosition().y, campo.y + altura, campo.y + campo.h - altura)));
}

void Player::AtualizarSobrecarga(const float deltaTime) {

    mEsperaDoPontoExtra = std::max(0.0f, mEsperaDoPontoExtra - deltaTime);

    if (mTempoDeSobrecarga <= 0.0f) {
        mPulsoDaSobrecarga = 0.0f;
        return;
    }
    mTempoDeSobrecarga -= deltaTime;
    mPulsoDaSobrecarga += FrequenciaDoPulso(PontoExtra::kSobrecarga - mTempoDeSobrecarga) * deltaTime;
}

void Player::AtualizarImunidade(const float deltaTime) {

    if (mTempoDeImunidade <= 0.0f) return;

    mTempoDeImunidade -= deltaTime;
    if (mTempoDeImunidade <= 0.0f) {
        mTempoDeImunidade = 0.0f;
        mPulsoDaImunidade = 0.0f;
        return;
    }
    mPulsoDaImunidade += FrequenciaDoPulso(PontoExtra::kImunidadeDoPresente - mTempoDeImunidade) * deltaTime;
}

void Player::AtualizarCorrente(const float deltaTime) {

    if (mTempoDeCorrente <= 0.0f) return;

    mTempoDeCorrente -= deltaTime;
    if (mTempoDeCorrente <= 0.0f) {
        mTempoDeCorrente = 0.0f;
        mFatorDaCorrente = 1.0f;
        mPulsoDaCorrente = 0.0f;
        return;
    }
    // Peso se le apagado, e nao agitado: o pulso so acelera perto do fim.
    const float fracaoRestante = mTempoDeCorrente / kDuracaoTipicaDaCorrente;
    mPulsoDaCorrente += (4.0f + (1.0f - fracaoRestante) * 14.0f) * deltaTime;
}

void Player::AtualizarCorDoEstado() {

    // A ordem e a prioridade: a imunidade, a sobrecarga, a corrente.
    if (mTempoDeImunidade > 0.0f) {
        const Uint8 rg = Onda(mPulsoDaImunidade);
        mDesenho->SetColor(rg, rg, 255);
        return;
    }
    if (mTempoDeSobrecarga > 0.0f) {
        const Uint8 gb = Onda(mPulsoDaSobrecarga);
        mDesenho->SetColor(255, gb, gb);
        return;
    }
    if (mTempoDeCorrente > 0.0f) {
        // Ferro, escuro e pulsando.
        const float osc = (Math::Sin(mPulsoDaCorrente) + 1.0f) * 0.5f;
        const auto escuro = static_cast<Uint8>(70.0f + osc * 45.0f);
        const auto azul = static_cast<Uint8>(150.0f + osc * 60.0f);
        mDesenho->SetColor(escuro, escuro, azul);
        return;
    }
    mDesenho->SetColor(255, 255, 255);
}

// ---- Os efeitos de fora ----

void Player::Proteger(const float segundos) {
    // O maior dos dois, e nao a soma.
    mTempoDeImunidade = std::max(mTempoDeImunidade, segundos);
}

void Player::Acorrentar(const float segundos, const float fatorDeVelocidade) {
    // O maior dos dois, como em Proteger; o fator acompanha o tempo que venceu.
    if (segundos > mTempoDeCorrente) {
        mTempoDeCorrente = segundos;
        mFatorDaCorrente = fatorDeVelocidade;
    }
}

void Player::GanharPontoExtra() {
    mPontosExtras = Math::Clamp(mPontosExtras + 1, 0, kMaximoDePontosExtras);
}
