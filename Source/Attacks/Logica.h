//
// Os bits e as portas logicas do Ricardo.
//

#pragma once

#include <cstdint>
#include <optional>
#include <string>

/**
 * CAMADA PURA.
 *
 * O Ricardo da Sistemas Logicos, e a fase 1 dele e a materia: o professor solta
 * PALAVRAS DE BITS que descem pela tela, cada bit numa coluna - o UM e nivel
 * alto, aceso, e machuca; o ZERO e nivel baixo, apagado, e se atravessa. No meio
 * do caminho ha uma fileira de PORTAS LOGICAS, uma por coluna, e todo bit sai
 * dela transformado: a porta da coluna recebe o bit (A) e a entrada B escrita
 * nela, e manda adiante a saida. O aluno tem de ler a porta e ir para onde a
 * parede VAI abrir.
 *
 * O PLACAR E A PORTA. Cada etapa da fase anuncia a operacao ("AND 0000 1111 1111
 * 0000"), e e esse texto que o Ricardo le (LerPorta) para saber o que as portas
 * fazem: o que o aluno le e, literalmente, o que o codigo faz.
 *
 * COLUNA 0 E O BIT MAIS ALTO, a esquerda - a palavra se le na tela como se
 * escreve no papel.
 *
 * Quem faz isso no jogo e o Ricardo (as portas, a travessia, o sorteio da
 * palavra) e a PalavraAttack (a fileira); aqui fica a logica, coberta por
 * test_logica.cpp.
 */
namespace Logica {

    /// Os nomes que o codigo e os dados dividem: a fase das portas, o gatilho
    /// que as forma, os postos (a palavra que ainda nao passou pela porta, e as
    /// portas) e as duas caras do bit. Um teste confere cada um nos dados.
    constexpr const char* kFaseDasPortas = "StateOne";
    constexpr const char* kGatilhoDasPortas = "portas";
    constexpr const char* kPostoDaPalavra = "palavra";
    constexpr const char* kPostoDasPortas = "portas";
    constexpr const char* kAnimacaoDoZero = "Zero";
    constexpr const char* kAnimacaoDoUm = "Um";

    /// A largura da palavra: 16 bits, e a tela dividida em 16 colunas.
    constexpr int kBits = 16;

    enum class Operacao { E, Ou, OuExclusivo, Nao };

    /// Uma fileira de portas: a operacao, e a entrada B de cada coluna (um bit
    /// por coluna, como a palavra). O NOT nao tem B.
    struct Porta {
        Operacao operacao = Operacao::Nao;
        std::uint32_t b = 0;
    };

    /// A tabela-verdade: a saida de uma porta para as entradas a e b. O NOT
    /// ignora b.
    bool Saida(Operacao op, bool a, bool b);

    /// O nome da operacao como o placar o escreve: AND, OR, XOR, NOT.
    const char* NomeDaOperacao(Operacao op);

    /// @brief Le a porta de um texto de placar: o nome da operacao e, menos no
    /// NOT, a entrada B com exatamente 'bits' digitos 0 e 1 - os espacos entre
    /// eles sao so para ler. Qualquer outra coisa nao e porta.
    std::optional<Porta> LerPorta(const std::string& texto, int bits);

    /// O bit da coluna (0 = o mais alto, a esquerda).
    bool Bit(std::uint32_t palavra, int coluna, int bits);

    /// A palavra que sai da fileira de portas quando entra 'a'.
    std::uint32_t Aplicar(const Porta& p, std::uint32_t a, int bits);

    /// A animacao da porta da coluna: o nome da operacao e, menos no NOT, o B
    /// daquela coluna ("AND1", "XOR0", "NOT").
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

    /// UMA PALAVRA JUSTA: antes e depois da porta ha ao menos kMinimoDeZeros
    /// buracos e kMinimoDeUns bits acesos - e uma parede com porta, nem um muro
    /// nem um campo vazio -, e a porta muda alguma coisa nela.
    constexpr int kMinimoDeZeros = 3;
    constexpr int kMinimoDeUns = 5;
    bool Justa(const Porta& p, std::uint32_t a, int bits);

    /// @brief Uma palavra justa para a porta, a partir de um numero sorteado:
    /// a primeira justa de uma volta inteira pelas palavras que comeca nele.
    /// Se a porta nao admitir palavra justa nenhuma, o proprio sorteio.
    std::uint32_t Sortear(const Porta& p, int bits, std::uint32_t sorteio);

    /// @brief O DESLOCAMENTO: em que coluna o professor esta no instante t,
    /// andando uma coluna a cada 'segundosPorPasso', de 'inicio' para a
    /// direita ate 'ultima', dali ate 'primeira', e de volta - um bit num
    /// registrador de deslocamento, para um lado e para o outro.
    int ColunaDoDeslocamento(float t, float segundosPorPasso, int primeira, int ultima, int inicio);
}
