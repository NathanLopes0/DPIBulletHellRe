//
// A comunicacao minima da dupla: o sinal do Botao 3 e o "valeu" de quem foi coberto.
//

#pragma once

#include <vector>

/// Dois desconhecidos no gabinete nao tem como se chamar dentro do jogo, e quem cobre o parceiro so
/// ve que contou no resultado. Um canal minimo resolve os dois (requisitos-coop, 2.8): o sinal, que
/// o aluno da com o Botao 3, e o coracao que sobe de quem foi coberto, na cor de quem cobriu. Nao
/// conhece batalha, aluno nem SDL.
namespace Comunicacao {

    // ---- O sinal ----

    /// Entre dois sinais do mesmo aluno: segurar ou martelar o botao nao vira um pisca-pisca.
    constexpr float kEsperaDoSinal = 1.0f;

    /// O instante de quem ainda nao deu nenhum sinal.
    constexpr float kNuncaSinalizou = -1.0e9f;

    /// Quanto o anel do sinal leva para abrir e sumir.
    constexpr float kDuracaoDoSinal = 0.6f;

    /// O aluno que deu o ultimo sinal em 'ultimo' pode dar outro em 'agora'?
    bool PodeSinalizar(float agora, float ultimo);

    /// O raio do anel 't' segundos depois do sinal: abre depressa e desacelera.
    float RaioDoSinal(float t);

    /// Quao aceso o anel esta (0 a 1): apaga ate o fim de kDuracaoDoSinal.
    float AlfaDoSinal(float t);

    // ---- O valeu ----

    /// Quanto o coracao leva para subir e sumir.
    constexpr float kDuracaoDoValeu = 0.9f;

    /// Onde ele nasce, acima do centro do aluno coberto: acima do "v" da mira, que fica em 44.
    constexpr float kAlturaDoValeu = 64.0f;

    /// Quanto ele ja subiu 't' segundos depois de nascer.
    float SubidaDoValeu(float t);

    /// Quao aceso ele esta (0 a 1): inteiro, e apaga no fim.
    float AlfaDoValeu(float t);

    /// Uma linha do coracao em pixel art: na 'linha', os pixels de 'de' a 'ate'.
    struct Faixa {
        int linha;
        int de;
        int ate;
    };

    /// O coracao, 9 x 8 pixels, linha a linha. Cada pixel ocupa kPixelDoCoracao pixels de tela.
    const std::vector<Faixa>& Coracao();
    constexpr int kLarguraDoCoracao = 9;
    constexpr int kAlturaDoCoracao = 8;
    constexpr int kPixelDoCoracao = 2;
}
