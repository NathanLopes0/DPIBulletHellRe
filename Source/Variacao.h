//
// O sorteio de cada batalha: o que muda de uma luta para a outra.
//

#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

/**
 * CAMADA PURA. Coberto por test_variacao.cpp.
 *
 * TODA BATALHA TEM UMA SEMENTE (Battle::SementeDaBatalha), sorteada quando ela
 * carrega, e cada chefe tira dela o que varia DENTRO do conceito de cada fase:
 * a ordem das rodadas, os valores que o placar mostra, de onde sai cada coisa.
 * O conceito e o mesmo em toda luta; o que muda e o exemplo - como uma lista de
 * exercicios em que cada aluno pega outros numeros.
 *
 * UM SORTEADOR PROPRIO, e nao o Random do jogo, por dois motivos:
 *
 *  1. O Random e um so para tudo, e cada colisao, cada capivara e cada sorteio
 *     de ataque consome dele. Se as fases tirassem dali o roteiro, o roteiro da
 *     fase 3 dependeria de quantas vezes o aluno foi atingido na fase 1.
 *     Aqui cada fase tem a PROPRIA sequencia (SorteioDa), derivada da semente da
 *     batalha e do nome da fase: o roteiro da fase e decidido antes de ela
 *     comecar, e nada do que acontece na luta o muda.
 *
 *  2. As distribuicoes da biblioteca padrao (uniform_int_distribution,
 *     std::shuffle) sao implementadas de um jeito em cada compilador: a mesma
 *     semente daria uma luta no MSVC e outra no GCC, e um teste que conferisse
 *     o roteiro de uma semente passaria num e quebraria no outro. O mt19937 e
 *     especificado bit a bit pelo padrao; o resto - faixa, real, embaralhar -
 *     e feito aqui, com aritmetica inteira.
 */
namespace Variacao {

    /// Mistura dois numeros num terceiro bem espalhado (o finalizador do
    /// splitmix64). E o que separa a sequencia de cada fase: sementes vizinhas
    /// e rotulos parecidos dao sequencias que nao se parecem.
    std::uint32_t Misturar(std::uint32_t semente, std::uint32_t rotulo);

    /// Um rotulo estavel para um nome (FNV-1a de 32 bits): o mesmo nome da o
    /// mesmo numero em qualquer maquina, e e assim que "StateTwo" vira a
    /// sequencia da fase 2.
    std::uint32_t Rotulo(const std::string& nome);

    class Sorteio {
    public:
        explicit Sorteio(std::uint32_t semente) : mGerador(semente) {}

        /// A sequencia de um pedaco (uma rodada, um filosofo), tirada desta
        /// sem gastar dela: pedir o filho nao muda o que esta sorteia depois.
        [[nodiscard]] Sorteio Filho(std::uint32_t rotulo) const;

        /// Os 32 bits crus do mt19937.
        std::uint32_t Bruto() { return static_cast<std::uint32_t>(mGerador()); }

        /// Um inteiro em [de, ate], todos com a mesma chance (por rejeicao: o
        /// resto da divisao sozinho favorece os pequenos). Com ate < de,
        /// devolve de.
        int Inteiro(int de, int ate);

        /// Um real em [de, ate), com 24 bits de resolucao. Nunca devolve `ate`,
        /// nem quando o arredondamento do float quisesse (LWG 2524 - a
        /// armadilha 13 do RETOMAR).
        float Real(float de, float ate);

        /// Verdadeiro com chance `p` (0 nunca, 1 sempre).
        bool Chance(float p);

        /// Embaralha no lugar (Fisher-Yates, com Inteiro).
        template <typename T>
        void Embaralhar(std::vector<T>& v) {
            for (int i = static_cast<int>(v.size()) - 1; i > 0; --i) {
                const int j = Inteiro(0, i);
                if (j != i) std::swap(v[static_cast<size_t>(i)], v[static_cast<size_t>(j)]);
            }
        }

        /// Um elemento qualquer. O vetor nao pode ser vazio.
        template <typename T>
        const T& Um(const std::vector<T>& v) {
            return v[static_cast<size_t>(Inteiro(0, static_cast<int>(v.size()) - 1))];
        }

    private:
        std::mt19937 mGerador;
    };

    /**
     * @brief UMA ORDEM PARA AS FORMAS de uma fase: 0 a n - 1 embaralhados, com
     * as `fixas` primeiras no lugar - a que define o conceito abre a fase.
     *
     * E o molde dos chefes que tem um "Proximo(vez, ...)": a vez k da fase
     * passa a ser a forma NaOrdem(ordem, k).
     */
    std::vector<int> Ordem(Sorteio& s, int n, int fixas = 1);

    /// A forma da vez `vez` numa ordem de n: a volta seguinte repete a ordem,
    /// n adiante (vez n + 2 e ordem[2] + n), para quem conta voltas pela vez.
    /// Sem ordem (vazia), a vez de sempre.
    int NaOrdem(const std::vector<int>& ordem, int vez);

    /// A sequencia da fase `fase` numa batalha de semente `semente`.
    inline Sorteio SorteioDa(const std::uint32_t semente, const std::string& fase) {
        return Sorteio(Misturar(semente, Rotulo(fase)));
    }
}
