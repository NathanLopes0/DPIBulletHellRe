//
// O if, o else if e o switch da fase 1 do Andre.
//

#pragma once

#include <cstdint>
#include "Fase.h"
#include "../Dificuldade.h"
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A FASE 1 DO ANDRE E O DESVIO CONDICIONAL, e ele esta DESENHADO no campo como
 * um fluxograma: o losango da decisao embaixo do professor, e um RAMO por
 * faixa do campo, da esquerda para a direita na ordem em que aparecem no
 * codigo, cada um com o seu cabecalho no alto ("if (n > 5)", "else").
 *
 * Os dados sao baloes do BOCA, cada um com um numero n. O balao sai do
 * professor, para no losango - a condicao e avaliada ali, com o valor DELE -,
 * e segue pela seta do ramo que ela escolheu ate o cabecalho. La ele estoura, e
 * os confetes caem pela faixa daquele ramo, de cima a baixo. O aluno le o
 * numero, le a condicao e sai da faixa antes de o confete chegar.
 *
 * Um comando por vez, cada um com o seu placar:
 *
 *   if (n > 5) / else                  duas faixas
 *   if (n > 2 && n < 7) / else         duas faixas, com o E logico
 *   if (n % 2 == 0) / else             duas faixas, o par e o impar
 *   if (n < 3) / else if (n < 7) / else  tres faixas: o primeiro que vale
 *   switch (n) sem break               quatro faixas, case 0 a case 3: o balao
 *                                      estoura no case dele e CAI nos de
 *                                      baixo, um depois do outro - e um n sem
 *                                      case nao executa nada
 *   if (n = 0) / else                  a pegadinha: e uma ATRIBUICAO. O numero
 *                                      do balao VIRA 0 no losango, e o if e
 *                                      sempre falso (com "n = 7", sempre
 *                                      verdadeiro)
 *
 * Quem desenha, move e estoura e o Andre; aqui ficam os comandos, quem vai
 * para que ramo, as faixas e os textos. Coberto por test_condicional.cpp.
 */
namespace Condicional {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoSe = "StateOne";
    constexpr const char* kGatilhoDoBalao = "balao";
    constexpr const char* kGatilhoDoRotulo = "rotulo";
    constexpr const char* kGatilhoDoConfete = "confete";

    enum class Forma { Maior, Intervalo, Par, Impar, SenaoSe, Escolha, Atribuicao };

    /// Um comando: a forma e os numeros dela. Maior: "n > a". Intervalo:
    /// "n > a && n < b". SenaoSe: "n < a", "n < b". Atribuicao: "n = a".
    struct Comando {
        Forma forma = Forma::Maior;
        int a = 0;
        int b = 0;
    };
    bool operator==(const Comando& x, const Comando& y);

    /// Os numeros dos baloes vao de 0 a kMaiorValor.
    constexpr int kMaiorValor = 9;

    /// Quantos cases tem o switch: case 0 a case kCases - 1.
    constexpr int kCases = 4;

    /// @brief Quantas faixas o comando divide o campo: 2, 3 ou 4.
    int Ramos(const Comando& c);

    /// @brief O valor de n DEPOIS de a condicao ser avaliada: o mesmo, menos na
    /// atribuicao, que o troca.
    int Depois(const Comando& c, int n);

    /// @brief Os ramos que executam com o valor n, em ordem: um so no if; no
    /// switch sem break, o case de n e todos os de baixo; vazio se n nao tem
    /// case.
    std::vector<int> Executados(const Comando& c, int n);

    /// @brief O comando da vez: a forma segue a ordem da fase (a mais simples
    /// primeiro), e os numeros sao sorteados.
    Comando Proximo(int vez, std::uint32_t sorteio);

    /// @brief O numero de um balao para o comando: de 0 a kMaiorValor; no
    /// switch, quase sempre um case que tem quem caia depois dele.
    int Sortear(const Comando& c, std::uint32_t sorteio);

    /// @brief O que o placar diz: "if (n > 5) { ... } else { ... }".
    std::string Placar(const Comando& c);

    /// @brief O que o placar diz quando a regra do comando aparece: a
    /// atribuicao que trocou n, a queda pelos cases, o n sem case. Vazio se o
    /// comando nao tem nada a explicar com esse n.
    std::string Explicacao(const Comando& c, int n);

    /// @brief O cabecalho do ramo: "if (n > 5)", "else if (n < 7)", "case 2:".
    std::string Rotulo(const Comando& c, int ramo);

    /// @brief A animacao do cabecalho do ramo, e a dele aceso (quando o
    /// confete cai na faixa).
    std::string CaraDoRotulo(const Comando& c, int ramo);
    std::string CaraDoRotuloAceso(const Comando& c, int ramo);

    /// @brief Todos os comandos que Proximo pode sortear - para o teste
    /// conferir que toda cara existe nos dados.
    std::vector<Comando> Todos();

    /// @brief A animacao do balao de numero n na cor 'cor' (0 a kCores - 1).
    constexpr int kCores = 4;
    std::string CaraDoBalao(int n, int cor);

    /// @brief A animacao do confete do balao da cor 'cor': o nome da cor.
    std::string CaraDoConfete(int cor);

    /// @brief A faixa do ramo, de 'inicio' a 'fim', num campo de 'largura'.
    float InicioDaFaixa(int ramo, int ramos, float largura);
    float FimDaFaixa(int ramo, int ramos, float largura);

    /// @brief Onde cai cada confete da faixa: x de cada um, nas kFileiras
    /// fileiras, e o atraso (em pixels acima do cabecalho) de cada fileira. A
    /// primeira fileira tem um confete em cada borda da faixa.
    struct Confete { float x; float acima; };
    std::vector<Confete> Confetes(float inicio, float fim);

    // ---- O RELOGIO E O DESENHO ---------------------------------------------

    /// Um comando a cada kPorComando: seis na fase (Fase::kDuracao) - as cinco
    /// formas e, no sexto, a primeira de novo, com o E logico as vezes.
    constexpr int kComandosPorFase = 6;
    constexpr float kPorComando = Fase::kDuracao / static_cast<float>(kComandosPorFase);

    /// Os baloes de cada comando: o primeiro kPrimeiroBalao depois de ele
    /// abrir, um a cada kEntreBaloes. O ultimo estoura antes de o comando
    /// acabar (um teste confere com o pior caminho).
    constexpr int kBaloesPorComando = 3;
    constexpr float kPrimeiroBalao = 0.15f;
    constexpr float kEntreBaloes = 0.9f;

    /// O balao desce do professor ate o losango, kAbaixoDoProfessor abaixo
    /// dele, fica kNaDecisao parado la, e segue a seta do ramo ate o
    /// cabecalho: de lado ate a faixa, e depois para cima, em angulo reto.
    constexpr float kAbaixoDoProfessor = 105.0f;
    constexpr float kVelocidadeDoBalao = 950.0f;
    constexpr float kNaDecisao = 0.3f;

    /// Onde o professor fica: nesta altura, num x sorteado a cada comando.
    constexpr float kYDoProfessor = 185.0f;
    constexpr float kXMinimo = 380.0f;
    constexpr float kXMaximo = 820.0f;

    /// Os cabecalhos dos ramos: nesta altura, no meio da faixa.
    constexpr float kYDosRotulos = 96.0f;

    /// O confete: kFileiras fileiras de um a cada kEntreConfetes px, a segunda
    /// meio passo para o lado e kEntreFileiras acima. A velocidade e a do
    /// ataque em fases.json (um teste confere).
    constexpr int kFileiras = 2;
    constexpr float kEntreConfetes = 50.0f;
    constexpr float kEntreFileiras = 40.0f;
    constexpr float kVelocidadeDoConfete = 300.0f;

    /// No switch sem break, cada case de baixo estoura kCascata depois do de
    /// cima.
    constexpr float kCascata = 0.3f;

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// As formas se revezam numa ORDEM DA LUTA (Variacao::Ordem de kFormas, com
    /// o if simples abrindo): a vez k da fase e Proximo(NaOrdem(ordem, k)).
    constexpr int kFormas = 5;

    /// O que o Dificil muda: um balao a mais por comando, um atras do outro
    /// mais depressa, o balao fica menos tempo no losango - le-se a condicao
    /// com menos folga - e o confete cai mais rapido. A regra e a mesma.
    struct Degrau {
        int baloesPorComando = kBaloesPorComando;
        float entreBaloes = kEntreBaloes;
        float naDecisao = kNaDecisao;
        float rapidez = 1.0f;          ///< do confete
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
