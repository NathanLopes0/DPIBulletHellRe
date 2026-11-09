//
// Como a nota cresce durante uma batalha.
//

#pragma once

#include <cstddef>

#include "../Attacks/Fase.h"

/// So aritmetica de nota; nao conhece batalha, chefe, SDL nem disco.
namespace Nota {

    /// Abaixo disto o ganho e cheio.
    inline constexpr float kFaixaFacil = 60.0f;

    /// A nota em que o ganho chegaria a zero. Fica acima de 100: em 100 ainda sobra ganho, pequeno,
    /// e por isso a nota cheia e alcancavel.
    inline constexpr float kAssintota = 110.0f;

    /// Quao rapido o ganho mingua acima de kFaixaFacil.
    inline constexpr float kExpoente = 1.5f;

    inline constexpr float kDuracaoDaFaseMedida = 17.0f;
    inline constexpr float kGanhoPorAcertoEm17s = 0.56f;

    /// O ganho bruto de cada tiro que chega no chefe. Proporcional ao tempo da fase.
    inline constexpr float kGanhoPorAcerto = kGanhoPorAcertoEm17s * kDuracaoDaFaseMedida / Fase::kDuracao;

    /// O maximo de quem levou dano demais. Nao e 100, e esse e o ponto: a nota cheia fica reservada
    /// a quem quase nao foi atingido.
    inline constexpr float kTetoComDano = 99.99f;

    /// A partir de quantos acertos sofridos o teto passa a valer.
    inline constexpr int kAcertosQuePerdemACheia = 3;

    /// A nota de aprovacao. Vale 60 como kFaixaFacil, e isso e coincidencia.
    inline constexpr float kNotaAprovacao = 60.0f;

    /// O ganho bruto por terminar uma fase sem ser atingido.
    inline constexpr float kBonusDeFaseLimpa = 12.0f;

    /// Quanto um acerto sofrido custa, em bruto, antes da divisao entre os jogadores
    /// (PerdaPorJogador). Sozinho, um acerto tira 6 pontos.
    inline constexpr float kPerdaPorAcerto = 6.0f;

    /// O piso da faixa de exame: daqui ate a aprovacao, a batalha segue para a fase final em vez de
    /// terminar.
    inline constexpr float kNotaDeExame = 40.0f;

    inline constexpr float kNotaMaxima = 100.0f;
    inline constexpr float kNotaMinima = 0.0f;

    /// Quanto de um ganho bruto sobra, dada a nota atual. Abaixo de kFaixaFacil devolve o ganho
    /// inteiro.
    float Ganho(float notaAtual, float ganhoBruto);

    /// Se este numero de acertos sofridos ja custou a nota cheia.
    bool PerdeuACheia(int acertosSofridos);

    /// Se esta nota E a nota cheia - a que vale o dourado e o teste final.
    bool ECheia(float nota);

    /// O que acontece ao fim da terceira fase de um chefe.
    enum class Desfecho {
        Reprovado,     ///< abaixo de 40: a batalha acaba e o aluno nao passou
        Aprovado,      ///< 60 ou mais, sem ser a nota cheia: acaba e passou
        VaiParaFinal   ///< a batalha continua na StateFinal
    };

    /// Para onde a batalha vai quando a terceira fase termina.
    Desfecho AposTerceiraFase(float nota);

    /// O maior valor que a nota pode atingir com este tanto de dano.
    float TetoCom(int acertosSofridos);

    /// A nota depois de somar um ganho bruto.
    float Somar(float notaAtual, float ganhoBruto, int acertosSofridos);

    /// A nota depois de uma perda. A perda nao passa pela curva: perder e perder.
    float Subtrair(float notaAtual, float perda);

    /// O que um jogador causa, repartido pelos que estao em campo. A nota do co-op e uma so, e sem
    /// isto ela seria outra coisa em cada direcao.
    float PorJogador(float bruto, size_t quantosJogadores);

    /// O alivio que a perda - e so a perda - ganha em grupo, alem da divisao.
    constexpr float kAlivioDaPerdaEmGrupo = 0.8f;

    /// O que um acerto sofrido custa, com a divisao e o alivio.
    float PerdaPorJogador(float bruto, size_t quantosJogadores);

    // ---- A duvida perguntada de perto ----

    /// Quem pergunta de perto arrisca mais - menos tempo para desviar, e o professor ao alcance de
    /// um esbarrao -, e a resposta vale mais: o ganho do acerto cresce de 1, de kLonge para la, ate
    /// kFatorDePerto, de kPerto para ca, em linha reta no meio. A distancia vai do centro de quem
    /// perguntou ao do professor, quando a resposta chega. Do fundo do campo o fator e 1, como antes.
    inline constexpr float kPerto = 170.0f;
    inline constexpr float kLonge = 470.0f;
    inline constexpr float kFatorDePerto = 1.5f;

    /// Quanto o ganho de um acerto se multiplica, com quem perguntou a 'distancia' do professor.
    float FatorDePerto(float distancia);

    /// A partir deste fator a resposta sai com dois pontos de exclamacao: a faixa de perto, que o
    /// aluno ve sem ler numero nenhum.
    inline constexpr float kFatorDaRespostaDupla = 1.35f;
}
