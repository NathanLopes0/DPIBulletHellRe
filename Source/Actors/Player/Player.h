//
// Created by nslop on 03/10/2024.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include <cstddef>

#include "../Actor.h"
#include "../../Painel.h"

class PlayerProjectile;
class PlayerOrb;

class Player : public Actor {

public:
    /**
     * @param quem QUAL POSICAO do painel move este aluno, e de qual sessao sai
     *        a aparencia dele. As duas coisas sao a mesma posicao, e por isso
     *        entram num parametro so - com duas, haveria como criar um aluno
     *        que anda com o manche de um e tem a cara do outro.
     *
     *        O padrao e a posicao 1, que e o jogo de um jogador.
     */
    explicit Player(Scene* scene, size_t quem = 0);

    //Process Input -> Update, sobrescritas de Actor para Player
    void OnProcessInput(const Uint8* keyState) override;

    void UpdateOverload(float deltaTime);

    void OnUpdate(float deltaTime) override;

    void SetIsInvincible(const bool invincible) { isInvincible = invincible; }
    [[nodiscard]] bool GetIsInvincible() const { return isInvincible; }

    [[nodiscard]] int GetExtraPoints() const { return mExtraPoints;}
    void AddExtraPoint();


private:

    //variaveis de jogador
    float playerSpeed;
    float atkTimer;
    float invincibleTimer;
    bool isInvincible;
    /// Tempo desde a ultima troca de visibilidade durante a invencibilidade.
    /// Era um 'static float' dentro de InvincibleUpdate: estado de INSTANCIA
    /// guardado em variavel de funcao, compartilhado por todas as instancias e
    /// persistente entre cenas. Funcionava porque os dois caminhos de saida o
    /// reinicializam, mas so por sorte.
    float mBlinkTimer;
    bool mMoving;                       //ve se o jogador está se movendo. Usada para mudar a animação
    /// A posicao deste aluno no painel. Ver o construtor.
    size_t mQuem = 0;

    /// O painel que move este aluno. Uma funcao, e nao a conversao repetida em
    /// cada leitura: foi a conversao escrita duas vezes que fez a tela de
    /// matricula ler o botao do jogador 1 e anunciar o do jogador 2.
    [[nodiscard]] Painel::Jogador MeuPainel() const {
        return mQuem == 1 ? Painel::Jogador::Dois : Painel::Jogador::Um;
    }

    int mExtraPoints = 1;
    float mOverloadTimer = 0.0f;
    const float OVERLOAD_DURATION = 3.f;
    float mPulseTimer = 0.0f;



    //OnProcessInput Sub-functions, and its sub-functions
    void MoveInput(const Uint8 *state);
    void ShootInput(const Uint8 *state);
        void Shoot();
    void SpecialInput(const Uint8 *state);


    //OnUpdate Sub-functions
    void InvincibleUpdate(float deltaTime);
    void HandleAnimation();
    void DecreaseAtkTimer(float deltaTime);

    void BorderLimitCheck();

};


#endif //DPIBULLETHELLRE_PLAYER_H
