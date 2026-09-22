//
// Resolucao do REFERENCIAL de um caminho: origem e orientacao.
//

#pragma once

#include "../Math.h"

/**
 * CAMADA PURA (Humble Object)
 *
 * Este arquivo nao conhece Actor, Projectile, Scene nem SDL. Ele recebe um
 * retrato do mundo, devolve geometria, e nada mais. Toda a logica de decisao
 * do referencial mora aqui justamente para poder ser testada em milissegundos,
 * sem subir o jogo.
 *
 * Quem faz a ponte com o motor e PathBehavior::update, que le os componentes,
 * monta o ContextoDeResolucao e chama ResolverReferencial. Essa parte fica
 * trivial demais para errar.
 */

/**
 * @brief Regra que decide para onde o caminho aponta.
 *
 * E um dado por valor, e nao uma hierarquia com heranca, de proposito: um
 * unique_ptr<IResolvedor> ou um std::function alocariam no heap POR PROJETIL,
 * que e exatamente o custo que o Flyweight das formas removeu. Esta struct cabe
 * em 8 bytes e e copiavel.
 *
 * O conjunto de modos e fechado por natureza (ha quatro jeitos de mirar, nao
 * dezenas), e Strategy so se paga quando o conjunto e aberto.
 */
struct Mira {
    enum Modo {
        AlinharComVelocidade,  ///< Padrao: o caminho segue a direcao de voo.
        MirarNoJogador,        ///< Aponta para a posicao atual do jogador.
        MirarPrevendo,         ///< Aponta para onde o jogador estara. parametro = segundos.
        AnguloFixo             ///< Direcao absoluta. parametro = angulo em RADIANOS.
    };

    Modo modo = AlinharComVelocidade;
    float parametro = 0.0f;

    constexpr Mira() = default;
    constexpr Mira(const Modo m, const float p = 0.0f) : modo(m), parametro(p) {}
};

/**
 * @brief Retrato do mundo no instante da ativacao.
 *
 * Preenchido pelo PathBehavior a partir do motor. Existe para que a funcao de
 * resolucao nao precise conhecer nenhuma classe do jogo.
 */
struct ContextoDeResolucao {
    Vector2 posicaoProjetil   = Vector2::Zero;
    Vector2 velocidadeProjetil = Vector2::Zero;
    Vector2 posicaoJogador     = Vector2::Zero;
    Vector2 velocidadeJogador  = Vector2::Zero;

    /// false quando a cena nao e Battle ou nao ha jogador vivo.
    bool temJogador = false;
};

/**
 * @brief Origem e orientacao com que a forma sera desenhada no mundo.
 *
 * cos e sin sao guardados prontos em vez do angulo porque o PathBehavior os usa
 * diretamente para girar cada waypoint, todo frame.
 */
struct Referencial {
    Vector2 origem = Vector2::Zero;
    float cos = 1.0f;
    float sin = 0.0f;
};

/**
 * @brief Decide o referencial do caminho. Funcao PURA.
 *
 * CADEIA DE FALLBACK (requisito RNF3): se o modo pedido nao puder ser
 * resolvido - sem jogador, ou jogador exatamente em cima do projetil - cai para
 * AlinharComVelocidade; se tambem nao houver velocidade, devolve identidade
 * (a forma sai como foi escrita). Nunca devolve NaN e nunca falha.
 */
Referencial ResolverReferencial(const ContextoDeResolucao& contexto, const Mira& mira);

/**
 * @brief Aponta a velocidade na direcao desejada PRESERVANDO o modulo atual.
 *
 * Este e o contrato de toda ProjectileMotion: a Motion decide a DIRECAO, e o
 * modulo e de quem ja o tinha - inclusive dos Modifiers que rodaram no frame
 * anterior. Sem isto, um SlowDown aplicado depois que um caminho ativa durava
 * exatamente um frame: a Motion reescrevia o modulo original no frame seguinte.
 *
 * @param velocidadeAtual velocidade antes deste passo
 * @param direcaoDesejada para onde apontar; nao precisa estar normalizada
 * @param moduloMinimo usado quando a velocidade atual e nula, para o projetil
 *        nunca ficar parado (projetil parado nunca sai da tela e nunca volta ao
 *        pool)
 * @return a nova velocidade. Se direcaoDesejada for nula, devolve
 *         velocidadeAtual sem alteracao.
 */
Vector2 DirecionarPreservandoModulo(const Vector2& velocidadeAtual,
                                    const Vector2& direcaoDesejada,
                                    float moduloMinimo);
