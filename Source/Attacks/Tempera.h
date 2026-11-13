//
// A repescagem do Arroyo: a tempera simulada.
//

#pragma once

#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// Quando o problema e dificil demais, aproxima-se: a heuristica da tempera simulada. Um enxame de
/// solucoes anda pelo campo procurando o minimo de uma energia cujo vale mais fundo e o aluno - e
/// dois vales rasos, os otimos locais. Quente, cada solucao aceita ate passo para pior, e o enxame se
/// espalha; esfriando, so aceita o que melhora, e o enxame desce para os vales - quem caiu num otimo
/// local fica preso la. No meio da fase a temperatura sobe de novo (o reaquecimento), e o enxame se
/// solta.
///
/// O criterio e o de Metropolis, de verdade: o passo para pior (delta > 0) e aceito com chance
/// exp(-delta / T).
namespace Tempera {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateFinal";
    constexpr const char* kGatilhoDaSolucao = "solucao";
    constexpr const char* kGatilhoDoVale = "vale";
    constexpr const char* kGatilhoDoCongelado = "congelou";

    /// A temperatura no instante 't' da fase: comeca em kQuente, esfria geometricamente ate quase
    /// zero e, em kReaquecimento, volta a kQuente.
    constexpr float kQuente = 1000.0f;
    constexpr float kFrio = 8.0f;
    constexpr float kReaquecimento = Fase::kDuracao / 2.0f;
    float Temperatura(float t);

    /// A energia de uma solucao em 'x': a distancia ao vale mais perto, com o vale do aluno
    /// kFundoDoAluno pixels mais fundo que os outros. Quanto menor, melhor.
    constexpr float kFundoDoAluno = 140.0f;
    float Energia(const Vector2& x, const Vector2& aluno, const std::vector<Vector2>& vales);

    /// Metropolis: aceita o passo de energia 'delta' a temperatura 't', com o 'sorteio' em [0, 1)?
    /// O que melhora sempre passa. A energia esta em pixels; a temperatura, em graus da tela -
    /// kEscala converte.
    constexpr float kEscala = 0.12f;
    bool Aceita(float delta, float temperatura, float sorteio);

    /// A chance de aceitar um passo de 'delta' pixels para pior: o que o placar mostra.
    float ChanceDePior(float delta, float temperatura);

    /// Um passo proposto a partir de 'x': uma direcao qualquer, de comprimento que cresce com a
    /// temperatura (de kPassoFrio a kPassoQuente).
    constexpr float kPassoQuente = 120.0f;
    constexpr float kPassoFrio = 18.0f;
    Vector2 Proposta(const Vector2& x, float temperatura, Variacao::Sorteio& s);

    /// O placar: "T = 820  ·  aceita pior: 61%".
    std::string Placar(float temperatura);

    /// O passo de referencia do placar: quanto pior e "pior".
    constexpr float kPiorDeReferencia = 40.0f;

    /// O que o Dificil muda: mais solucoes, decidindo mais depressa.
    struct Degrau {
        int solucoes = 14;
        float porPasso = 0.16f;       ///< segundos entre duas propostas de cada solucao
        float rapidez = 150.0f;       ///< px/s maximo de uma solucao indo ao lugar novo
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
