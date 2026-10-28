//
// A tela de carregamento da batalha: o ritmo da barra e do veu.
//

#pragma once

/**
 * CAMADA PURA. Coberto por test_carregamento.cpp.
 *
 * A BATALHA SO APARECE PRONTA. Enquanto os pools de projetil esquentam (um
 * tipo por quadro, Battle::UpdateLoadingStep), uma tela escura cobre tudo -
 * o chefe, os alunos, a faixa da nota - com uma barra enchendo. Pronta, a
 * tela some num esmaecer curto, e so ENTAO a luta comeca: o chefe ataca e a
 * musica toca.
 *
 * O QUE ESTA AQUI e o ritmo:
 *
 *   a barra ANDA atras do progresso de verdade, com uma rapidez maxima - um
 *   chefe com poucos tipos carrega em tres quadros, e uma barra que enche em
 *   tres quadros e um piscar, e nao uma tela;
 *   cheia, ela SEGURA um instante, para ser vista cheia;
 *   depois o veu SOME, e acabou.
 */
namespace Carregamento {

    struct Estado {
        float mostrado = 0.0f;   ///< o que a barra mostra, 0 a 1
        float cheia = -1.0f;     ///< ha quanto tempo ela esta cheia (negativo: ainda nao)
    };

    /// A barra anda no maximo isto por segundo: vazia a cheia em 0,7 s.
    constexpr float kRapidezMaxima = 1.0f / 0.7f;
    /// Cheia, segura isto...
    constexpr float kSegura = 0.25f;
    /// ...e o veu some em isto.
    constexpr float kSome = 0.35f;

    /// @brief Um quadro de 'dt' com 'feito' (0 a 1) do carregamento de verdade.
    void Avancar(Estado& e, float feito, float dt);

    /// @brief O carregamento acabou E a tela ja mostrou isso: a batalha pode
    /// comecar a aparecer (o veu comeca a sumir).
    bool Pronto(const Estado& e);

    /// @brief O veu ja sumiu: a luta comeca.
    bool Acabou(const Estado& e);

    /// @brief Quao opaco o veu esta: 1 ate Pronto, e cai a 0 ate Acabou.
    float Opacidade(const Estado& e);
}
