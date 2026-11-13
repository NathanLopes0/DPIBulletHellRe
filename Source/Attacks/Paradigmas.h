//
// A fase 2 do Arroyo: os paradigmas de projeto - guloso, programacao dinamica e backtracking.
//

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "Fase.h"

/// Tres etapas, tres jeitos de projetar um algoritmo, cada um com o problema de sala dele:
///   - Guloso: o troco. As moedas saem da maior para a menor, e cada uma persegue o aluno - escolher
///     o melhor agora. Nos sistemas de moedas ruins o guloso erra, e erra com moeda a mais.
///   - Programacao dinamica: a tabela da maior subsequencia comum, preenchida em diagonal - a frente
///     de onda das dependencias. Cada casa calculada chove; a que casa letra com letra chove em leque.
///   - Backtracking: as seis rainhas, colocadas linha a linha e tiradas quando nao ha saida. A rainha
///     colocada atira pela coluna; achada a solucao, as seis atacam nas oito direcoes.
namespace Paradigmas {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateTwo";
    constexpr std::array<const char*, 3> kEtapas = {"Guloso", "Dinamica", "Backtracking"};
    constexpr const char* kGatilhoDaMoeda = "moeda";
    constexpr const char* kGatilhoDaCelula = "celula";
    constexpr const char* kGatilhoDaChuva = "chuva";
    constexpr const char* kGatilhoDoLeque = "casou";
    constexpr const char* kGatilhoDaRainha = "rainha";
    constexpr const char* kGatilhoDaColuna = "ataque";
    constexpr const char* kGatilhoDaSolucao = "solucao";

    /// Onde cada etapa comeca, em segundos desde o comeco da fase.
    constexpr std::array<float, 3> kInicio = {0.0f, 6.5f, 13.5f};
    constexpr std::array<float, 3> kDuracao = {6.5f, 7.0f, 6.5f};

    /// A etapa no instante 't' da fase (0, 1 ou 2).
    int EtapaNoTempo(float t);

    // ---- Guloso: o troco ----

    struct Troco {
        int valor = 0;
        std::vector<int> moedas;   ///< o sistema, da maior para a menor
    };

    /// Os tres trocos da etapa: um sistema bom (o guloso acerta) e dois ruins (ele erra).
    const std::vector<Troco>& Trocos();

    /// As moedas que o guloso da: a maior que cabe, de novo e de novo.
    std::vector<int> Guloso(const Troco& t);

    /// O menor numero de moedas possivel - a programacao dinamica, que o guloso nao fez.
    int Otimo(const Troco& t);

    /// O placar: "Troco de 30: 25+1+1+1+1+1 (ótimo: 3 moedas)".
    std::string PlacarDoTroco(const Troco& t);

    /// A cara da moeda em projeteis.json: "M100", "M25", "M4"...
    std::string CaraDaMoeda(int valor);

    /// O tamanho da moeda (a escala do projetil): a maior e a maior.
    float EscalaDaMoeda(int valor);

    constexpr float kEntreMoedas = 0.22f;    ///< segundos entre duas moedas do mesmo troco
    constexpr float kDuracaoDoTroco = 2.15f; ///< cada troco, na etapa de 6,5 s

    // ---- Programacao dinamica: a tabela ----

    /// As duas palavras: a piada de sala (algoritmo e logaritmo sao anagramas quase).
    constexpr const char* kPalavraA = "ALGORITMO";
    constexpr const char* kPalavraB = "LOGARITMO";

    /// A tabela da maior subsequencia comum de a e b: (|a| + 1) x (|b| + 1), a linha e a coluna 0
    /// valendo zero.
    std::vector<std::vector<int>> Tabela(const std::string& a, const std::string& b);

    /// A casa (i, j) (de 1 em diante) veio da diagonal - a letra i de a e a letra j de b casam?
    bool Casou(const std::string& a, const std::string& b, int i, int j);

    /// As casas de 1 a 'linhas' x 1 a 'colunas', em ondas: a onda d tem as casas com i + j = d + 2 -
    /// todas as dependencias de uma casa estao em ondas anteriores.
    std::vector<std::vector<std::pair<int, int>>> Ondas(int linhas, int colunas);

    /// O lugar da casa (i, j) na tela, com a tabela centrada em 'centroX' e o topo em 'topo'.
    Vector2 LugarDaCasa(int i, int j, int linhas, int colunas, float centroX, float topo);
    constexpr float kLarguraDaCasa = 34.0f;
    constexpr float kAlturaDaCasa = 26.0f;

    /// A cara da casa: "V0".."V9" (calculada) ou "C1".."C9" (casou letra com letra).
    std::string CaraDaCasa(int valor, bool casou);

    // ---- Backtracking: as rainhas ----

    constexpr int kRainhas = 6;

    /// Um passo da busca: a rainha da linha 'linha' entra (ou sai) na coluna 'coluna'.
    struct Passo {
        bool coloca = true;
        int linha = 0;
        int coluna = 0;
    };

    /// Os passos da busca em profundidade de 'n' rainhas, ate a primeira solucao. Para 6: 31
    /// colocacoes e 25 retiradas.
    std::vector<Passo> Busca(int n);

    /// As rainhas de pe depois de 'passos' passos: a coluna de cada linha, ou -1.
    std::vector<int> NoTabuleiro(const std::vector<Passo>& busca, int passos, int n);

    /// O lugar da casa (linha, coluna) do tabuleiro de 'n', centrado em 'centro'.
    Vector2 LugarNoTabuleiro(int linha, int coluna, int n, const Vector2& centro);
    constexpr float kCasaDoTabuleiro = 56.0f;

    /// As oito direcoes de ataque de uma rainha, em graus (0 e para a direita, 90 para baixo).
    std::array<float, 8> DirecoesDaRainha();

    /// O que o Dificil muda.
    struct Degrau {
        float porPasso = 0.105f;      ///< segundos entre dois passos da busca
        float porOnda = 0.36f;        ///< segundos entre duas ondas da tabela
        float velocidadeDaMoeda = 230.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
