//
// O modo de plataforma do aluno: a gravidade puxa para um dos quatro lados do campo.
//

#pragma once

#include "../Motor/Math.h"

/// Quando um professor vira o jogo de plataforma (o Lucas Ferreira, na INF 216), o aluno deixa de
/// voar: cai para um dos lados do campo, anda ao longo dele e pula para o lado oposto. Segurar o pulo
/// pula mais alto; soltar no meio corta. Nao conhece aluno, campo nem SDL: quem usa e Player.
namespace Plataforma {

    /// Para que lado a gravidade puxa: o chao e esse lado do campo.
    enum class Lado { Baixo, Esquerda, Cima, Direita };

    /// O vetor unitario da gravidade: Baixo e (0, 1).
    Vector2 Direcao(Lado l);
    /// O eixo do chao: andar e ir ao longo dele. Baixo e (1, 0); os outros sao o de Baixo girado junto
    /// com a gravidade (Esquerda desce, Cima vai para a esquerda, Direita sobe).
    Vector2 AoLongo(Lado l);
    /// Quantos graus (horario, como o SDL) o desenho do aluno gira para os pes ficarem no chao de 'l'.
    float Giro(Lado l);

    /// Em px/s/s: caindo, ou subindo com o pulo ja solto.
    constexpr float kGravidade = 2100.0f;
    /// Em px/s/s: subindo com o pulo segurado - e o que deixa o pulo longo.
    constexpr float kGravidadeSegurando = 950.0f;
    /// A velocidade com que o pulo sai do chao.
    constexpr float kPulo = 640.0f;
    /// Soltar o pulo na subida corta a velocidade de subida a isto: o toque e um pulinho.
    constexpr float kCorteDoPulo = 240.0f;
    constexpr float kQuedaMaxima = 950.0f;
    /// A velocidade de quem a gravidade acabou de virar: e arremessado ao chao novo.
    constexpr float kBatida = 1000.0f;

    struct Estado {
        /// A velocidade ao longo da gravidade: positiva cai, negativa sobe.
        float queda = 0.0f;
        bool noChao = false;
    };

    /// Um quadro de 'dt' segundos. 'intencao' e o que o manche pede, em px/s (como a velocidade do
    /// voo): o que vai ao longo do chao anda; empurrar para longe do chao pula. 'encostou' diz se o
    /// aluno esta encostado no lado da gravidade. Devolve a velocidade do aluno.
    Vector2 Passo(Estado& e, Lado l, Vector2 intencao, bool encostou, float dt, float andar);

    /// A gravidade virou (ou acabou de ligar): o aluno e arremessado ao chao novo.
    void Virar(Estado& e);

    /// A altura do pulo segurado ate o alto, e a do toque (solto no primeiro quadro).
    float AlturaSegurando();
    float AlturaDoToque();
}
