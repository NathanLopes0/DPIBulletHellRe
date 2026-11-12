// Resolucao do referencial de um caminho: origem e orientacao.

#pragma once

#include "../Motor/Math.h"

/// Este arquivo nao conhece Actor, Projectile, Scene nem SDL. Ele recebe um retrato do mundo,
/// devolve geometria, e nada mais.

/// Regra que decide para onde o caminho aponta.
struct Mira {
    enum Modo {
        AlinharComVelocidade,  ///< Padrao: o caminho segue a direcao de voo.
        MirarNoJogador,        ///< Aponta para a posicao atual do jogador.
        MirarPrevendo,         ///< Aponta para onde o jogador estara. parametro = segundos.
        AnguloFixo             ///< Direcao absoluta. parametro = angulo em radianos.
    };

    Modo modo = AlinharComVelocidade;
    float parametro = 0.0f;

    constexpr Mira() = default;
    constexpr Mira(const Modo m, const float p = 0.0f) : modo(m), parametro(p) {}
};

/// Retrato do mundo no instante da ativacao. Preenchido pelo PathBehavior a partir do motor.
struct ContextoDeResolucao {
    Vector2 posicaoProjetil   = Vector2::Zero;
    Vector2 velocidadeProjetil = Vector2::Zero;
    Vector2 posicaoJogador     = Vector2::Zero;
    Vector2 velocidadeJogador  = Vector2::Zero;

    /// False quando a cena nao e Battle ou nao ha jogador vivo.
    bool temJogador = false;
};

/// Origem e orientacao com que a forma sera desenhada no mundo.
struct Referencial {
    Vector2 origem = Vector2::Zero;
    float cos = 1.0f;
    float sin = 0.0f;
};

/// Decide o referencial do caminho. Funcao pura.
Referencial ResolverReferencial(const ContextoDeResolucao& contexto, const Mira& mira);

/// Aponta a velocidade na direcao desejada preservando o modulo atual.
Vector2 DirecionarPreservandoModulo(const Vector2& velocidadeAtual,
                                    const Vector2& direcaoDesejada,
                                    float moduloMinimo);

/// Ajusta o modulo da velocidade mantendo a direcao.
Vector2 AjustarModulo(const Vector2& velocidade, float novoModulo);

/// O rumo de um tiro que sai de 'de' a 'rapidez' px/s para onde o alvo. Vai estar, se continuar
/// andando como anda.
Vector2 RumoPrevendo(const Vector2& de, const Vector2& alvo, const Vector2& velocidadeDoAlvo, float rapidez);

/// O ritmo de um ataque em ciclos: quando comeca, quanto dura cada fase e quantas vezes repete.
struct RitmoDeCiclos {
    float atraso = 0.0f;            ///< segundos ate o primeiro ciclo
    float duracaoInvestida = 0.2f;  ///< segundos de fase rapida
    float duracaoPausa = 1.0f;      ///< segundos de fase lenta
    int repeticoes = 1;             ///< quantas investidas ao todo

    /// Quanto dura um ciclo completo (investida + pausa).
    [[nodiscard]] constexpr float Ciclo() const { return duracaoInvestida + duracaoPausa; }

    /// Instante em que a investida de indice k comeca (k a partir de zero).
    [[nodiscard]] constexpr float InicioDaInvestida(const int k) const {
        return atraso + Ciclo() * static_cast<float>(k);
    }

    /// Quanto dura a sequencia inteira. Sao N investidas com N-1 pausas entre elas, entao a ultima
    /// pausa nao entra.
    [[nodiscard]] constexpr float DuracaoTotal() const {
        return repeticoes <= 0 ? 0.0f
                               : Ciclo() * static_cast<float>(repeticoes) - duracaoPausa;
    }
};

/// Onde um pulso investida/pausa esta num dado instante.
struct FaseDoPulso {
    bool ativo = false;       ///< false antes do atraso e depois da ultima repeticao
    bool investindo = false;  ///< true na investida, false na pausa
    int ciclo = 0;            ///< quantos ciclos completos ja passaram
    float modulo = 0.0f;
};

/// Calcula a fase do pulso. Funcao pura - nao conhece Projectile nem SDL.
FaseDoPulso CalcularFaseDoPulso(float tempoDecorrido, const RitmoDeCiclos& ritmo,
                                float moduloInvestida, float moduloPausa);
