//
// O sorteio de cada batalha: o que muda de uma luta para a outra.
//

#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

/// O conceito e o mesmo em toda luta; o que muda e o exemplo - como uma lista de exercicios em que
/// cada aluno pega outros numeros.
namespace Variacao {

    /// Mistura dois numeros num terceiro bem espalhado (o finalizador do splitmix64).
    std::uint32_t Misturar(std::uint32_t semente, std::uint32_t rotulo);

    /// Um rotulo estavel para um nome (fnv-1a de 32 bits).
    std::uint32_t Rotulo(const std::string& nome);

    /// Uma semente nova, para uma batalha que ninguem prendeu (Simulacao.h prende). O random_device
    /// sozinho nao basta: em alguns MinGW antigos ele devolve sempre a mesma sequencia, e toda luta
    /// sairia igual. O relogio entra junto.
    std::uint32_t SementeNova();

    class Sorteio {
    public:
        explicit Sorteio(std::uint32_t semente) : mGerador(semente) {}

        /// Os 32 bits crus do mt19937.
        std::uint32_t Bruto() { return static_cast<std::uint32_t>(mGerador()); }

        /// Um inteiro em [de, ate], todos com a mesma chance (por rejeicao: o
        /// resto da divisao sozinho favorece os pequenos). Com ate < de,
        /// devolve de.
        int Inteiro(int de, int ate);

        /// Um real em [de, ate), com 24 bits de resolucao.
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

    /// Uma ordem para as formas de uma fase: 0 a n - 1 embaralhados, com as `fixas` primeiras no
    /// lugar - a que define o conceito abre a fase.
    std::vector<int> Ordem(Sorteio& s, int n, int fixas = 1);

    /// A forma da vez `vez` numa ordem de n: a volta seguinte repete a ordem, n adiante (vez n + 2
    /// e ordem[2] + n), para quem conta voltas pela vez.
    int NaOrdem(const std::vector<int>& ordem, int vez);

    /// A sequencia da fase `fase` numa batalha de semente `semente`.
    inline Sorteio SorteioDa(const std::uint32_t semente, const std::string& fase) {
        return Sorteio(Misturar(semente, Rotulo(fase)));
    }

    // ---- O disparo de cada vez ----

    /// O quanto um ataque muda de um disparo para o outro ("variar" em fases.json). Cada valor e o
    /// desvio maximo para cada lado, sorteado por igual: a media fica a do arquivo - e a dificuldade
    /// tambem.
    struct DoDisparo {
        float giro = 0.0f;        ///< graus: o leque (ou o anel) inteiro gira ate isto para cada lado
        float abertura = 0.0f;    ///< fracao do angulo do leque: 0.2 abre de 80% a 120%
        float velocidade = 0.0f;  ///< fracao da velocidade
        int projeteis = 0;        ///< ate isto a mais ou a menos, nunca abaixo de um
        float cadencia = 0.0f;    ///< fracao da espera ate o proximo disparo (so os ataques do relogio)
    };

    /// O que o sorteio deu para um disparo.
    struct Disparo {
        float giro = 0.0f;        ///< graus a somar no centro do leque
        float abertura = 1.0f;    ///< o que multiplica o angulo do leque
        float velocidade = 1.0f;  ///< o que multiplica a velocidade
        int projeteis = 0;        ///< o que somar na quantidade
        float espera = 1.0f;      ///< o que multiplica o cooldown ate o proximo disparo
    };

    /// Um disparo. Os cinco valores saem sempre, e na mesma ordem: ligar um nao muda o que os outros
    /// sorteiam.
    Disparo Sortear(Sorteio& s, const DoDisparo& v);

    /// Quantos projeteis saem com 'n' no arquivo e 'mais' do sorteio: nunca menos que um (e zero
    /// continua zero).
    int Projeteis(int n, int mais);
}
