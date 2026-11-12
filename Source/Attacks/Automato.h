//
// O automato finito da fase 1 do Vladimir.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>

/// A fase 1 do Vladimir e um automato finito deterministico, desenhado no campo: tres estados, e o
/// professor e a cabeca que anda neles.
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

    /// Onde fica o automato, no campo de 1200 x 800.
    constexpr std::array<float, kEstados> kXDosEstados = {220.0f, 600.0f, 980.0f};
    constexpr std::array<float, kEstados> kYDosEstados = {170.0f, 260.0f, 170.0f};
    constexpr float kXDoDiagrama = 600.0f;
    constexpr float kYDoDiagrama = 230.0f;

    /// O tamanho das palavras sorteadas.
    constexpr int kMenorPalavra = 4;
    constexpr int kMaiorPalavra = 6;

    /// A transicao: o estado para onde 'q' vai lendo 'letra'. Uma letra fora do alfabeto, ou um
    /// estado que nao existe, deixa onde esta.
    int Proximo(int q, char letra);

    /// Se o estado aceita.
    bool Aceita(int q);

    /// O estado em que o automato termina lendo 'palavra' a partir de q0.
    int Rodar(const std::string& palavra);

    /// Uma palavra de {a, b}, de kMenorPalavra a kMaiorPalavra letras, tirada do 'sorteio'. A mesma
    /// semente da a mesma palavra.
    std::string Sortear(std::uint32_t sorteio);

    struct Leitura {
        std::string palavra;
        std::size_t lidos = 0;
        int estado = kInicial;
    };

    /// O que falta ler.
    std::string Resto(const Leitura& l);

    /// A palavra acabou?
    bool Terminou(const Leitura& l);

    /// Um passo da leitura: o simbolo lido e a transicao feita.
    struct Passo {
        char letra = 0;
        int de = kInicial;
        int para = kInicial;
        /// A transicao volta para o mesmo estado - o laco do diagrama.
        [[nodiscard]] bool Laco() const { return de == para; }
    };

    /// Le o proximo simbolo. Com a palavra no fim, nao anda: o passo devolvido tem letra 0 e fica
    /// no estado.
    Passo Ler(Leitura& l);

    void Trocar(Leitura& l);

    /// "q0", "q1"... com o indice em subscrito, em UTF-8: "q₁".
    std::string NomeDoEstado(int q);

    /// O placar durante a leitura: a configuracao (q, w), com a palavra vazia escrita epsilon.
    /// "(q₁, bab)".
    std::string Configuracao(const Leitura& l);

    /// O placar depois da palavra: a configuracao final e o veredito. "(q₂, ε) aceita" ou "(q₀, ε)
    /// rejeita".
    std::string Veredito(const Leitura& l);
}
