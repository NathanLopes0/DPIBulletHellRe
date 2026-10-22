//
// O pipeline da fase 3 do Ricardo.
//

#pragma once

#include <array>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A fase 3 do Ricardo e Arquitetura de Computadores: um PIPELINE de cinco
 * estagios - IF (busca), ID (decodificacao), EX (execucao), MEM (memoria) e WB
 * (escrita) - atravessado no alto da tela, com as instrucoes andando um
 * estagio por ciclo. Cada uma faz o seu ataque no estagio dela: a soma no EX,
 * o load e o store no MEM.
 *
 * O PROGRAMA E DE VERDADE: o laco classico que soma os elementos de um vetor,
 * em assembly MIPS. O bne do fim do laco e PREVISTO como desvio tomado - a
 * busca continua do comeco do laco sem esperar -, e acerta em todas as voltas
 * menos na ultima: ai o desvio nao e tomado, as duas instrucoes buscadas pelo
 * caminho errado viram BOLHA (o flush), e a busca segue do sw. E a penalidade
 * de um desvio mal previsto, aparecendo como duas bolhas no pipeline.
 *
 * E O ALUNO TAMBEM FAZ BOLHA: a instrucao que ele derruba a tiro vira bolha
 * no estagio em que estava, e nao faz o ataque dela (Estourar).
 *
 * Quem faz isso no jogo e o Ricardo (as instrucoes em campo, os ataques, o
 * placar); aqui fica o pipeline, coberto por test_pipeline.cpp.
 */
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

    /// O PROGRAMA: soma os elementos de um vetor.
    ///   0  lw   $t0, 0($a0)       carrega o elemento
    ///   1  add  $t1, $t1, $t0     soma
    ///   2  addi $a0, $a0, 4       anda uma palavra (4 bytes)
    ///   3  bne  $a0, $a1, laco    volta, se nao acabou
    ///   4  sw   $t1, 0($a2)       guarda a soma
    ///   5  j    main              e recomeca
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
    };

    /// O que um ciclo fez.
    struct Ciclo {
        bool flush = false;   ///< um bne mal previsto esvaziou o IF e o ID
        int saiu = kBolha;    ///< o que saiu pelo WB
    };

    /// @brief Um ciclo: todo mundo anda um estagio, o do WB sai, o IF busca a
    /// instrucao do pc (prevendo o bne como tomado e seguindo o j), e o bne que
    /// chegou ao EX se resolve - se nao desvia, o IF e o ID viram bolha e o pc
    /// vai para depois dele.
    Ciclo Avancar(Estado& e);

    /// O aluno derrubou a instrucao do estagio: ela vira bolha ali.
    void Estourar(Estado& e, int estagio);

    /// A animacao da instrucao ("lw", "add", ...).
    const char* Animacao(Tipo t);

    /// @brief O gatilho que a instrucao puxa no estagio - a soma no EX, o load
    /// e o store no MEM -, ou vazio. O bne e o j nao atacam: o desvio ja e o
    /// que eles fazem.
    std::string Gatilho(Tipo t, int estagio);
}
