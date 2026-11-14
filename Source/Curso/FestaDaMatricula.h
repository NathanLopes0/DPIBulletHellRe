//
// A festa da matricula: o que a selecao de fases desenha quando uma optativa acaba de ser comprada.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"

/// A compra de uma optativa vira uma pequena festa no losango dela, em tres tempos:
///   - as horas pagam: uma ficha dourada por kHorasPorFicha horas do preco sai dos blocos de horas do
///     painel de matricula e salta em arco ate o losango, girando como moeda, uma atras da outra;
///   - o carimbo: quando a ultima chega, o losango da dois pulinhos, um anel dourado em losango se
///     abre em volta dele e as faiscas espirram e caem;
///   - e a faixa de "matriculado" fica no alto.
/// Tudo e funcao do tempo desde o comeco da festa: a cena so pergunta onde cada coisa esta.
namespace FestaDaMatricula {

    constexpr int kHorasPorFicha = 5;
    constexpr int kMenosFichas = 3;
    constexpr int kMaisFichas = 8;
    /// Quantas fichas pagam um preco de 'preco' horas.
    int Fichas(int preco);

    constexpr float kEntreFichas = 0.11f;   ///< uma sai depois da outra
    constexpr float kVooDaFicha = 0.6f;     ///< o arco de cada uma
    constexpr float kAlturaDoArco = 110.0f; ///< acima do mais alto dos dois pontos

    /// Um ponto da parabola de 'de' a 'para' em 'u' (0 a 1), subindo 'altura' acima do mais alto.
    Vector2 NoArco(Vector2 de, Vector2 para, float altura, float u);

    struct Ficha {
        Vector2 onde;
        float largura = 1.0f;   ///< 0 a 1: a moeda girando, de frente a de lado
        bool voando = false;
    };
    /// A ficha 'k' de 'n' no instante 't': sai de 'de' (cada uma um pouco ao lado, na fileira dos
    /// blocos) e chega em 'para'.
    Ficha FichaEm(int k, int n, float t, Vector2 de, Vector2 para);

    /// Quando a ultima ficha chega: o carimbo.
    float Carimbo(int fichas);

    /// Quanto o losango sobe (px, para cima) 't' segundos depois do carimbo: dois pulinhos, o
    /// segundo menor, e parado depois.
    float Pulo(float desdeOCarimbo);
    constexpr float kPrimeiroPulo = 26.0f;
    constexpr float kDuracaoDoPulo = 0.7f;

    /// O anel em losango que se abre do carimbo: o raio (do centro a ponta) e a opacidade (0 a 1).
    struct Anel {
        float raio = 0.0f;
        float opacidade = 0.0f;
    };
    Anel AnelEm(float desdeOCarimbo, float raioInicial);
    constexpr float kVidaDoAnel = 0.55f;

    /// As faiscas do carimbo: espirram em volta do centro e caem (a gravidade).
    struct Faisca {
        Vector2 onde;
        float opacidade = 0.0f;
    };
    std::vector<Faisca> Faiscas(Vector2 centro, float desdeOCarimbo, int quantas);
    constexpr int kFaiscas = 18;
    constexpr float kVidaDaFaisca = 0.9f;
    constexpr float kGravidade = 520.0f;

    /// A festa inteira dura isto, para 'fichas' fichas.
    float Duracao(int fichas);
}
