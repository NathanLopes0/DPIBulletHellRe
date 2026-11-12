//
// Os bits e as portas logicas do Ricardo.
//

#pragma once

#include <array>

#include "../Regras/Variacao.h"

#include <cstdint>
#include <optional>
#include <string>

/// O Ricardo da Sistemas Logicos, e a fase 1 dele e a materia: o professor solta.
namespace Logica {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um nos dados.
    constexpr const char* kFaseDasPortas = "StateOne";
    constexpr const char* kGatilhoDasPortas = "portas";
    constexpr const char* kPostoDaPalavra = "palavra";
    constexpr const char* kPostoDasPortas = "portas";
    constexpr const char* kAnimacaoDoZero = "Zero";
    constexpr const char* kAnimacaoDoUm = "Um";

    /// A largura da palavra: 16 bits, e a tela dividida em 16 colunas.
    constexpr int kBits = 16;

    enum class Operacao { E, Ou, OuExclusivo, Nao };

    /// Uma fileira de portas: a operacao, e a entrada B de cada coluna (um bit por coluna, como a
    /// palavra). O not nao tem B.
    struct Porta {
        Operacao operacao = Operacao::Nao;
        std::uint32_t b = 0;
    };

    /// A tabela-verdade: a saida de uma porta para as entradas a e b. O not ignora b.
    bool Saida(Operacao op, bool a, bool b);

    /// O nome da operacao como o placar o escreve: and, or, xor, not.
    const char* NomeDaOperacao(Operacao op);

    /// Le a porta de um texto de placar: o nome da operacao e, menos no not, a entrada B com
    /// exatamente 'bits' digitos 0 e 1.
    std::optional<Porta> LerPorta(const std::string& texto, int bits);

    /// O bit da coluna (0 = o mais alto, a esquerda).
    bool Bit(std::uint32_t palavra, int coluna, int bits);

    /// A palavra que sai da fileira de portas quando entra 'a'.
    std::uint32_t Aplicar(const Porta& p, std::uint32_t a, int bits);

    /// A animacao da porta da coluna: o nome da operacao e, menos no not, o B daquela coluna
    /// ("AND1", "XOR0", "NOT").
    std::string AnimacaoDaPorta(const Porta& p, int coluna, int bits);

    /// Em que coluna esta o x, numa tela de 'largura' dividida em 'bits'
    /// colunas; fora da tela, a coluna da borda.
    int Coluna(float x, float largura, int bits);

    /// O x do meio da coluna.
    float CentroDaColuna(int coluna, float largura, int bits);

    /// Quantos uns a palavra tem.
    int Uns(std::uint32_t palavra, int bits);

    /// A palavra escrita como o placar escreve, de quatro em quatro.
    std::string Texto(std::uint32_t palavra, int bits);

    /// Uma palavra justa: antes e depois da porta ha ao menos kMinimoDeZeros buracos e kMinimoDeUns
    /// bits acesos.
    constexpr int kMinimoDeZeros = 3;
    constexpr int kMinimoDeUns = 5;
    bool Justa(const Porta& p, std::uint32_t a, int bits);

    /// Uma palavra justa para a porta, a partir de um numero sorteado: a primeira justa de uma
    /// volta inteira pelas palavras que comeca nele.
    std::uint32_t Sortear(const Porta& p, int bits, std::uint32_t sorteio);

    int ColunaDoDeslocamento(float t, float segundosPorPasso, int primeira, int ultima, int inicio);

    // ---- O que muda de uma luta para a outra ----

    /// As quatro portas desta luta, uma por etapa da fase. O placar de cada etapa e Anuncio(porta),
    /// e o Ricardo continua lendo a porta dele.
    struct Plano {
        std::array<Porta, 4> portas{};
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// O placar da porta: "AND 0000 1111 1111 0000", "NOT".
    std::string Anuncio(const Porta& p);
}
