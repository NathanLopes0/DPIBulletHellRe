//
// Os sequenciadores da final do Luizinho: as excecoes.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A final do Luizinho sao os sequenciadores - o que desvia o fluxo para fora da ordem do texto -,
/// e o maior deles e a excecao.
namespace Excecao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDasExcecoes = "StateFinal";
    constexpr const char* kGatilhoDaExcecao = "excecao";
    constexpr const char* kGatilhoDoDestroco = "destroco";
    constexpr const char* kGatilhoDoFinally = "finally";

    // ---- A hierarquia ----

    enum class Tipo { Exception, IOException, RuntimeException, NullPointerException, Reprovado };
    constexpr int kTipos = 5;

    /// A superclasse de t; a de Exception e ela mesma.
    Tipo Pai(Tipo t);

    /// Se t e um 'base': ele mesmo, ou uma subclasse dele.
    bool EhUm(Tipo t, Tipo base);

    std::string Nome(Tipo t);

    // ---- As rodadas ----

    /// Uma chamada na pilha: a funcao, o tipo que o catch dela trata (se tem
    /// catch) e se ela tem finally.
    struct Quadro {
        std::string funcao;
        std::optional<Tipo> captura;
        bool finally = false;
    };

    /// Uma rodada: a pilha, do main (o primeiro) ate quem lanca (o ultimo), e
    /// o tipo lancado.
    struct Rodada {
        std::vector<Quadro> pilha;
        Tipo lancada = Tipo::Exception;
    };

    constexpr int kRodadas = 5;
    Rodada RodadaDaVez(int k);

    /// O que acontece em cada faixa, de quem lanca para cima.
    enum class Acontece { Desempilha, Finally, Captura, Escapa };
    struct Passo {
        int quadro = 0;              ///< o indice na pilha; -1 no Escapa
        Acontece o = Acontece::Desempilha;
        float quando = 0.0f;         ///< segundos desde o lancamento
    };

    std::vector<Passo> Propagar(const Rodada& r);

    /// O indice de quem captura, ou -1 se ninguem.
    int QuemCaptura(const Rodada& r);

    // ---- Os textos ----

    std::string Rotulo(const Quadro& q);

    /// O placar: as chamadas ("main() -> f() -> g() -> h()"), o lancamento e o desfecho.
    std::string PlacarDasChamadas(const Rodada& r);
    std::string PlacarDoLancamento(const Rodada& r);
    std::string PlacarDoDesfecho(const Rodada& r);

    /// As caras: a excecao com o nome do tipo, o destroco e o anel do finally.
    std::string CaraDaExcecao(Tipo t);
    constexpr const char* kCaraDoDestroco = "Destroco";
    constexpr const char* kCaraDoFinally = "Finally";

    // ---- O campo ----

    struct Retangulo { float x = 0.0f; float y = 0.0f; float w = 0.0f; float h = 0.0f; };

    /// A faixa da chamada de profundidade k (0 e o main), num campo de 'largura'.
    Retangulo Faixa(int k, float largura);
    constexpr float kTopoDaPilha = 140.0f;

    constexpr float kAlturaDaFaixa = 60.0f;
    constexpr float kEntreFaixas = 4.0f;
    constexpr float kMargem = 20.0f;
    constexpr int kMaiorPilha = 4;

    /// A cortina de uma faixa que cai: um destroco a cada 'entre' (o degrau do Dificil encurta) ao
    /// longo da faixa, um deles em cima do aluno.
    constexpr float kEntreDestrocos = 60.0f;
    std::vector<float> Cortina(float xDoAluno, float largura, float entre = kEntreDestrocos);

    /// Onde o professor fica na faixa: no meio da altura dela, num x sorteado a cada rodada entre
    /// kXMinimo e kXMaximo (o canto da esquerda e do rotulo).
    constexpr float kXMinimo = 600.0f;
    constexpr float kXMaximo = 1000.0f;
    constexpr float kAEsquerda = 150.0f;

    // ---- O relogio ----

    /// Uma rodada a cada kPorRodada, a primeira kPrimeiro depois de a fase abrir (antes dela o
    /// placar diz o latim).
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorRodada = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kRodadas);
    constexpr float kEntreChamadas = 0.2f;
    constexpr float kLancamento = 1.0f;
    constexpr float kPorQuadro = 0.4f;
    constexpr float kRetorno = 0.8f;

    /// As velocidades sao as dos ataques em fases.json (um teste confere).
    constexpr float kVelocidadeDoDestroco = 280.0f;
    constexpr float kVelocidadeDoFinally = 220.0f;
    constexpr int kAnelDoFinally = 12;

    // ---- O degrau ----

    /// O que o Dificil muda: a cortina tem um destroco a cada 52 px (e nao 60).
    struct Degrau {
        float entreDestrocos = kEntreDestrocos;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
    // ---- As rodadas geradas ----

    /// Uma rodada gerada: a pilha tem de 3 a kMaiorPilha chamadas, do main ate quem lanca.
    Rodada Gerar(Variacao::Sorteio& s);

    /// A rodada k da luta: gerada, e a ultima de cada kRodadas e sempre o catch (Reprovado e) - a
    /// repescagem e um catch.
    Rodada RodadaDaLuta(Variacao::Sorteio& s, int k);
}
