//
// Created by nslop on 21/11/2024.
//

#pragma once

#include <cstddef>
#include <string>
#include <utility>

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

    /// @brief O tiro do aluno pode derrubar este projetil? Ver
    /// DescricaoDeProjetil::derrubavel. Propriedade do TIPO, como a corrente.
    void DefinirDerrubavel(const bool derrubavel) { mDerrubavel = derrubavel; }
    [[nodiscard]] bool Derrubavel() const { return mDerrubavel; }

    /// @brief Este projetil e um abrigo? Ver DescricaoDeProjetil::abrigo. Do
    /// TIPO, como o derrubavel.
    void DefinirAbrigo(const bool abrigo) { mAbrigo = abrigo; }
    [[nodiscard]] bool Abrigo() const { return mAbrigo; }

    /// @brief Quantos tiros o derrubam (DescricaoDeProjetil::resistencia). Do
    /// TIPO, como o derrubavel; os golpes ja levados sao do disparo.
    void DefinirResistencia(const int resistencia) { mResistencia = resistencia < 1 ? 1 : resistencia; }

    /// @brief Um tiro o acertou. Devolve true no golpe que o derruba - o
    /// ultimo da resistencia; nos anteriores o tiro so some nele.
    bool Golpear() { return ++mGolpes >= mResistencia; }
    /// @brief O tiro do aluno SOME neste projetil, sem derruba-lo e sem chegar
    /// ao chefe? E a capsula fechada do Luizinho (CapsulaAttack): o de fora nao
    /// alcanca o de dentro.
    ///
    /// Ao contrario do derrubavel, e do DISPARO, e nao do tipo: a mesma letra
    /// deixa de ser blindada quando a capsula abre. Por isso volta a falso no
    /// Reset - uma capsula que morreu fechada nao pode emprestar a blindagem a
    /// letra que sair do pool no ataque seguinte.
    void DefinirBlindado(const bool blindado) { mBlindado = blindado; }
    [[nodiscard]] bool Blindado() const { return mBlindado; }

    /// @brief Este projetil e so um ENFEITE: nao acerta o aluno, e o tiro do
    /// aluno passa por ele, mesmo que o tipo seja derrubavel. E o cursor da
    /// sprint da Maria Lucia (SprintAttack): sai da mesma fabrica que as
    /// historias, para usar a mesma folha, mas e a mao da professora, e nao um
    /// projetil. Do disparo, como a blindagem: volta a falso no Reset.
    void DefinirEnfeite(const bool enfeite) { mEnfeite = enfeite; }
    [[nodiscard]] bool Enfeite() const { return mEnfeite; }

    /// @brief O POSTO em que este projetil esta: um lugar de onde o chefe atira
    /// (PontosAttack, Boss::PontosDoPosto). Vazio = nenhum. E a historia parada
    /// no quadro da sprint, que solta o codigo dela; quem a poe e a tira do
    /// posto e o behavior dela. Do disparo: volta a vazio no Reset.
    void DefinirPosto(std::string posto) { mPosto = std::move(posto); }
    [[nodiscard]] const std::string& Posto() const { return mPosto; }
    [[nodiscard]] float SegundosDeCorrente() const { return mSegundosDeCorrente; }
    [[nodiscard]] float FatorDaCorrente() const { return mFatorDaCorrente; }
    [[nodiscard]] float MultiplicadorDaPerda() const { return mMultiplicadorDaPerda; }

    /**
     * @brief Captura tambem a rotacao de fabrica, alem da escala e da animacao.
     *
     * PRECISOU EXISTIR porque girar virou coisa POR REGRA, e nao so por tipo de
     * projetil (Regra::rotacionar). Sem capturar o padrao, o primeiro
     * projetil que uma regra desligasse voltaria do pool desligado e
     * contaminaria as rajadas do mesmo tipo que querem girar.
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

    /// Ver DefinirDerrubavel. Do tipo, como a corrente: nao volta no Reset.
    bool mDerrubavel = false;

    /// Ver Abrigo.
    bool mAbrigo = false;

    /// Ver DefinirResistencia e Golpear. A resistencia e do tipo; os golpes,
    /// do disparo - voltam a zero no Reset.
    int mResistencia = 1;
    int mGolpes = 0;

    /// Ver DefinirBlindado. Do disparo: volta a falso no Reset.
    bool mBlindado = false;

    /// Ver DefinirEnfeite. Do disparo: volta a falso no Reset.
    bool mEnfeite = false;

    /// Ver DefinirPosto. Do disparo: volta a vazio no Reset.
    std::string mPosto;

    /// O valor com que a fabrica construiu este projetil. Ver
    /// MarcarPadraoDeFabrica.
    bool mRotacaoPadrao = false;

    /// A posicao do jogador que este projetil persegue. Ver DefinirAlvo.
    size_t mAlvo = 0;

};
