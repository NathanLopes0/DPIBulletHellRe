//
// O clock da fase 2 do Ricardo.
//

#pragma once

#include <cstdint>
#include <optional>
#include <string>

/**
 * CAMADA PURA.
 *
 * A fase 2 do Ricardo e Organizacao de Computadores, e o coracao de um
 * computador e o CLOCK: uma onda quadrada, e a cada borda de subida os
 * registradores copiam o que tem na entrada - o dado anda UM registrador. A
 * fase e isso, em tres pecas:
 *
 *   o professor ANDA NA ONDA QUADRADA: no nivel alto em cima, no baixo
 *   embaixo, saltando nas bordas (OndaQuadradaStrategy);
 *
 *   os tiros dele sao REGISTRADORES: parados entre uma borda e outra, e a cada
 *   borda de subida dao um passo de kPasso pixels - o tiro anda na velocidade
 *   do clock (Ricardo::AtualizarRelogio);
 *
 *   a cada segundo sai o valor de um CONTADOR JOHNSON - o registrador de
 *   deslocamento com a saida invertida de volta na entrada - como um leque de
 *   bits: o 1 machuca, o 0 e a fresta.
 *
 * O PLACAR E O CLOCK, como na fase 1 o placar e a porta: cada etapa anuncia a
 * frequencia ("clock 2 Hz"), e e dali que o professor e os tiros a leem
 * (LerFrequencia). A frequencia DOBRA de etapa em etapa, ate o overclock.
 *
 * O TEMPO E O DA FASE. As bordas de subida da frequencia f ficam em k/f
 * segundos do comeco da fase; com f inteiro e etapas que mudam em segundos
 * inteiros, toda troca de frequencia cai numa borda, e todo segundo inteiro e
 * borda de qualquer um dos clocks - e ai que o contador anda.
 */
namespace Relogio {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoRelogio = "StateTwo";
    constexpr const char* kGatilhoDoContador = "contador";
    constexpr const char* kPostoDosRegistradores = "registradores";

    /// Quanto um registrador anda a cada borda de subida: um bit e um pouco.
    /// A 1 Hz o leque de cada segundo sai quando o anterior acaba de dar um
    /// passo, e com 64 px - menos que os 70 do bit - os dois se encavalavam.
    /// A 1 Hz o tiro anda 72 px/s; no overclock de 8 Hz, 576.
    constexpr float kPasso = 72.0f;


    /// @brief A frequencia escrita no placar: o numero logo antes de "Hz"
    /// ("clock 2 Hz", "overclock 8 Hz"; a virgula vale como ponto). Sem "Hz",
    /// sem numero, ou zero e negativo, nao e frequencia.
    std::optional<float> LerFrequencia(const std::string& placar);

    /// Houve borda de subida de um clock de 'frequencia' entre 'antes'
    /// (exclusive) e 'agora' (inclusive)? O instante zero nao e borda.
    bool Subiu(float antes, float agora, float frequencia);

    /// O clock esta no nivel alto em 't'? A primeira metade de cada periodo,
    /// logo depois da borda de subida.
    bool Alto(float t, float frequencia);

    /// Quanto dura o passo do registrador: depressa, para o passo ser um
    /// salto, e sempre menos que o meio periodo - ele para antes da proxima
    /// borda.
    float DuracaoDoPasso(float frequencia);

    /// @brief O estado k de um contador Johnson de 'bits' bits: os uns entram
    /// pela esquerda ate encher, e saem pela esquerda ate esvaziar - 2*bits
    /// estados por volta. 0000, 1000, 1100, 1110, 1111, 0111, 0011, 0001, 0000...
    std::uint32_t Johnson(int k, int bits);
}
