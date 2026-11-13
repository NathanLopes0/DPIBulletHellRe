//
// A fase 3 do Arroyo: os limites inferiores e a NP-completude.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// Duas etapas, o que nao se faz mais rapido e o que talvez nao se faca rapido nunca:
///   - Limite inferior: a rede de ordenacao. Seis fios descem o campo, os valores caem por eles e,
///     em cada comparador, os dois que estao fora de ordem trocam de fio - na diagonal, cruzando o
///     caminho do aluno. Embaixo saem ordenados. Ordenar por comparacao pede ao menos log2(n!)
///     comparacoes, e a rede usa 12 para 6 (o minimo e 10).
///   - NP-completo: o caixeiro viajante. A cada rodada ha mais cidades e o numero de rotas explode -
///     12, 60, 360 -; o caixeiro tenta rota atras de rota, a melhor fica desenhada, e cada cidade em
///     que ele passa atira.
namespace Intratavel {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateThree";
    constexpr std::array<const char*, 2> kEtapas = {"LimiteInferior", "Caixeiro"};
    constexpr const char* kGatilhoDoValor = "valor";
    constexpr const char* kGatilhoDaCidade = "cidade";
    constexpr const char* kGatilhoDoCaixeiro = "caixeiro";
    constexpr const char* kGatilhoDaVisita = "visita";
    constexpr const char* kGatilhoDaRotaNova = "rota";

    /// A rede ocupa os primeiros kFimDaRede segundos da fase; o caixeiro, o resto.
    constexpr float kFimDaRede = 8.0f;

    // ---- Limite inferior: a rede de ordenacao ----

    constexpr int kFios = 6;

    /// Os comparadores, camada por camada: em cada par (i, j), com i < j, o menor fica no fio i.
    const std::vector<std::vector<std::pair<int, int>>>& Camadas();

    /// Quantos comparadores a rede tem (12).
    int Comparadores();

    /// O menor numero de comparacoes que ordena n valores no pior caso, por qualquer algoritmo de
    /// comparacao: o teto de log2(n!). Para 6, 10.
    int LimiteInferior(int n);

    /// Os valores depois de passarem pela rede. Ordena qualquer permutacao de 6 (um teste confere as
    /// 720).
    std::vector<int> Ordenar(std::vector<int> valores);

    /// O x do fio 'i', com os fios espalhados em 'largura' a partir de 'esquerda'.
    float XDoFio(int i, float esquerda, float largura);

    /// O y da camada 'c' de comparadores, entre 'topo' e 'fundo'.
    float YDaCamada(int c, float topo, float fundo);

    /// O placar da rede: "Ω(n log n): log₂ 6! ≈ 9,5 → 10 · a rede usa 12".
    std::string PlacarDaRede();

    constexpr float kEntreOndas = 1.45f;      ///< segundos entre duas levas de valores
    constexpr float kTroca = 0.32f;           ///< segundos que um valor leva para trocar de fio

    // ---- NP-completo: o caixeiro viajante ----

    /// As rodadas: quantas cidades em cada uma.
    constexpr std::array<int, 3> kCidades = {5, 6, 7};
    constexpr float kDuracaoDaRodada = (Fase::kDuracao - kFimDaRede) / static_cast<float>(kCidades.size());

    /// Quantas rotas diferentes ha por 'n' cidades: (n - 1)! / 2.
    long long Rotas(int n);

    /// 'n' cidades sorteadas no retangulo (x, y, largura, altura), longe umas das outras.
    std::vector<Vector2> Cidades(Variacao::Sorteio& s, int n, float x, float y, float largura, float altura);

    /// O comprimento da rota fechada que visita as cidades na ordem 'rota'.
    float Comprimento(const std::vector<Vector2>& cidades, const std::vector<int>& rota);

    /// Uma rota qualquer: a cidade 0 primeiro, as outras embaralhadas.
    std::vector<int> Tentativa(Variacao::Sorteio& s, int n);

    /// O placar da rodada: "n = 6 · 60 rotas · tentativa 3 · melhor 1840".
    std::string PlacarDoCaixeiro(int cidades, int tentativa, float melhor);

    /// Onde o caixeiro esta 'u' (0 a 1) da volta inteira, andando na mesma rapidez em todo trecho.
    Vector2 NaRota(const std::vector<Vector2>& cidades, const std::vector<int>& rota, float u);

    /// O que o Dificil muda.
    struct Degrau {
        float porTentativa = 0.95f;   ///< segundos de uma volta do caixeiro
        float queda = 150.0f;         ///< px/s dos valores na rede
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
