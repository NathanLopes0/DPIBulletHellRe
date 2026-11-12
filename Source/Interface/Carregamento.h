//
// A tela de carregamento da batalha: o ritmo da barra e do veu.
//

#pragma once

/// A batalha so aparece pronta.
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

    /// Um quadro de 'dt' com 'feito' (0 a 1) do carregamento de verdade.
    void Avancar(Estado& e, float feito, float dt);

    /// O carregamento acabou E a tela ja mostrou isso: a batalha pode comecar a aparecer (o veu
    /// comeca a sumir).
    bool Pronto(const Estado& e);

    bool Acabou(const Estado& e);

    /// Quao opaco o veu esta: 1 ate Pronto, e cai a 0 ate Acabou.
    float Opacidade(const Estado& e);
}
