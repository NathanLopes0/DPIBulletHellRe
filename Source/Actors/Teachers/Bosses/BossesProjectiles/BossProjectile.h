//
// Created by nslop on 21/11/2024.
//

#pragma once

#include <cstddef>
#include <string>
#include <utility>

#include "../../../Projectile.h"

class Boss;

/// Uma especialização de Projétil que é disparada por um Boss. Responsável por carregar seus
/// componentes específicos e atualizar seus Behaviors.
class BossProjectile : public Projectile {
public:
    explicit BossProjectile(Scene* scene, Boss* owner);
    ~BossProjectile() override = default;

    void OnUpdate(float deltaTime) override;

    /// De quem este projetil e. Posto uma vez, ao ser disparado.
    void DefinirAlvo(const size_t posicao) { mAlvo = posicao; }

    /// O tipo dele: o nome em projeteis.json ("Bloco", "Chave"). Da fabrica, e nao do disparo: nao
    /// volta no Reset. A devolutiva conta por ele o que acertou o aluno.
    void DefinirTipo(std::string tipo) { mTipo = std::move(tipo); }
    [[nodiscard]] const std::string& Tipo() const { return mTipo; }

    Vector2 GetPlayerPosition() const;

    /// Velocidade atual do jogador; (0,0) se nao houver jogador ou cena Battle.
    /// Usada pela mira preditiva dos caminhos.
    Vector2 GetPlayerVelocity() const;

    /// True quando ha cena Battle com jogador vivo. Permite ao chamador distinguir "jogador na
    /// origem" de "nao ha jogador".
    bool HasPlayer() const;

    Vector2 GetPlayerDirection() const;

    /// Define a folga que o projetil tem para alem da borda antes de morrer. A margem e: tamanho do
    /// sprite * emSprites + tamanho da tela / divisorDeTela.
    void DefinirMargemDeSaida(float emSprites, float divisorDeTela);

    /// Faz o sprite girar para acompanhar a direcao do movimento.
    void DefinirRotacaoPelaVelocidade(const bool girar) { mRotacionarComAVelocidade = girar; }

    // ---- O que este projetil faz ao acertar ----
    void DefinirAcorrentamento(const float segundos, const float fator, const float multiplicador) {
        mSegundosDeCorrente = segundos;
        mFatorDaCorrente = fator;
        mMultiplicadorDaPerda = multiplicador;
    }

    [[nodiscard]] bool Acorrenta() const { return mSegundosDeCorrente > 0.0f; }

    /// O tiro do aluno pode derrubar este projetil?
    void DefinirDerrubavel(const bool derrubavel) { mDerrubavel = derrubavel; }
    [[nodiscard]] bool Derrubavel() const { return mDerrubavel; }

    /// Este projetil e um abrigo?
    void DefinirAbrigo(const bool abrigo) { mAbrigo = abrigo; }
    [[nodiscard]] bool Abrigo() const { return mAbrigo; }

    /// Quantos tiros o derrubam (DescricaoDeProjetil::resistencia). Do tipo, como o derrubavel; os
    /// golpes ja levados sao do disparo.
    void DefinirResistencia(const int resistencia) { mResistencia = resistencia < 1 ? 1 : resistencia; }

    /// Um tiro o acertou. Devolve true no golpe que o derruba - o ultimo da resistencia; nos
    /// anteriores o tiro so some nele.
    bool Golpear() { return ++mGolpes >= mResistencia; }
    /// O tiro do aluno some neste projetil, sem derruba-lo e sem chegar ao chefe?
    void DefinirBlindado(const bool blindado) { mBlindado = blindado; }
    [[nodiscard]] bool Blindado() const { return mBlindado; }

    /// Este projetil e so um enfeite: nao acerta o aluno, e o tiro do aluno passa por ele, mesmo
    /// que o tipo seja derrubavel.
    void DefinirEnfeite(const bool enfeite) { mEnfeite = enfeite; }
    [[nodiscard]] bool Enfeite() const { return mEnfeite; }

    /// Na dupla, se e a primeira vez que este disparo vai no aluno 'aluno' (Battle::MarcarOCaminhoDosAlunos):
    /// cada projetil conta uma chance de cobertura por aluno, por mais quadros que passe indo nele.
    bool PrimeiraVezNoCaminhoDe(size_t aluno);

    /// O posto em que este projetil esta: um lugar de onde o chefe atira (PontosAttack,
    /// Boss::PontosDoPosto). Vazio = nenhum.
    void DefinirPosto(std::string posto) { mPosto = std::move(posto); }
    [[nodiscard]] const std::string& Posto() const { return mPosto; }
    [[nodiscard]] float SegundosDeCorrente() const { return mSegundosDeCorrente; }
    [[nodiscard]] float FatorDaCorrente() const { return mFatorDaCorrente; }
    [[nodiscard]] float MultiplicadorDaPerda() const { return mMultiplicadorDaPerda; }

    /// Captura tambem a rotacao de fabrica, alem da escala e da animacao.
    void MarcarPadraoDeFabrica() override;

    void Reset() override;

protected:
    // Implementação obrigatória do contrato da classe base.
    [[nodiscard]] bool virtual IsOffScreen() const override;

    float mMargemEmSprites = 1.0f;
    float mMargemDivisorDeTela = 12.0f;

    /// Falso por padrao: girar so faz sentido para arte que tem frente.
    bool mRotacionarComAVelocidade = false;

    /// Zero segundos = este projetil nao prende.
    float mSegundosDeCorrente = 0.0f;
    float mFatorDaCorrente = 1.0f;
    float mMultiplicadorDaPerda = 1.0f;

    /// Do tipo, como a corrente: nao volta no Reset.
    bool mDerrubavel = false;

    bool mAbrigo = false;

    /// A resistencia e do tipo; os golpes, do disparo - voltam a zero no Reset.
    int mResistencia = 1;
    int mGolpes = 0;

    /// Do disparo: volta a falso no Reset.
    bool mBlindado = false;

    /// Do disparo: volta a falso no Reset.
    bool mEnfeite = false;
    /// Em quais alunos este disparo ja foi (um bit por posicao): PrimeiraVezNoCaminhoDe.
    unsigned mCaminhos = 0;

    /// Do disparo: volta a vazio no Reset.
    std::string mPosto;

    /// Da fabrica: fica.
    std::string mTipo;

    /// O valor com que a fabrica construiu este projetil.
    bool mRotacaoPadrao = false;

    /// A posicao do jogador que este projetil persegue.
    size_t mAlvo = 0;

};
