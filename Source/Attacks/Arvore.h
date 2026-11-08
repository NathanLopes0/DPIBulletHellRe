//
// A arvore binaria de busca da fase 3 do Salles.
//

#pragma once

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

/// A fase 3 do Salles e uma arvore binaria de busca, plantada no meio do campo embaixo dele.
namespace Arvore {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaArvore = "StateThree";
    constexpr const char* kPostoDaArvore = "arvore";
    constexpr const char* kGatilhoDaChave = "no";
    constexpr const char* kGatilhoDaVisita = "visitar";
    constexpr const char* kGatilhoDoNulo = "folha_nula";
    constexpr const char* kGatilhoDoAchado = "achado";
    constexpr const char* kGatilhoDaAresta = "aresta";

    /// Quantos pontos desenham a aresta entre um no e o pai dele.
    constexpr int kPontosPorAresta = 3;

    /// A profundidade maxima: a raiz e 0, e as folhas mais fundas, 4.
    constexpr int kProfundidadeMaxima = 4;

    /// Onde fica a arvore no campo.
    constexpr float kXDaRaiz = 600.0f;
    constexpr float kYDaRaiz = 225.0f;
    constexpr float kEntreNiveis = 78.0f;
    constexpr float kLarguraDaArvore = 1040.0f;

    /// O relogio: a primeira operacao, a pausa entre duas, e o tempo em cada no
    /// do caminho.
    constexpr float kPrimeira = 1.2f;
    constexpr float kEntreOperacoes = 0.25f;
    constexpr float kEmCadaNo = 0.2f;

    /// A chance de uma operacao ser insercao, e a de a chave buscada estar na
    /// arvore.
    constexpr float kChanceDeInserir = 0.65f;
    constexpr float kChanceDeAchar = 0.5f;

    /// As chaves vao de 1 a kMaiorChave (as caras sao as da tabela hash).
    constexpr int kMaiorChave = 99;

    /// A arvore: o indice de cada no (como num heap) e a chave dele.
    using Nos = std::map<int, int>;

    /// A profundidade do indice: 0 para a raiz (1), 1 para 2 e 3...
    int Profundidade(int indice);

    /// Se o indice ainda cabe na arvore (profundidade ate a maxima).
    bool Cabe(int indice);

    /// O lugar do no de indice i no campo.
    float X(int indice);
    float Y(int indice);

    /// O caminho de uma chave a partir da raiz: os indices dos nos que ela visita, e por ultimo o
    /// lugar vazio onde ela entraria.
    std::vector<int> Caminho(const Nos& nos, int chave);

    /// Os indices da subarvore de i que estao ocupados, i incluso.
    std::vector<int> Subarvore(const Nos& nos, int indice);

    enum class Tipo { Inserir, Buscar };

    struct Operacao {
        Tipo tipo = Tipo::Inserir;
        int chave = 1;
    };

    /// A proxima operacao. Com a arvore vazia, insere; uma chave nova e sempre uma que nao esta na
    /// arvore.
    Operacao Proxima(const Nos& nos, std::uint32_t sorteio);

    std::string PlacarDaOperacao(const Operacao& op);
    std::string PlacarDaComparacao(int chave, int doNo);
    std::string PlacarDoFim(const Operacao& op, bool achou, bool cabe);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// A arvore desta luta: a chance de uma insercao (de 0,55 a 0,75 - com menos, a arvore fica
    /// baixa e ha mais busca.
    struct Plano {
        float chanceDeInserir = kChanceDeInserir;
    };
    Plano Sortear(Variacao::Sorteio& s);
    Operacao Proxima(const Nos& nos, std::uint32_t sorteio, const Plano& p);

    /// O que o Dificil muda: a chave desce mais depressa pelos nos, cada no
    /// lido atira 25% mais rapido, e o anel do achado tem doze.
    struct Degrau {
        float emCadaNo = kEmCadaNo;
        float rapidez = 1.0f;
        int anelDoAchado = 10;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
