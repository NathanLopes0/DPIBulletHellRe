//
// A amostragem com temperatura de um modelo de linguagem.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Dificuldade.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * Um modelo de linguagem escolhe o proximo token SORTEANDO entre os candidatos,
 * com probabilidade softmax(logit / T). A TEMPERATURA T e o botao: perto de
 * zero o sorteio vira escolha - sai sempre o mais provavel -, e alta ela
 * achata as probabilidades e sai qualquer coisa, a "alucinacao".
 *
 * Na fase 1 do Julio os candidatos sao ANGULOS: o tiro mirado no aluno e o
 * mais provavel, e quanto mais longe da mira, menor o logit. O placar diz a
 * temperatura de cada etapa. Quem sorteia no jogo e a TokenAttack; aqui ficam
 * as contas, cobertas por test_temperatura.cpp.
 */
namespace Temperatura {

    /// softmax(logit / T). Com T que nao e positiva, todo o peso vai para o
    /// maior logit (o primeiro, se houver empate): e a escolha gulosa.
    std::vector<float> Probabilidades(const std::vector<float>& logits, float temperatura);

    /// O candidato sorteado com 'u' em [0, 1): o primeiro cuja probabilidade
    /// acumulada passa de u. Um u de 1 ou mais (o limite que o sorteio pode
    /// devolver) cai no ultimo candidato com probabilidade.
    int Amostrar(const std::vector<float>& probabilidades, float u);

    /// Os candidatos de angulo: 'n' desvios de -abertura/2 a +abertura/2, e o
    /// logit de cada um, -|desvio| / escala. Com um candidato so, o desvio zero.
    struct Candidatos {
        std::vector<float> desvios;
        std::vector<float> logits;
    };
    Candidatos DeAngulo(int n, float abertura, float escala = 15.0f);

    // ---- AS TEMPERATURAS DE CADA LUTA (a fase 1 do Julio) -------------------

    /// As tres etapas da fase - fria, morna e quente -, cada uma com a
    /// temperatura sorteada na faixa dela, em decimos para o placar dizer o
    /// numero exato: fria de 0,1 a 0,3, morna de 0,6 a 1,0, quente de 1,4 a
    /// 1,8. A de sempre e 0,2, 0,8 e 1,6.
    constexpr int kEtapas = 3;
    using Temperaturas = std::array<float, kEtapas>;
    constexpr Temperaturas kDeSempre = {0.2f, 0.8f, 1.6f};
    Temperaturas Sortear(Variacao::Sorteio& s);

    /// @brief No Dificil o modelo roda mais quente - o degrau da fase 1: fria
    /// de 0,3 a 0,5, morna de 0,9 a 1,3 e quente de 1,8 a 2,2. Os nomes das
    /// etapas sao os mesmos, e o placar diz o numero. No Normal, as faixas de
    /// cima.
    Temperaturas Sortear(Variacao::Sorteio& s, Dificuldade::Nivel n);

    /// A temperatura das frases de "e" da repescagem: 0,3 no arquivo, 0,7 no
    /// Dificil. E a dos tokens da previsao da fase 3: zero no arquivo - o
    /// modelo te prediz -, 0,4 no Dificil, que solta dois de cada vez.
    constexpr float kDoLoopNoDificil = 0.7f;
    constexpr float kDaPrevisaoNoDificil = 0.4f;

    /// O placar da etapa: "temperatura = 0,8"; na quente, "temperatura = 1,6 ·
    /// alucinando" (o ponto do meio em UTF-8, como em julio/fases.json).
    std::string Anuncio(float temperatura, bool alucinando);
}
