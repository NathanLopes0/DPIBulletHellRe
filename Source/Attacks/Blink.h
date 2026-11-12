//
// O Blink: a fase final do Ricardo.
//

#pragma once

#include <string>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <optional>
#include <string>

/// O Ricardo pesquisa hardware, e a repescagem dele e o primeiro programa de quem pega um Arduino:
/// o blink, que acende o led, espera, apaga, espera.
namespace Blink {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoBlink = "StateFinal";
    constexpr const char* kPostoDasPlacas = "placas";
    constexpr const char* kPostoDasAcesas = "acesas";
    constexpr const char* kGatilhoDoLed = "acender";
    constexpr const char* kAnimacaoApagada = "Apagada";
    constexpr const char* kAnimacaoAcesa = "Acesa";

    /// O delay do sketch no placar, em segundos: o numero dentro de "delay(...)", em milissegundos
    /// como no Arduino.
    std::optional<float> LerDelay(const std::string& placar);

    /// O lado que esta aceso no instante t, piscando a cada 'delay' segundos:
    /// true = o da esquerda. Comeca pela esquerda.
    bool EsquerdaAcesa(float t, float delay);

    /// E quando um lado acende - e as placas dele atiram.
    bool Trocou(float antes, float agora, float delay);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// O sketch desta luta: o delay de partida (800, 1000 ou 1200 ms), que
    /// cai pela metade de etapa em etapa como sempre.
    struct Plano {
        int delayInicial = 1000;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// O que o Dificil muda: o sketch e mais apressado - o delay e 80% do da luta (escrito assim no
    /// placar, que e a regra) - e o led atira 20% mais depressa.
    struct Degrau {
        float delay = 1.0f;    ///< multiplica o delay do plano
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O placar da etapa k (0, 1, 2): "digitalWrite(13, HIGH); delay(500);",
    /// com o delay do plano no modo, pela metade a cada etapa.
    std::string Anuncio(int etapa, const Plano& p, const Degrau& d);
}
