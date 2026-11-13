//
// A fase 1 do Arroyo: a arvore de recursao de T(n) = aT(n/b) + f(n).
//

#pragma once

#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// A analise de um algoritmo de divisao e conquista, jogada: o professor solta T(n), que se divide em
/// 'a' subproblemas de tamanho n/b, que se dividem de novo, ate as folhas. Cada no, ao se dividir,
/// paga o trabalho f(n) dele em tiros - e o custo de cada nivel da arvore fica na tela: no merge sort
/// todo nivel custa o mesmo (n log n), no Karatsuba as folhas dominam, na busca binaria e um tiro por
/// nivel (log n). O placar mostra a recorrencia e, quando a arvore termina, o Teorema Mestre.
///
/// O aluno pode podar a arvore: o no e derrubavel, e o que cai leva a subarvore dele junto.
namespace Recorrencia {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr const char* kGatilhoDoNo = "no";
    constexpr const char* kGatilhoDoTrabalho = "trabalho";
    constexpr const char* kGatilhoDaFolha = "folha";

    /// Uma recorrencia T(n) = aT(n/b) + n^k, com o algoritmo que ela descreve.
    struct Caso {
        int a = 2;
        int b = 2;
        int k = 1;
        const char* algoritmo = "";
    };

    /// As recorrencias da fase. A primeira, o merge sort, abre toda luta: e o exemplo de sala.
    const std::vector<Caso>& Casos();

    /// Quantas arvores cabem numa fase, uma a cada kDuracaoDaRodada.
    constexpr int kRodadas = 5;
    constexpr float kDuracaoDaRodada = Fase::kDuracao / static_cast<float>(kRodadas);

    /// A ordem das recorrencias desta luta: kRodadas indices de Casos(), o merge sort primeiro.
    std::vector<int> Sortear(Variacao::Sorteio& s);

    /// "T(n) = 2T(n/2) + n".
    std::string Formula(const Caso& c);

    /// O Teorema Mestre: "Θ(n log n)", "Θ(log n)", "Θ(n^1,58)"...
    std::string Solucao(const Caso& c);

    /// O placar da rodada: a formula, e depois da arvore pronta, "= Θ(...)".
    std::string Placar(const Caso& c, bool pronta);

    /// Quantas vezes a arvore se divide: as folhas ficam no nivel Niveis(c). Com mais filhos, menos
    /// niveis, para a tela nao virar so tiro (a^Niveis folhas).
    int Niveis(const Caso& c);

    /// Quantos tiros um no do nivel 'nivel' solta ao se dividir: o trabalho f(n/b^nivel), com n
    /// valendo kN tiros de trabalho na raiz.
    constexpr int kN = 8;
    int Trabalho(const Caso& c, int nivel);

    /// Os desvios (graus) dos 'a' filhos em relacao ao rumo do pai, do mais a esquerda ao mais a
    /// direita. Um filho so segue reto.
    std::vector<float> Desvios(int a);

    /// O rotulo do no: "n", "n/2", "n/4"...; o da folha e "1".
    std::string Rotulo(const Caso& c, int nivel);

    /// A cara do no em projeteis.json: "N", "N2", "N4", "N8", "N3", "N9" ou "Um" (a folha).
    std::string Cara(const Caso& c, int nivel);

    /// Quantos acertos o no aguenta antes de cair: a raiz e o problema inteiro.
    int Resistencia(int nivel);

    /// O que o Dificil muda: a arvore se divide mais depressa, e anda mais depressa.
    struct Degrau {
        float porNivel = 0.78f;      ///< segundos de vida de um no antes de se dividir
        float velocidade = 105.0f;   ///< px/s do no
        float doTrabalho = 170.0f;   ///< px/s dos tiros de trabalho
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
