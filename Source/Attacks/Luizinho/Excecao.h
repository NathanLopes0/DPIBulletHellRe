//
// Os sequenciadores da final do Luizinho: as excecoes.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../Engine/Fase.h"
#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FINAL DO LUIZINHO SAO OS SEQUENCIADORES - o que desvia o fluxo para fora
 * da ordem do texto -, e o maior deles e a EXCECAO. A repescagem e, ela mesma,
 * um catch: quem nao passou cai nela.
 *
 * O campo e a PILHA DE CHAMADAS: cada chamada empilha uma faixa no alto, uma
 * embaixo da outra - main, f, g, h -, com o que ela faz com uma excecao
 * escrito no canto ("catch (IOException e)", "finally"). O professor e o
 * ponto de execucao: desce de faixa em faixa conforme as chamadas, e na mais
 * funda lanca a excecao. Ela SOBE, faixa por faixa, procurando quem a trate:
 *
 *   - a faixa que nao trata e DESEMPILHADA: se quebra, e os destrocos caem
 *     numa cortina sobre o aluno;
 *   - o finally roda de qualquer jeito: um anel, e a faixa se quebra tambem;
 *   - o catch do tipo dela - ou de um tipo ACIMA dela na hierarquia: uma
 *     NullPointerException e uma RuntimeException - a captura ali, e para;
 *   - se nem o main trata, o programa capota: "Exception in thread main".
 *
 * Quanto mais longe a excecao vai, mais faixas caem. Cinco rodadas, a ultima
 * com uma excecao do proprio programa: catch (Reprovado e) { repescagem(); }.
 *
 * Aqui ficam a hierarquia, as rodadas, a propagacao, as faixas, a cortina e
 * os textos; quem empilha, quebra e solta e o Luizinho. Coberto por
 * test_excecao.cpp.
 */
namespace Excecao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDasExcecoes = "StateFinal";
    constexpr const char* kGatilhoDaExcecao = "excecao";
    constexpr const char* kGatilhoDoDestroco = "destroco";
    constexpr const char* kGatilhoDoFinally = "finally";

    // ---- A HIERARQUIA -------------------------------------------------------

    enum class Tipo { Exception, IOException, RuntimeException, NullPointerException, Reprovado };
    constexpr int kTipos = 5;

    /// @brief A superclasse de t; a de Exception e ela mesma.
    Tipo Pai(Tipo t);

    /// @brief Se t E UM 'base': ele mesmo, ou uma subclasse dele.
    bool EhUm(Tipo t, Tipo base);

    std::string Nome(Tipo t);

    // ---- AS RODADAS ---------------------------------------------------------

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

    /// @brief A propagacao: a excecao deixa quem lancou (que e desempilhado),
    /// chega a faixa de cima a cada kPorQuadro, roda o finally de quem tem,
    /// desempilha quem nao trata, e para na primeira que trata - ou escapa do
    /// main.
    std::vector<Passo> Propagar(const Rodada& r);

    /// @brief O indice de quem captura, ou -1 se ninguem.
    int QuemCaptura(const Rodada& r);

    // ---- OS TEXTOS ----------------------------------------------------------

    /// @brief O canto da faixa: "f() - catch (IOException e)", "g() - finally"
    /// (com o ponto do meio no lugar do traco).
    std::string Rotulo(const Quadro& q);

    /// @brief O placar: as chamadas ("main() -> f() -> g() -> h()"), o
    /// lancamento e o desfecho.
    std::string PlacarDasChamadas(const Rodada& r);
    std::string PlacarDoLancamento(const Rodada& r);
    std::string PlacarDoDesfecho(const Rodada& r);

    /// As caras: a excecao com o nome do tipo, o destroco e o anel do finally.
    std::string CaraDaExcecao(Tipo t);
    constexpr const char* kCaraDoDestroco = "Destroco";
    constexpr const char* kCaraDoFinally = "Finally";

    // ---- O CAMPO ------------------------------------------------------------

    struct Retangulo { float x = 0.0f; float y = 0.0f; float w = 0.0f; float h = 0.0f; };

    /// @brief A faixa da chamada de profundidade k (0 e o main), num campo de
    /// 'largura': de borda a borda (menos kMargem), uma embaixo da outra a
    /// partir de kTopoDaPilha.
    Retangulo Faixa(int k, float largura);
    /// ABAIXO DO PLACAR, que fica no canto de cima a esquerda, com folga: e a
    /// altura com que a fase foi medida.
    constexpr float kTopoDaPilha = 140.0f;

    constexpr float kAlturaDaFaixa = 60.0f;
    constexpr float kEntreFaixas = 4.0f;
    constexpr float kMargem = 20.0f;
    constexpr int kMaiorPilha = 4;

    /// @brief A CORTINA de uma faixa que cai: um destroco a cada 'entre' (o
    /// degrau do Dificil encurta) ao longo da faixa, um deles em cima do aluno -
    /// parado, ele nao fica numa fresta.
    constexpr float kEntreDestrocos = 60.0f;
    std::vector<float> Cortina(float xDoAluno, float largura, float entre = kEntreDestrocos);

    /// Onde o professor fica na faixa: no meio da altura dela, num x sorteado
    /// a cada rodada entre kXMinimo e kXMaximo (o canto da esquerda e do
    /// rotulo). A excecao sobe kAEsquerda a esquerda dele, para o nome dela
    /// nao ficar atras do cartao.
    constexpr float kXMinimo = 600.0f;
    constexpr float kXMaximo = 1000.0f;
    constexpr float kAEsquerda = 150.0f;

    // ---- O RELOGIO ----------------------------------------------------------

    /// Uma rodada a cada kPorRodada, a primeira kPrimeiro depois de a fase
    /// abrir (antes dela o placar diz o latim). As chamadas empilham uma a
    /// cada kEntreChamadas; a excecao e lancada kLancamento depois de a
    /// rodada abrir e sobe uma faixa a cada kPorQuadro. Quem sobrou na pilha
    /// retorna kRetorno depois do desfecho.
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

    // ---- O DEGRAU -------------------------------------------------------------

    /// O que o Dificil muda: a cortina tem um destroco a cada 52 px (e nao 60)
    /// - a fresta entre dois fica mais estreita -, e os destrocos e o anel do
    /// finally saem 20% mais rapido.
    struct Degrau {
        float entreDestrocos = kEntreDestrocos;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
    // ---- AS RODADAS GERADAS -------------------------------------------------

    /**
     * @brief UMA RODADA GERADA: a pilha tem de 3 a kMaiorPilha chamadas, do
     * main ate quem lanca; cada uma acima de quem lanca tem, por sorteio, um
     * catch de um tipo qualquer da hierarquia e um finally; e a excecao
     * lancada e uma das tres concretas. Quem captura - pelo tipo exato, pela
     * hierarquia, ou ninguem - sai da propagacao. Antes as rodadas eram cinco,
     * escritas a mao, so em ordem sorteada: decoravam-se.
     */
    Rodada Gerar(Variacao::Sorteio& s);

    /// @brief A rodada k da luta: gerada, e a ultima de cada kRodadas e sempre
    /// o catch (Reprovado e) - a repescagem e um catch.
    Rodada RodadaDaLuta(Variacao::Sorteio& s, int k);
}
