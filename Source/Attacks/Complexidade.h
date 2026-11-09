//
// A fase 3 do Luizinho: a complexidade dos algoritmos (Teoria da Computacao).
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// Cada operacao do algoritmo e um tiro, e o vetor de entrada, de tamanho n, e um leque de casas
/// embaixo do professor: o O(1) le uma casa, a busca binaria acha o aluno em log n leituras, a
/// busca linear le o vetor inteiro, o merge sort o le log n vezes, os dois lacos n vezes, e os
/// subconjuntos dobram a cada passo. O grafico no canto mostra a curva de cada um.
namespace Complexidade {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaComplexidade = "StateThree";
    constexpr const char* kGatilhoDaOperacao = "operacao";
    /// O O(1), v[i]: a operacao unica sai pelo seu proprio ataque, maior e mais lenta.
    constexpr const char* kGatilhoDoAcesso = "acesso";

    enum class Ordem { Constante, Logaritmica, Linear, Linearitmica, Quadratica, Exponencial };
    constexpr int kOrdens = 6;

    /// "O(1)", "O(log n)", "O(n)", "O(n log n)", "O(n²)", "O(2ⁿ)" (em UTF-8).
    std::string Notacao(Ordem o);

    /// O algoritmo de cada ordem: "acesso v[i]", "busca binária", "busca linear", "merge sort",
    /// "dois laços", "subconjuntos" (em UTF-8).
    std::string Algoritmo(Ordem o);

    /// Quantas operacoes o algoritmo faz num vetor de 'n' casas: 1, o teto de log2(n + 1), n,
    /// n vezes o teto de log2(n), n² e 2ⁿ.
    int Operacoes(Ordem o, int n);

    /// Quantas vezes o algoritmo percorre o leque inteiro: o merge sort, o teto de log2(n); os dois
    /// lacos, n; a busca linear, uma. As outras ordens nao percorrem: zero.
    int Passadas(Ordem o, int n);

    /// A cara do tiro, pelo custo: "Barata" (O(1) e O(log n)), "Media" (O(n) e O(n log n)) e
    /// "Cara" (O(n²) e O(2ⁿ)).
    std::string CaraDaOperacao(Ordem o);

    /// Os n de cada ordem: a busca binaria aguenta um vetor muito maior com muito menos operacoes -
    /// e o contraste que o grafico mostra.

    /// O professor e o processador: a cada rodada ele vai para um x sorteado entre estes dois, e o
    /// leque sai de la.
    constexpr float kMenorX = 320.0f;
    constexpr float kMaiorX = 880.0f;
    int MenorN(Ordem o);
    int MaiorN(Ordem o);

    struct Rodada {
        Ordem ordem = Ordem::Linear;
        int n = 4;
    };

    /// O peso de cada ordem nesta luta.
    struct Plano {
        float pesoDasBaratas = 0.3f;    ///< O(1) e O(log n)
        float pesoDasMedias = 0.4f;     ///< O(n) e O(n log n)
        float pesoDasCaras = 0.3f;      ///< O(n²) e O(2ⁿ)
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// A rodada 'vez' da fase: a ordem sorteada (sem repetir a anterior) e um n da faixa dela, que
    /// sobe com 'vez' - o fim da fase e mais pesado que o comeco. 'nAMais' alonga o vetor das que
    /// o percorrem (O(n), O(n log n), O(n²)): o degrau do Dificil.
    Rodada Proxima(int vez, std::uint32_t sorteio, const Plano& p = Plano{}, int anterior = -1, int nAMais = 0);

    /// "busca binária: O(log n)" e, na linha de baixo, "n = 32 → 6 operações".
    std::string Placar(const Rodada& r);

    /// O leque: as n casas do vetor, do angulo de kAbertura/2 a esquerda do centro (para baixo) ate
    /// kAbertura/2 a direita. A casa i, em graus (90 e para baixo).
    constexpr float kAbertura = 120.0f;
    float AnguloDaCasa(int i, int n);

    /// A casa do leque mais perto do angulo (em graus), entre 0 e n - 1.
    int CasaDoAngulo(float graus, int n);

    /// Onde, dentro da casa i, uma operacao sai: 'sorte', de 0 a 1, espalha o tiro pela largura da
    /// casa inteira - entre um raio e o outro do leque nao fica vao parado, nem na divisa de duas
    /// casas, bem embaixo do professor quando n e par.
    constexpr float kEspalho = 1.0f;
    float AnguloNaCasa(int i, int n, float sorte);

    /// 'graus' preso dentro da casa i: a leitura que acha o aluno mira nele sem sair da casa.
    float PresoNaCasa(float graus, int i, int n);

    /// A busca binaria: o meio do intervalo, e o que sobra dele depois de ler o meio sabendo onde o
    /// alvo esta. Vazio (lo > hi) quando o meio era o alvo.
    struct Intervalo {
        int lo = 0;
        int hi = -1;
        [[nodiscard]] bool Vazio() const { return lo > hi; }
    };
    int Meio(const Intervalo& i);
    Intervalo Descartar(const Intervalo& i, int alvo);

    /// O grafico: as operacoes de cada ordem num n qualquer (para a curva), cortadas em
    /// kTetoDoGrafico. O eixo vai de n = 1 a kNoGrafico.
    constexpr float kNoGrafico = 8.0f;
    constexpr float kTetoDoGrafico = 64.0f;
    float NaCurva(Ordem o, float n);

    /// O relogio de uma rodada: kAntes de o placar escrever ate a primeira operacao; entre duas
    /// operacoes, kEntreLeituras na busca binaria, kNaPassada numa passada pelo leque (e
    /// kEntrePassadas entre uma passada e a outra, mais curtas nos dois lacos), kEntreDobras nos
    /// subconjuntos; e kDepois de a rodada acabar.
    constexpr float kPrimeira = 1.0f;
    constexpr float kAntes = 0.5f;
    constexpr float kEntreLeituras = 0.45f;
    constexpr float kNaPassada = 0.09f;
    constexpr float kNaPassadaDosLacos = 0.06f;
    constexpr float kEntrePassadas = 0.25f;
    constexpr float kEntreDobras = 0.32f;
    constexpr float kDepois = 0.4f;

    /// Quanto a rodada dura, da primeira operacao a ultima.
    float Duracao(const Rodada& r);

    /// Os tiros: a velocidade de uma operacao, e o desvio de cada metade de uma dobra (graus).
    constexpr float kVelocidade = 300.0f;
    constexpr float kVelocidadeDoAcesso = 240.0f;
    constexpr float kDesvioDaDobra = 16.0f;

    /// O que o Dificil muda: um processador mais rapido - as operacoes saem mais seguidas ('ritmo'
    /// multiplica o intervalo entre duas) e voam mais rapido -, operacoes maiores ('tamanho'
    /// multiplica a escala), rodadas mais seguidas e vetores um pouco maiores.
    struct Degrau {
        float rapidez = 1.0f;
        float ritmo = 1.0f;
        float tamanho = 1.0f;
        float depois = kDepois;
        int nAMais = 0;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
