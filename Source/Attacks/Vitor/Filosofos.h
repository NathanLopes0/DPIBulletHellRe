//
// A final do Vitor: o jantar dos filosofos.
//

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "../Engine/Fase.h"
#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FINAL DO VITOR E O DEADLOCK, no problema que todo curso de concorrencia
 * conta: o JANTAR DOS FILOSOFOS. Cinco filosofos em volta da mesa, um garfo
 * entre cada dois, e para comer cada um precisa dos dois garfos do lado dele.
 * O professor e o monitor, no meio da mesa.
 *
 * Os garfos ANDAM: quem pega um o leva para perto de si, e quem solta o
 * devolve a mesa. Quem come ATACA - garfadas em leque, miradas no aluno. Quem
 * espera um garfo tem uma linha ate quem esta com ele: a espera.
 *
 * Quatro rodadas:
 *
 *   o jantar         dois comem de cada vez, sem briga
 *   o deadlock       TODOS pegam o garfo da esquerda ao mesmo tempo, e todos
 *                    esperam o da direita: a espera circular. O pentagono das
 *                    esperas vira um cerco de projeteis, ate o professor matar
 *                    um deles - kill -9 - e o cerco estourar para fora
 *   a ordem          cada um pega primeiro o garfo de MENOR numero: o ultimo
 *                    disputa o primeiro garfo com o primeiro filosofo, a
 *                    espera vira uma fila e nao um circulo
 *   o garcom         um semaforo com quatro lugares: o professor tira um da
 *                    mesa, e com quatro nao ha circulo
 *
 * Aqui ficam os filosofos, os garfos, as rodadas (um roteiro de passos), a
 * mesa e as regras dela, a geometria e os textos; quem move os garfos, desenha
 * as esperas e ataca e o Vitor. Coberto por test_filosofos.cpp.
 */
namespace Filosofos {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoJantar = "StateFinal";
    constexpr const char* kGatilhoDoGarfo = "garfo";
    constexpr const char* kGatilhoDaGarfada = "garfada";
    constexpr const char* kGatilhoDaEspera = "espera";

    constexpr int kFilosofos = 5;

    /// O nome de cada um, em volta da mesa a partir do alto, no sentido do
    /// relogio.
    std::string Nome(int f);

    /// O garfo da esquerda do filosofo f e o f; o da direita, o seguinte. O
    /// garfo g fica entre o filosofo g - 1 e o g.
    int GarfoDaEsquerda(int f);
    int GarfoDaDireita(int f);

    // ---- AS RODADAS ---------------------------------------------------------

    enum class Rodada { Jantar, Deadlock, Ordem, Garcom };
    constexpr int kRodadas = 4;
    Rodada RodadaDaVez(int k);

    enum class Acao {
        Pega,       ///< pega o garfo
        Espera,     ///< quer o garfo, que esta com outro
        Come,       ///< tem os dois: come (e ataca)
        Solta,      ///< devolve os garfos que tem a mesa
        Morre,      ///< kill -9: solta o que tem, e sai da mesa
        Levanta     ///< o garcom o tira da mesa
    };

    struct Passo {
        float quando = 0.0f;         ///< segundos desde o comeco da rodada
        int filosofo = 0;
        Acao acao = Acao::Pega;
        int garfo = -1;              ///< Pega e Espera
    };

    /**
     * @brief O roteiro da rodada, em ordem de tempo.
     *
     * 'escolhido' e quem o professor escolhe: no deadlock, a VITIMA; no
     * garcom, quem fica DE FORA. Nas outras duas rodadas nao conta.
     */
    std::vector<Passo> Roteiro(Rodada r, int escolhido);

    // ---- A MESA -------------------------------------------------------------

    enum class Estado { Pensando, Esperando, Comendo, Morto, DeFora };

    struct Mesa {
        std::array<int, kFilosofos> dono{};       ///< de cada garfo: o filosofo, ou -1
        std::array<Estado, kFilosofos> estado{};
        std::array<int, kFilosofos> quer{};       ///< o garfo que ele espera, ou -1
    };

    /// A mesa posta: os garfos na mesa, todos pensando.
    Mesa MesaPosta();

    /**
     * @brief Aplica um passo. Devolve false - e nao mexe na mesa - se o passo
     * fere a regra: pegar um garfo que esta com outro (ou que nao e um dos
     * dois do lado dele), esperar um garfo livre, comer sem os dois, ou um
     * morto agir.
     */
    bool Aplicar(Mesa& m, const Passo& p);

    /// As esperas: quem espera -> quem esta com o garfo que ele quer.
    std::vector<std::pair<int, int>> Esperas(const Mesa& m);

    /// @brief A espera circular: seguindo as esperas a partir de alguem,
    /// volta-se a ele.
    bool EmDeadlock(const Mesa& m);

    std::vector<int> Comendo(const Mesa& m);

    // ---- O RELOGIO ----------------------------------------------------------

    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorRodada = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kRodadas);

    /// Quem come da uma garfada a cada kEntreGarfadas, a primeira kPrimeira
    /// depois de comecar a comer.
    constexpr float kPrimeiraGarfada = 0.15f;
    constexpr float kEntreGarfadas = 0.4f;

    /// No deadlock, o cerco cresce aresta a aresta - um projetil a cada
    /// kEntreEsperas - enquanto ninguem cede.
    constexpr float kEntreEsperas = 0.06f;

    // ---- O CAMPO ------------------------------------------------------------

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    /// A mesa no meio da largura, a kAlturaDaMesa do alto: os filosofos em
    /// volta, num raio, os garfos entre eles, mais perto da mesa.
    constexpr float kAlturaDaMesa = 270.0f;
    constexpr float kRaioDaMesa = 66.0f;
    constexpr float kRaioDosFilosofos = 172.0f;
    constexpr float kRaioDosGarfos = 108.0f;

    Ponto Centro(float largura);
    Ponto Lugar(int f, float largura);
    Ponto LugarDoGarfo(int g, float largura);

    /// Onde o garfo g fica na mao do filosofo f: perto dele, do lado do garfo.
    Ponto NaMao(int f, int g, float largura);

    /// O cerco de uma espera: kPorAresta pontos de um filosofo ao outro, sem
    /// as pontas (que sao os filosofos).
    constexpr int kPorAresta = 5;
    std::vector<Ponto> Cerco(int de, int para, float largura, int porAresta = kPorAresta);

    /// As velocidades: a da garfada e a do arquivo (um teste confere); a do
    /// estouro do cerco e a que o Vitor da a cada espera quando o ciclo quebra.
    constexpr float kVelocidadeDaGarfada = 210.0f;
    /// IMPAR, para o leque ter uma garfada no meio - mirada no aluno. Com
    /// quatro o meio do leque caia entre duas, e o piloto parado tomou um
    /// acerto na final inteira.
    ///
    /// E ESTREITO: tres garfadas em 16 graus. Com cinco em 40, de dois
    /// filosofos ao mesmo tempo, o leque cobria uns 300 px no chao e quem
    /// desviava era empurrado para o canto da tela e ficava la - o piloto que
    /// desvia terminava a final sozinha com 16 e quase sem atirar no chefe.
    constexpr int kGarfadasPorLeque = 3;
    constexpr float kVelocidadeDoEstouro = 230.0f;

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA (Variacao.h) ---------------------

    /**
     * @brief O jantar desta batalha.
     *
     * A HISTORIA TEM ORDEM: o jantar sem briga abre, o deadlock vem em
     * seguida, e as duas solucoes - a ordem pelo menor garfo e o garcom - vem
     * depois dele, em qualquer ordem. Quem se senta a mesa e outro a cada luta
     * (cinco de kPensadores, em lugares sorteados), e o jantar e a ordem
     * comecam de outro lugar da mesa (Rodar): quem come primeiro, e quem e o
     * ultimo na ordem dos garfos. No deadlock e no garcom quem o professor
     * escolhe ja muda sozinho - e o filosofo mais perto do aluno.
     */
    struct Plano {
        std::array<Rodada, kRodadas> rodadas{};
        std::array<std::string, kFilosofos> nomes{};
        int giro = 0;                  ///< de quantos lugares o jantar e a ordem andam
    };
    Plano PlanoDeSempre();
    Plano Sortear(Variacao::Sorteio& s);

    /// Os pensadores que podem se sentar a mesa.
    constexpr int kPensadores = 10;
    std::string Pensador(int i);

    /// @brief O roteiro com todos andando 'giro' lugares em volta da mesa (e
    /// os garfos junto): o mesmo jantar, comecando de outro lugar.
    std::vector<Passo> Rodar(std::vector<Passo> passos, int giro);

    /// O placar: o nome da rodada e quem come ("jantar · comendo: Kant, Hume"),
    /// a espera circular no deadlock ("deadlock · espera circular"), e o
    /// kill -9 da vitima - com os nomes desta luta.
    std::string Placar(Rodada r, const Mesa& m, const Plano& p = PlanoDeSempre());

    // ---- O DEGRAU (Dificuldade.h) -----------------------------------------------

    /// O que o Dificil muda: quem come da garfadas mais seguidas e mais
    /// rapidas, e o cerco do deadlock e mais cheio e estoura mais depressa.
    struct Degrau {
        float entreGarfadas = kEntreGarfadas;
        float rapidez = 1.0f;          ///< da garfada
        int porAresta = kPorAresta;
        float velocidadeDoEstouro = kVelocidadeDoEstouro;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
