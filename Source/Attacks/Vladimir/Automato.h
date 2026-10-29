//
// O automato finito da fase 1 do Vladimir.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>

/**
 * CAMADA PURA.
 *
 * A FASE 1 DO VLADIMIR E UM AUTOMATO FINITO DETERMINISTICO, desenhado no campo:
 * tres estados, e o professor e a cabeca que anda neles. Ele le uma palavra do
 * alfabeto {a, b}, um simbolo por passo, e a cada simbolo faz a transicao -
 * voa do estado em que esta para o estado que a tabela manda, e atira a letra
 * que leu. Lida a palavra, ele ACEITA ou REJEITA.
 *
 * A LINGUAGEM E "termina em ab", e o automato e o classico de tres estados:
 *
 *              a              b
 *     q0  ->  q1     q0  ->  q0        q0: nada de util lido
 *     q1  ->  q1     q1  ->  q2        q1: acabou de ler um "a"
 *     q2  ->  q1     q2  ->  q0        q2: acabou de ler "ab"  (aceita)
 *
 * O PLACAR E A CONFIGURACAO DO AUTOMATO: (q1, bab) - o estado e o que falta
 * ler. E a notacao da disciplina, e e o que deixa o aluno PREVER: lendo o
 * placar e a tabela ele sabe para onde o professor vai e que letra vem.
 *
 * Quem desenha os estados, voa e atira e o Vladimir; aqui ficam a tabela, o
 * sorteio da palavra, a leitura e o texto do placar. Coberto por
 * test_automato.cpp.
 */
namespace Automato {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoAutomato = "StateOne";
    constexpr const char* kPostoDosEstados = "estados";
    constexpr const char* kGatilhoDosEstados = "estados";
    constexpr const char* kGatilhoDoDiagrama = "diagrama";
    constexpr const char* kCaraDoDiagrama = "Diagrama";
    constexpr const char* kGatilhoDoA = "ler_a";
    constexpr const char* kGatilhoDoB = "ler_b";
    constexpr const char* kGatilhoDeAceitar = "aceita";
    constexpr const char* kGatilhoDeRejeitar = "rejeita";

    /// Quantos estados, e qual aceita.
    constexpr int kEstados = 3;
    constexpr int kInicial = 0;
    constexpr int kAceitacao = 2;

    /// ONDE FICA O AUTOMATO, no campo de 1200 x 800. Tem de bater com o desenho:
    /// o diagrama e uma figura so (Ferramentas/gerar_vladimir.py, CENTROS e
    /// DIAGRAMA_X0/Y0/W/H), e as setas dela apontam para estes centros. Ha
    /// teste que le o gerador e confere.
    constexpr std::array<float, kEstados> kXDosEstados = {220.0f, 600.0f, 980.0f};
    constexpr std::array<float, kEstados> kYDosEstados = {170.0f, 260.0f, 170.0f};
    constexpr float kXDoDiagrama = 600.0f;
    constexpr float kYDoDiagrama = 230.0f;

    /// O tamanho das palavras sorteadas.
    constexpr int kMenorPalavra = 4;
    constexpr int kMaiorPalavra = 6;

    /// @brief A transicao: o estado para onde 'q' vai lendo 'letra'. Uma letra
    /// fora do alfabeto, ou um estado que nao existe, deixa onde esta.
    int Proximo(int q, char letra);

    /// @brief Se o estado aceita.
    bool Aceita(int q);

    /// @brief O estado em que o automato termina lendo 'palavra' a partir de q0.
    int Rodar(const std::string& palavra);

    /// @brief Uma palavra de {a, b}, de kMenorPalavra a kMaiorPalavra letras,
    /// tirada do 'sorteio'. A mesma semente da a mesma palavra.
    std::string Sortear(std::uint32_t sorteio);

    /// @brief A leitura em curso: a palavra, quanto ja foi lido, e onde esta.
    struct Leitura {
        std::string palavra;
        std::size_t lidos = 0;
        int estado = kInicial;
    };

    /// @brief O que falta ler.
    std::string Resto(const Leitura& l);

    /// @brief A palavra acabou?
    bool Terminou(const Leitura& l);

    /// @brief Um passo da leitura: o simbolo lido e a transicao feita.
    struct Passo {
        char letra = 0;
        int de = kInicial;
        int para = kInicial;
        /// A transicao volta para o mesmo estado - o laco do diagrama.
        [[nodiscard]] bool Laco() const { return de == para; }
    };

    /// @brief Le o proximo simbolo. Com a palavra no fim, nao anda: o passo
    /// devolvido tem letra 0 e fica no estado.
    Passo Ler(Leitura& l);

    /// @brief O que falta ler passa pelo intercambio (Alfabeto::TrocarPalavra):
    /// o que ja foi lido fica como foi.
    void Trocar(Leitura& l);

    /// @brief "q0", "q1"... com o indice em subscrito, em UTF-8: "q₁".
    std::string NomeDoEstado(int q);

    /// @brief O placar durante a leitura: a configuracao (q, w), com a palavra
    /// vazia escrita epsilon. "(q₁, bab)".
    std::string Configuracao(const Leitura& l);

    /// @brief O placar depois da palavra: a configuracao final e o veredito.
    /// "(q₂, ε) aceita" ou "(q₀, ε) rejeita".
    std::string Veredito(const Leitura& l);
}
