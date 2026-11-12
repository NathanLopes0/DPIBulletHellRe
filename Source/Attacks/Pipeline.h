//
// O pipeline da fase 3 do Ricardo.
//

#pragma once

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <array>
#include <string>
#include <vector>

/// A fase 3 do Ricardo e Arquitetura de Computadores: um pipeline de cinco estagios.
namespace Pipeline {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoPipeline = "StateThree";
    constexpr const char* kGatilhoDosEstagios = "estagios";
    constexpr const char* kGatilhoDaBusca = "buscar";
    constexpr const char* kPostoDosEstagios = "estagios";
    constexpr const char* kPostoDoPipeline = "pipeline";
    constexpr const char* kAnimacaoDaBolha = "bolha";

    /// Quanto dura um ciclo do pipeline: a cada um, todo mundo anda um estagio
    /// e uma instrucao entra. O primeiro vem logo depois de os estagios se
    /// formarem.
    constexpr float kCiclo = 0.75f;
    constexpr float kPrimeiroCiclo = 0.5f;

    constexpr int kEstagios = 5;
    enum Estagio { IF = 0, ID = 1, EX = 2, MEM = 3, WB = 4 };

    /// O nome do estagio, como a arte e o placar o escrevem.
    const char* NomeDoEstagio(int estagio);

    enum class Tipo { Lw, Add, Addi, Bne, Sw, J };

    struct Instrucao {
        Tipo tipo;
        std::string texto;   ///< a linha de assembly, para o placar
        int alvo = -1;       ///< para onde desvia ou salta
    };

    const std::vector<Instrucao>& Programa();

    /// Quantas voltas o laco da: nas primeiras kVoltas-1 o bne desvia, e a
    /// previsao acerta; na ultima nao desvia, e ela erra.
    constexpr int kVoltas = 3;

    /// Um estagio vazio.
    constexpr int kBolha = -1;

    struct Estado {
        std::array<int, kEstagios> estagio{kBolha, kBolha, kBolha, kBolha, kBolha};
        int pc = 0;           ///< a proxima instrucao a buscar
        int desvios = 0;      ///< quantos bne ja se resolveram nesta volta do programa
        int voltas = kVoltas; ///< quantas voltas o laco da (o vetor tem tantos elementos)
    };

    /// O que um ciclo fez.
    struct Ciclo {
        bool flush = false;   ///< Um bne mal previsto esvaziou o if e o ID.
        int saiu = kBolha;    ///< O que saiu pelo wb.
    };

    Ciclo Avancar(Estado& e);

    /// O aluno derrubou a instrucao do estagio: ela vira bolha ali.
    void Estourar(Estado& e, int estagio);

    /// A animacao da instrucao ("lw", "add", ...).
    const char* Animacao(Tipo t);

    /// O gatilho que a instrucao puxa no estagio - a soma no ex, o load e o store no mem -, ou
    /// vazio. O bne e o j nao atacam: o desvio ja e o que eles fazem.
    std::string Gatilho(Tipo t, int estagio);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// O programa desta luta: o vetor tem 2, 3 ou 4 elementos - o laco da
    /// tantas voltas, e o flush do bne mal previsto vem depois delas.
    struct Plano {
        int voltas = kVoltas;
    };
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kMenosVoltas = 2;
    constexpr int kMaisVoltas = 4;

    /// O que o Dificil muda: o ciclo e mais curto (0,62 s), e o ataque de cada
    /// estagio anda 20% mais depressa.
    struct Degrau {
        float ciclo = kCiclo;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
