//
// A fase 2 do Michel: do descritor feito a mao as deep features.
//

#pragma once

#include <array>
#include <utility>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// A fase 2 do Michel e a evolucao dos descritores. O campo vira uma grade de celulas - a imagem:
///   - no gradiente, cada celula guarda uma seta (o histograma de gradientes orientados, em oito
///     direcoes), e os tiros andam pela seta da celula em que estao. As setas apontam para o aluno;
///     no meio da etapa a imagem gira, e o descritor feito a mao nao gira junto - as setas ficam de
///     lado, e o fluxo vira redemoinho;
///   - na convolucao, os TIROS DO ALUNO sao a imagem: cada celula que um tiro cruza acende, e apaga
///     devagar. Um filtro 3x3 varre a grade linha por linha, e onde a resposta passa do limiar (a
///     ReLU), a celula pisca e dispara no aluno. Atirar e preciso, mas o que se acende e o que a rede
///     le;
///   - no pooling, o maximo de cada bloco 2x2 vira uma celula grande, que dispara mais forte, e no
///     fim tudo vira um vetor so - o anel do tamanho de tudo o que se acendeu.
namespace Convolucao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateTwo";
    constexpr std::array<const char*, 3> kEtapas = {"Gradiente", "Convolucao", "Pooling"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDoFluxo = "fluxo";
    constexpr const char* kGatilhoDaAtivacao = "ativacao";
    constexpr const char* kGatilhoDoVetor = "vetor";
    constexpr const char* kGatilhoDoLeque = "leque";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A imagem ----

    constexpr int kColunas = 8;
    constexpr int kLinhas = 6;

    /// Uma grade de valores, coluna por coluna dentro de cada linha. Fora dela, tudo vale zero (o
    /// padding de zeros da convolucao).
    struct Grade {
        int colunas = 0;
        int linhas = 0;
        std::vector<float> v;
        [[nodiscard]] float Em(int c, int l) const;
        float& Celula(int c, int l);
        [[nodiscard]] bool Dentro(int c, int l) const { return c >= 0 && l >= 0 && c < colunas && l < linhas; }
    };
    Grade Vazia(int colunas, int linhas);

    /// Acende a celula (c, l): soma 'quanto', ate o 'teto'.
    void Pintar(Grade& g, int c, int l, float quanto, float teto);
    /// Tudo apaga: metade do brilho a cada 'meiaVida' segundos.
    void Esmaecer(Grade& g, float dt, float meiaVida);

    // ---- O descritor feito a mao ----

    constexpr int kBins = 8;
    /// O bin de uma direcao: 0 e a direita, e cada um gira 45 graus no sentido de y.
    int Bin(Vector2 direcao);
    Vector2 DirecaoDoBin(int bin);
    /// A seta da celula com centro 'centro': aponta para o aluno, girada de 'giro' bins - o giro da
    /// imagem que o descritor nao acompanha.
    int BinDaCelula(Vector2 centro, Vector2 aluno, int giro);
    /// Quantos bins a imagem gira no meio da etapa: 90 graus.
    constexpr int kGiroDaImagem = 2;

    // ---- A convolucao ----

    using Filtro = std::array<float, 9>;
    /// O filtro gaussiano 3x3: o meio pesa mais. Os pesos somam 1.
    constexpr Filtro kGaussiano = {1.0f / 16, 2.0f / 16, 1.0f / 16, 2.0f / 16, 4.0f / 16,
                                   2.0f / 16, 1.0f / 16, 2.0f / 16, 1.0f / 16};
    /// A resposta do filtro centrado na celula (c, l), com padding de zeros.
    float Resposta(const Grade& g, int c, int l, const Filtro& f);
    /// A ReLU com limiar: o que passa dele, e zero abaixo.
    float Relu(float x, float limiar);
    /// Onde a janela esta no passo 'passo' da varredura: linha por linha, da esquerda para a direita,
    /// e de volta ao comeco. Passo 1 (stride 1).
    std::pair<int, int> Janela(int passo, int colunas, int linhas);

    // ---- O pooling ----

    /// O max pooling 2x2, passo 2: cada bloco vira o maior valor dele. Uma sobra de borda vira um
    /// bloco menor.
    Grade MaxPool(const Grade& g);
    /// O pooling global (a media de tudo): o vetor de features em que a imagem acaba.
    float Global(const Grade& g);

    /// Quantos tiros uma ativacao 'a' dispara: de 'minimo' (quase nada) a 'maximo' (com 'cheia').
    int TirosDaAtivacao(float a, int minimo, int maximo, float cheia);

    // ---- O ritmo ----

    struct Degrau {
        float entreFluxos = 0.12f;       ///< um tiro do gradiente a cada
        float velocidadeDoFluxo = 135.0f;
        float vidaDoFluxo = 4.6f;
        float atualizacaoDoDescritor = 0.8f;
        float brilhoPorSegundo = 1.3f;   ///< o quanto um tiro do aluno acende a celula em que esta
        float meiaVida = 1.1f;
        float limiar = 0.16f;            ///< o da ReLU
        float passosPorSegundo = 8.0f;   ///< a janela da convolucao
        float passosDoPooling = 2.6f;    ///< a janela do pooling, por segundo
        float velocidadeDaAtivacao = 165.0f;
        float avisoDaAtivacao = 0.34f;   ///< a celula pisca antes de disparar
        int tirosMinimos = 4;
        int tirosMaximos = 9;
        float entreLeques = 1.15f;
        int tirosDoLeque = 5;
        int tirosDoVetor = 36;           ///< o anel do vetor, com a imagem toda acesa
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// Quando o vetor de features dispara, desde o comeco da etapa do pooling.
    constexpr float kVetor = 5.8f;
}
