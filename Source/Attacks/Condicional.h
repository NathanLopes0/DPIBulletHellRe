//
// O if, o else if e o switch da fase 1 do Andre.
//

#pragma once

#include <cstdint>
#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include <string>
#include <vector>

/// A fase 1 do Andre e o desvio condicional, e ele esta desenhado no campo como um fluxograma. Os
/// dados sao baloes do boca, cada um com um numero n.
namespace Condicional {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoSe = "StateOne";
    constexpr const char* kGatilhoDoBalao = "balao";
    constexpr const char* kGatilhoDoRotulo = "rotulo";
    constexpr const char* kGatilhoDoConfete = "confete";

    enum class Forma { Maior, Intervalo, Par, Impar, SenaoSe, Escolha, Atribuicao };

    /// Um comando: a forma e os numeros dela. Maior: "n > a".
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

    /// Quantas faixas o comando divide o campo: 2, 3 ou 4.
    int Ramos(const Comando& c);

    /// O valor de n depois de a condicao ser avaliada: o mesmo, menos na atribuicao, que o troca.
    int Depois(const Comando& c, int n);

    /// Os ramos que executam com o valor n, em ordem: um so no if; no switch sem break, o case de n
    /// e todos os de baixo; vazio se n nao tem case.
    std::vector<int> Executados(const Comando& c, int n);

    /// O comando da vez: a forma segue a ordem da fase (a mais simples primeiro), e os numeros sao
    /// sorteados.
    Comando Proximo(int vez, std::uint32_t sorteio);

    /// O numero de um balao para o comando: de 0 a kMaiorValor; no switch, quase sempre um case que
    /// tem quem caia depois dele.
    int Sortear(const Comando& c, std::uint32_t sorteio);

    /// O que o placar diz: "if (n > 5) { ... } else { ... }".
    std::string Placar(const Comando& c);

    /// O que o placar diz quando a regra do comando aparece: a atribuicao que trocou n, a queda
    /// pelos cases, o n sem case.
    std::string Explicacao(const Comando& c, int n);

    /// A animacao do cabecalho do ramo, e a dele aceso (quando o confete cai na faixa).
    std::string CaraDoRotulo(const Comando& c, int ramo);
    std::string CaraDoRotuloAceso(const Comando& c, int ramo);

    /// Todos os comandos que Proximo pode sortear - para o teste conferir que toda cara existe nos
    /// dados.
    std::vector<Comando> Todos();

    /// A animacao do balao de numero n na cor 'cor' (0 a kCores - 1).
    constexpr int kCores = 4;
    std::string CaraDoBalao(int n, int cor);

    /// A animacao do confete do balao da cor 'cor': o nome da cor.
    std::string CaraDoConfete(int cor);

    /// A faixa do ramo, de 'inicio' a 'fim', num campo de 'largura'.
    float InicioDaFaixa(int ramo, int ramos, float largura);
    float FimDaFaixa(int ramo, int ramos, float largura);

    /// Onde cai cada confete da faixa: x de cada um, nas kFileiras fileiras, e o atraso (em pixels
    /// acima do cabecalho) de cada fileira.
    struct Confete { float x; float acima; };
    std::vector<Confete> Confetes(float inicio, float fim);

    // ---- O relogio e o desenho ----

    /// Um comando a cada kPorComando: seis na fase (Fase::kDuracao) - as cinco
    /// formas e, no sexto, a primeira de novo, com o E logico as vezes.
    constexpr int kComandosPorFase = 6;
    constexpr float kPorComando = Fase::kDuracao / static_cast<float>(kComandosPorFase);

    /// Os baloes de cada comando: o primeiro kPrimeiroBalao depois de ele abrir, um a cada
    /// kEntreBaloes.
    constexpr int kBaloesPorComando = 3;
    constexpr float kPrimeiroBalao = 0.15f;
    constexpr float kEntreBaloes = 0.9f;

    /// O balao desce do professor ate o losango, kAbaixoDoProfessor abaixo dele, fica kNaDecisao
    /// parado la, e segue a seta do ramo ate o cabecalho.
    constexpr float kAbaixoDoProfessor = 105.0f;
    constexpr float kVelocidadeDoBalao = 950.0f;
    constexpr float kNaDecisao = 0.3f;

    /// Onde o professor fica: nesta altura, num x sorteado a cada comando.
    constexpr float kYDoProfessor = 185.0f;
    constexpr float kXMinimo = 380.0f;
    constexpr float kXMaximo = 820.0f;

    /// Os cabecalhos dos ramos: nesta altura, no meio da faixa.
    constexpr float kYDosRotulos = 96.0f;

    /// O confete: kFileiras fileiras de um a cada kEntreConfetes px, a segunda meio passo para o
    /// lado e kEntreFileiras acima.
    constexpr int kFileiras = 2;
    constexpr float kEntreConfetes = 50.0f;
    constexpr float kEntreFileiras = 40.0f;
    constexpr float kVelocidadeDoConfete = 300.0f;

    /// No switch sem break, cada case de baixo estoura kCascata depois do de
    /// cima.
    constexpr float kCascata = 0.3f;

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// As formas se revezam numa ordem da luta (Variacao::Ordem de kFormas, com o if simples
    /// abrindo): a vez k da fase e Proximo(NaOrdem(ordem, k)).
    constexpr int kFormas = 5;

    /// O que o Dificil muda: um balao a mais por comando, um atras do outro mais depressa, o balao
    /// fica menos tempo no losango. A regra e a mesma.
    struct Degrau {
        int baloesPorComando = kBaloesPorComando;
        float entreBaloes = kEntreBaloes;
        float naDecisao = kNaDecisao;
        float rapidez = 1.0f;          ///< do confete
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
