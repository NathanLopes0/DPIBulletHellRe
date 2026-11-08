//
// Created by nslop on 03/10/2024.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include <cstddef>

#include "../Actor.h"
#include "../../Entrada/Painel.h"
#include "../../Regras/Acerto.h"
#include "../../Regras/PontoExtra.h"

class PlayerProjectile;
class PlayerOrb;

class Player : public Actor {

public:
    explicit Player(Scene* scene, size_t quem = 0);

    // Process Input -> Update, sobrescritas de Actor para Player.
    void OnProcessInput(const Uint8* keyState) override;

    void UpdateOverload(float deltaTime);

    void OnUpdate(float deltaTime) override;

    void SetIsInvincible(const bool invincible) { isInvincible = invincible; }

    /// Invencivel por qualquer motivo: os quadros de graca depois de apanhar ou a imunidade dada
    /// pelo parceiro.
    [[nodiscard]] bool GetIsInvincible() const { return isInvincible || mImunidadeTimer > 0.0f; }

    /// O mesmo causador nao acerta duas vezes seguidas (Acerto.h): 'quem' e o projetil ou o corpo
    /// do chefe, e 'idade' a vida dele (0 para o corpo).
    [[nodiscard]] bool JaFoiAcertadoPor(const void* quem, const float idade) const { return mAcertos.Lembra(quem, idade); }
    void FoiAcertadoPor(const void* quem, const float idade) { mAcertos.Lembrar(quem, idade); }

    /// Protege este aluno por um tempo, as custas do Ponto Extra do parceiro. Renova, nao acumula.
    void Proteger(float segundos);

    /// Prende o aluno: ele anda a uma fracao da velocidade por um tempo. E o efeito das correntes
    /// do Vitor - a piada de "concorrentes".
    void Acorrentar(float segundos, float fatorDeVelocidade);

    /// Battle e as regras de projetil leem isto.
    [[nodiscard]] bool EstaAcorrentado() const { return mCorrenteTimer > 0.0f; }

    /// Quao rapido o aluno anda neste instante, ja com a corrente. Fonte unica.
    [[nodiscard]] float VelocidadeDeAndar() const {
        return playerSpeed * (mCorrenteTimer > 0.0f ? mFatorDaCorrente : 1.0f);
    }

    /// Quanto ainda falta da imunidade do parceiro. Zero quer dizer desprotegido.
    [[nodiscard]] float TempoDeImunidade() const { return mImunidadeTimer; }

    /// O tempo que quem gasta um Ponto Extra sozinho passa sem atirar. Um numero so, e publico por
    /// isso.

    static constexpr float OVERLOAD_DURATION = PontoExtra::kSobrecarga;
    static constexpr float IMUNIDADE_DO_PRESENTE = PontoExtra::kImunidadeDoPresente;

    [[nodiscard]] int GetExtraPoints() const { return mExtraPoints;}

    /// Quantos tiros ele ja deu na batalha. O Observer da Maria Lucia, como
    /// orientadora do TCC, atira a cada tantos (Golpe::Notificacoes).
    [[nodiscard]] int Disparos() const { return mDisparos; }
    void AddExtraPoint();

private:

    // Variaveis de jogador.
    float playerSpeed;
    float atkTimer;
    float invincibleTimer;
    bool isInvincible;
    /// Tempo desde a ultima troca de visibilidade durante a invencibilidade.
    float mBlinkTimer;
    bool mMoving;                       //ve se o jogador está se movendo. Usada para mudar a animação
    /// A posicao deste aluno no painel.
    size_t mQuem = 0;

    /// O painel que move este aluno.
    [[nodiscard]] Painel::Jogador MeuPainel() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    int mExtraPoints = 1;
    int mDisparos = 0;
    float mOverloadTimer = 0.0f;
    float mPulseTimer = 0.0f;

    /// Quanto falta para este aluno poder gastar outro Ponto Extra.
    float mEsperaDoPontoExtra = 0.0f;

    /// Quanto resta da imunidade dada pelo parceiro.
    float mImunidadeTimer = 0.0f;

    /// Quanto falta da corrente, e quanto ela corta da velocidade.
    static constexpr float kDuracaoTipicaDaCorrente = 3.5f;

    float mCorrenteTimer = 0.0f;
    float mFatorDaCorrente = 1.0f;
    float mPulsoDaCorrente = 0.0f;

    /// O pulso da imunidade, separado de mPulseTimer.
    float mPulsoDaImunidade = 0.0f;

    // OnProcessInput Sub-functions, and its sub-functions.
    void MoveInput(const Uint8 *state);
    void ShootInput(const Uint8 *state);
        void Shoot();
    void SpecialInput(const Uint8 *state);

    // OnUpdate Sub-functions.
    void InvincibleUpdate(float deltaTime);
    Acerto::Memoria mAcertos;
    void UpdateImunidade(float deltaTime);
    void UpdateCorrente(float deltaTime);

    /// O unico lugar que pinta o aluno.
    void AtualizarCorDoEstado();
    void HandleAnimation();
    void DecreaseAtkTimer(float deltaTime);

    void BorderLimitCheck();

};

#endif // DPIBULLETHELLRE_PLAYER_H.
