//
// A final do Vitor: o jantar dos filosofos.
//

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A final do Vitor e o deadlock, no problema que todo curso de concorrencia conta: o jantar dos
/// filosofos.
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

    // ---- As rodadas ----

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

    /// O roteiro da rodada, em ordem de tempo. 'escolhido' e quem o professor escolhe: no deadlock,
    /// a vitima; no garcom, quem fica de fora.
    std::vector<Passo> Roteiro(Rodada r, int escolhido);

    // ---- A mesa ----

    enum class Estado { Pensando, Esperando, Comendo, Morto, DeFora };

    struct Mesa {
        std::array<int, kFilosofos> dono{};       ///< de cada garfo: o filosofo, ou -1
        std::array<Estado, kFilosofos> estado{};
        std::array<int, kFilosofos> quer{};       ///< o garfo que ele espera, ou -1
    };

    /// A mesa posta: os garfos na mesa, todos pensando.
    Mesa MesaPosta();

    /// Aplica um passo.
    bool Aplicar(Mesa& m, const Passo& p);

    /// As esperas: quem espera -> quem esta com o garfo que ele quer.
    std::vector<std::pair<int, int>> Esperas(const Mesa& m);

    /// A espera circular: seguindo as esperas a partir de alguem, volta-se a ele.
    bool EmDeadlock(const Mesa& m);

    std::vector<int> Comendo(const Mesa& m);

    // ---- O relogio ----

    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorRodada = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kRodadas);

    /// Quem come da uma garfada a cada kEntreGarfadas, a primeira kPrimeira
    /// depois de comecar a comer.
    constexpr float kPrimeiraGarfada = 0.15f;
    constexpr float kEntreGarfadas = 0.4f;

    /// No deadlock, o cerco cresce aresta a aresta - um projetil a cada
    /// kEntreEsperas - enquanto ninguem cede.
    constexpr float kEntreEsperas = 0.06f;

    // ---- O campo ----

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
    /// Impar, para o leque ter uma garfada no meio - mirada no aluno.
    constexpr int kGarfadasPorLeque = 3;
    constexpr float kVelocidadeDoEstouro = 230.0f;

    // ---- O que muda de uma luta para a outra (Variacao.h) ----

    /// O jantar desta batalha.
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

    /// O roteiro com todos andando 'giro' lugares em volta da mesa (e os garfos junto): o mesmo
    /// jantar, comecando de outro lugar.
    std::vector<Passo> Rodar(std::vector<Passo> passos, int giro);

    /// O roteiro no ritmo desta luta: cada intervalo entre dois instantes seguidos (e o do comeco ao
    /// primeiro) dura de (1 - desvio) a (1 + desvio) do de sempre, e o ultimo passo fica onde
    /// estava. A ordem nao muda: quem soltava o garfo antes de o vizinho pegar continua soltando
    /// antes - so que ninguem pensa nem come o mesmo tempo duas vezes.
    std::vector<Passo> NoRitmo(std::vector<Passo> passos, Variacao::Sorteio& s, float desvio);
    constexpr float kDesvioDoRitmo = 0.3f;

    /// Quem come nao da as garfadas no compasso: cada uma sai de (1 - kDesvioDaGarfada) a
    /// (1 + kDesvioDaGarfada) do passo do degrau depois da anterior.
    constexpr float kDesvioDaGarfada = 0.25f;

    /// O placar: o nome da rodada e quem come ("jantar · comendo.
    std::string Placar(Rodada r, const Mesa& m, const Plano& p = PlanoDeSempre());

    // ---- O degrau (Dificuldade.h) ----

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
