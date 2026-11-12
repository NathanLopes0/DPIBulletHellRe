//
// A amostragem com temperatura de um modelo de linguagem.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// Um modelo de linguagem escolhe o proximo token sorteando entre os candidatos, com probabilidade
/// softmax(logit / T).
namespace Temperatura {

    /// Softmax(logit / T). Com T que nao e positiva, todo o peso vai para o maior logit (o
    /// primeiro, se houver empate): e a escolha gulosa.
    std::vector<float> Probabilidades(const std::vector<float>& logits, float temperatura);

    /// O candidato sorteado com 'u' em [0, 1): o primeiro cuja probabilidade acumulada passa de u.
    int Amostrar(const std::vector<float>& probabilidades, float u);

    /// Os candidatos de angulo: 'n' desvios de -abertura/2 a +abertura/2, e o
    /// logit de cada um, -|desvio| / escala. Com um candidato so, o desvio zero.
    struct Candidatos {
        std::vector<float> desvios;
        std::vector<float> logits;
    };
    Candidatos DeAngulo(int n, float abertura, float escala = 15.0f);

    // ---- As temperaturas de cada luta (a fase 1 do Julio) ----

    /// As tres etapas da fase - fria, morna e quente -, cada uma com a temperatura sorteada na
    /// faixa dela, em decimos para o placar dizer o numero exato.
    constexpr int kEtapas = 3;
    using Temperaturas = std::array<float, kEtapas>;
    constexpr Temperaturas kDeSempre = {0.2f, 0.8f, 1.6f};

    /// No Dificil o modelo roda mais quente - o degrau da fase 1: fria de 0,3 a 0,5, morna de 0,9 a
    /// 1,3 e quente de 1,8 a 2,2.
    Temperaturas Sortear(Variacao::Sorteio& s, Dificuldade::Nivel n = Dificuldade::Nivel::Normal);

    /// A temperatura das frases de "e" da repescagem: 0,3 no arquivo, 0,7 no Dificil.
    constexpr float kDoLoopNoDificil = 0.7f;
    constexpr float kDaPrevisaoNoDificil = 0.4f;

    /// O placar da etapa: "temperatura = 0,8"; na quente, "temperatura = 1,6 ·
    /// alucinando" (o ponto do meio em UTF-8, como em julio/fases.json).
    std::string Anuncio(float temperatura, bool alucinando);
}
