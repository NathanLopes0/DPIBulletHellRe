//
// Created by nslop on 21/11/2024.
//

#pragma once

#include <cstddef>

#include "../../../Projectile.h"

class Boss;

/**
 * @class BossProjectile
 * @brief Uma especialização de Projétil que é disparada por um Boss.
 * Responsável por carregar seus componentes específicos e atualizar seus Behaviors.
 */
class BossProjectile : public Projectile {
public:
    explicit BossProjectile(Scene* scene, Boss* owner);
    ~BossProjectile() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] Boss* GetBossOwner() const;

    /**
     * @brief De quem este projetil e. Posto UMA vez, ao ser disparado.
     *
     * ESTE E O MOTIVO DE O REVEZAMENTO SER POSSIVEL. Os tres GetPlayer* abaixo
     * sao lidos A CADA QUADRO enquanto o projetil voa - e assim que a
     * perseguicao e a mira preditiva funcionam. Se eles perguntassem "de quem e
     * a vez agora", um projetil perseguidor trocaria de alvo no meio do voo a
     * cada disparo novo do chefe: dezenas deles mudando de rumo ao mesmo tempo.
     *
     * Guarda a POSICAO, e nao o ponteiro. Os projeteis vem de um pool que vive
     * no Game e sobrevive a troca de batalha; um ponteiro guardado apontaria
     * para um jogador ja destruido na batalha seguinte.
     *
     * Quem poe e Boss::ExecuteAttack, na leva inteira de uma vez - uma rajada e
     * UM disparo, e todos os projeteis dela miram o mesmo aluno.
     */
    void DefinirAlvo(const size_t posicao) { mAlvo = posicao; }

    Vector2 GetPlayerPosition() const;

    /// Velocidade atual do jogador; (0,0) se nao houver jogador ou cena Battle.
    /// Usada pela mira preditiva dos caminhos.
    Vector2 GetPlayerVelocity() const;

    /// true quando ha cena Battle com jogador vivo. Permite ao chamador
    /// distinguir "jogador na origem" de "nao ha jogador".
    bool HasPlayer() const;

    Vector2 GetPlayerDirection() const;

    /**
     * @brief Define a folga que o projetil tem para alem da borda antes de morrer.
     *
     * A margem e: tamanho do sprite * emSprites + tamanho da tela / divisorDeTela.
     * Passar 0 em divisorDeTela remove a parcela proporcional a tela.
     *
     * Os dois numeros existem porque os projeteis do jogo usavam DUAS regras
     * diferentes. A geral era 1 sprite + um doze avos da tela; os baloes do Andre
     * tinham uma subclasse propria so para usar 2 sprites e nenhuma parcela de
     * tela, o que os faz morrer MAIS CEDO (para um sprite de 32, o balao morre em
     * -64 e os outros em -132).
     *
     * Parametrizar em vez de manter a subclasse: a diferenca e de numero, nao de
     * comportamento, e era o ultimo motivo para existir uma classe de projetil por
     * professor.
     */
    void DefinirMargemDeSaida(float emSprites, float divisorDeTela);

    /**
     * @brief Faz o sprite girar para acompanhar a direcao do movimento.
     *
     * Para as listas do Salles, cuja arte tem uma seta: sem girar, a seta de
     * todo no aponta para a direita e o encadeamento nao le como encadeamento.
     *
     * A rotacao acompanha a VELOCIDADE a cada quadro, e nao so o angulo de
     * lancamento, porque os caminhos curvam e a lista duplamente encadeada
     * chega a voltar - a seta tem de virar junto.
     */
    void DefinirRotacaoPelaVelocidade(const bool girar) { mRotacionarComAVelocidade = girar; }

    // ---- O QUE ESTE PROJETIL FAZ AO ACERTAR ---------------------------------
    //
    // So dois projeteis do jogo tem efeito: as CORRENTES do Vitor. Todos os
    // outros deixam estes campos no padrao, que e "nao prende ninguem e custa
    // a nota de sempre". Quem le e aplica e Battle, no laco de colisao.
    void DefinirAcorrentamento(const float segundos, const float fator, const float multiplicador) {
        mSegundosDeCorrente = segundos;
        mFatorDaCorrente = fator;
        mMultiplicadorDaPerda = multiplicador;
    }

    [[nodiscard]] bool Acorrenta() const { return mSegundosDeCorrente > 0.0f; }
    [[nodiscard]] float SegundosDeCorrente() const { return mSegundosDeCorrente; }
    [[nodiscard]] float FatorDaCorrente() const { return mFatorDaCorrente; }
    [[nodiscard]] float MultiplicadorDaPerda() const { return mMultiplicadorDaPerda; }

    /**
     * @brief Captura tambem a rotacao de fabrica, alem da escala e da animacao.
     *
     * PRECISOU EXISTIR porque girar virou coisa POR REGRA, e nao so por tipo de
     * projetil. O "Estruturas" do Salles gira - a lista da fase 1 e uma caixa
     * com seta, e sem girar a seta aponta sempre para a direita -, mas a MESMA
     * folha serve a arvore da fase 3, cujo no e simetrico e nao deve girar.
     * Sem capturar o padrao, o primeiro projetil que uma regra desligasse
     * voltaria do pool desligado e contaminaria a fase 1.
     */
    void MarcarPadraoDeFabrica() override;

    void Reset() override;

protected:
    // Implementação obrigatória do contrato da classe base.
    [[nodiscard]] bool virtual IsOffScreen() const override;

    /// Ver DefinirMargemDeSaida. Os padroes sao a regra geral, entao um projetil
    /// que nunca chama o metodo se comporta como antes desta mudanca.
    float mMargemEmSprites = 1.0f;
    float mMargemDivisorDeTela = 12.0f;

    /// Ver DefinirRotacaoPelaVelocidade. Falso por padrao: girar so faz sentido
    /// para arte que tem frente.
    bool mRotacionarComAVelocidade = false;

    /// Ver DefinirAcorrentamento. Zero segundos = este projetil nao prende.
    /// NAO precisam voltar ao padrao no Reset como a rotacao e a escala: sao
    /// propriedade do TIPO, e nenhuma regra os altera por rajada.
    float mSegundosDeCorrente = 0.0f;
    float mFatorDaCorrente = 1.0f;
    float mMultiplicadorDaPerda = 1.0f;

    /// O valor com que a fabrica construiu este projetil. Ver
    /// MarcarPadraoDeFabrica.
    bool mRotacaoPadrao = false;

    /// A posicao do jogador que este projetil persegue. Ver DefinirAlvo.
    size_t mAlvo = 0;

};
