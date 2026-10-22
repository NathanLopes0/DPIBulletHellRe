//
// A amostragem com temperatura de um modelo de linguagem.
//

#pragma once

#include <vector>

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
}
