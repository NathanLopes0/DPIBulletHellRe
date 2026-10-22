//
// O Blink: a fase final do Ricardo.
//

#pragma once

#include <optional>
#include <string>

/**
 * CAMADA PURA.
 *
 * O Ricardo pesquisa hardware, e a repescagem dele e o primeiro programa de
 * quem pega um Arduino: o BLINK, que acende o LED, espera, apaga, espera.
 *
 *     void loop() {
 *         digitalWrite(13, HIGH);  delay(1000);
 *         digitalWrite(13, LOW);   delay(1000);
 *     }
 *
 * Quatro placas ficam presas ao professor, duas de cada lado, e piscam o
 * Blink em dupla: enquanto o LED de um lado esta aceso, o do outro esta
 * apagado. A placa ACESA atira quando acende; a apagada nao. O PLACAR E O
 * SKETCH: cada etapa anuncia o delay ("delay(500);"), e e dali que as placas o
 * leem (LerDelay) - ele cai pela metade de etapa em etapa, e o pisca-pisca
 * acelera.
 *
 * Quem faz isso no jogo e o Ricardo (as placas em campo, o LED, o tiro); aqui
 * fica o relogio do Blink, coberto por test_blink.cpp.
 */
namespace Blink {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoBlink = "StateFinal";
    constexpr const char* kPostoDasPlacas = "placas";
    constexpr const char* kPostoDasAcesas = "acesas";
    constexpr const char* kGatilhoDoLed = "acender";
    constexpr const char* kAnimacaoApagada = "Apagada";
    constexpr const char* kAnimacaoAcesa = "Acesa";

    /// @brief O delay do sketch no placar, em segundos: o numero dentro de
    /// "delay(...)", em milissegundos como no Arduino. Sem ele, ou zero e
    /// negativo, nao ha delay.
    std::optional<float> LerDelay(const std::string& placar);

    /// O lado que esta aceso no instante t, piscando a cada 'delay' segundos:
    /// true = o da esquerda. Comeca pela esquerda.
    bool EsquerdaAcesa(float t, float delay);

    /// O LED trocou de lado entre 'antes' (exclusive) e 'agora' (inclusive)?
    /// E quando um lado acende - e as placas dele atiram.
    bool Trocou(float antes, float agora, float delay);
}
