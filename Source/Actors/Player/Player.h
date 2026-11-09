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
#include "../../Regras/Raspao.h"

class PlayerProjectile;
class PlayerOrb;

class Player : public Actor {

public:
    explicit Player(Scene* scene, size_t quem = 0);

    // Process Input -> Update, sobrescritas de Actor para Player.
    void OnProcessInput(const Uint8* keyState) override;

    void AtualizarSobrecarga(float deltaTime);

    void OnUpdate(float deltaTime) override;

    void DefinirInvencivel(const bool invincible) { mInvencivel = invincible; }

    /// Invencivel por qualquer motivo: os quadros de graca depois de apanhar ou a imunidade dada
    /// pelo parceiro.
    [[nodiscard]] bool EstaInvencivel() const { return mInvencivel || mTempoDeImunidade > 0.0f; }

    /// O mesmo causador nao acerta duas vezes seguidas (Acerto.h): 'quem' e o projetil ou o corpo
    /// do chefe, e 'idade' a vida dele (0 para o corpo).
    [[nodiscard]] bool JaFoiAcertadoPor(const void* quem, const float idade) const { return mAcertos.Lembra(quem, idade); }
    void FoiAcertadoPor(const void* quem, const float idade) { mAcertos.Lembrar(quem, idade); }

    /// Um projetil raspa o aluno uma vez so (Raspao.h): a mesma memoria dos acertos, com o prazo da
    /// vida de um projetil.
    [[nodiscard]] bool JaRaspou(const void* quem, const float idade) const { return mRaspoes.Lembra(quem, idade); }
    void Raspou(const void* quem, const float idade) { mRaspoes.Lembrar(quem, idade); }

    /// Protege este aluno por um tempo, as custas do Ponto Extra do parceiro. Renova, nao acumula.
    void Proteger(float segundos);

    /// Prende o aluno: ele anda a uma fracao da velocidade por um tempo. E o efeito das correntes
    /// do Vitor - a piada de "concorrentes".
    void Acorrentar(float segundos, float fatorDeVelocidade);

    /// Battle e as regras de projetil leem isto.
    [[nodiscard]] bool EstaAcorrentado() const { return mTempoDeCorrente > 0.0f; }

    /// Quao rapido o aluno anda neste instante, ja com a corrente. Fonte unica.
    [[nodiscard]] float VelocidadeDeAndar() const {
        return mVelocidade * (mTempoDeCorrente > 0.0f ? mFatorDaCorrente : 1.0f);
    }

    /// Quanto ainda falta da imunidade do parceiro. Zero quer dizer desprotegido.
    [[nodiscard]] float TempoDeImunidade() const { return mTempoDeImunidade; }

    [[nodiscard]] int PontosExtras() const { return mPontosExtras;}

    /// Quantos tiros ele ja deu na batalha. O Observer da Maria Lucia, como
    /// orientadora do TCC, atira a cada tantos (Golpe::Notificacoes).
    [[nodiscard]] int Disparos() const { return mDisparos; }
    void GanharPontoExtra();

private:

    // Variaveis de jogador.
    float mVelocidade;
    float mRecargaDoTiro;
    float mTempoInvencivel;
    bool mInvencivel;
    /// Tempo desde a ultima troca de visibilidade durante a invencibilidade.
    float mTempoDePiscar;
    bool mAndando;                       //ve se o jogador está se movendo. Usada para mudar a animação
    /// A posicao deste aluno no painel.
    size_t mQuem = 0;

    /// O painel que move este aluno.
    [[nodiscard]] Painel::Jogador MeuPainel() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    int mPontosExtras = 1;
    int mDisparos = 0;
    float mTempoDeSobrecarga = 0.0f;
    float mTempoDoPulso = 0.0f;

    /// Quanto falta para este aluno poder gastar outro Ponto Extra.
    float mEsperaDoPontoExtra = 0.0f;

    /// Quanto resta da imunidade dada pelo parceiro.
    float mTempoDeImunidade = 0.0f;

    /// Quanto falta da corrente, e quanto ela corta da velocidade.
    static constexpr float kDuracaoTipicaDaCorrente = 3.5f;

    float mTempoDeCorrente = 0.0f;
    float mFatorDaCorrente = 1.0f;
    float mPulsoDaCorrente = 0.0f;

    /// O pulso da imunidade, separado de mTempoDoPulso.
    float mPulsoDaImunidade = 0.0f;

    // OnProcessInput Sub-functions, and its sub-functions.
    void LerManche(const Uint8 *state);
    void LerBotaoDeTiro(const Uint8 *state);
        void Atirar();
    void LerBotaoDoPontoExtra(const Uint8 *state);

    // OnUpdate Sub-functions.
    void AtualizarInvencibilidade(float deltaTime);
    Acerto::Memoria mAcertos;
    Acerto::Memoria mRaspoes{Raspao::kMemoria};
    void UpdateImunidade(float deltaTime);
    void UpdateCorrente(float deltaTime);

    /// O unico lugar que pinta o aluno.
    void AtualizarCorDoEstado();
    void AtualizarAnimacao();
    void DescontarRecarga(float deltaTime);

    void PrenderNoCampo();

};

#endif // DPIBULLETHELLRE_PLAYER_H.
