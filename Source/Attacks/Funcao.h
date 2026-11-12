//
// As funcoes da fase 3 do Andre.
//

#pragma once

#include "../Regras/Dificuldade.h"

#include <cstdint>
#include <string>
#include <vector>

/// A fase 3 do Andre sao as funcoes.
namespace Funcao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaFuncao = "StateThree";
    constexpr const char* kGatilhoDoCartao = "cartao";
    constexpr const char* kGatilhoDoValor = "valor";
    constexpr const char* kGatilhoDoLeque = "leque";

    /// As funcoes que existem, cada uma com o seu cartao.
    enum class Nome { Baloes, Leque, Dobro, Meio, Segue };
    constexpr int kFuncoes = 5;

    enum class Forma { Baloes, Leque, BaloesDoDobro, LequeDoMeio, PorValor, PorReferencia };
    constexpr int kFormas = 6;

    /// Um comando. Baloes: baloes(a, b).
    struct Comando {
        Forma forma = Forma::Baloes;
        int a = 0;
        int b = 0;
    };

    /// O vetor do chao e o da fase 2 (Laco::kCasas): casas 0 a kCasas - 1.
    constexpr int kCasas = 12;

    /// As contas das funcoes que devolvem valor. meio e a divisao de inteiros: meio(3, 8) e 5.
    int Dobro(int x);
    int Meio(int a, int b);

    /// A funcao de fora - a que faz o ataque -, e a de dentro, cujo retorno vira argumento dela (a
    /// mesma, se o comando nao tem uma).
    Nome Externa(const Comando& c);
    Nome Interna(const Comando& c);
    bool TemInterna(const Comando& c);

    /// Os argumentos que vao a funcao de dentro (os do comando inteiro, se nao ha funcao de
    /// dentro), e o que ela devolve.
    std::vector<int> ArgumentosDaInterna(const Comando& c);
    int Retorno(const Comando& c);

    /// Os argumentos que chegam a funcao de fora, ja com o retorno da de dentro no lugar dela - e,
    /// por valor, com a casa do aluno no instante da chamada.
    std::vector<int> ArgumentosDaExterna(const Comando& c, int casaDoAluno);

    /// As casas em que o baloes(de, ate) solta um cacho: de 'de' a 'ate', cortadas ao vetor. Vazio
    /// se de > ate - o laco de dentro nao roda.
    std::vector<int> CasasDosBaloes(int de, int ate);

    /// O comando da vez: a forma segue a ordem da fase, e os numeros sao sorteados.
    Comando Proximo(int vez, std::uint32_t sorteio);

    /// O placar: a chamada como foi escrita ("baloes(dobro(3), 11);"), e depois que os valores
    /// chegaram ("baloes(6, 11);"; por valor, com a copia.
    std::string Placar(const Comando& c);
    std::string PlacarComValores(const Comando& c, int casaDoAluno);

    /// A assinatura no cartao da funcao, e as animacoes dele.
    std::string CaraDoCartao(Nome f);
    std::string CaraDoCartaoAceso(Nome f);

    /// A animacao do valor v que voa: de kMenorValor (a casa do aluno menos um, com ele em v[0]) a
    /// kMaiorValor (o dobro de 11).
    constexpr int kMenorValor = -1;
    constexpr int kMaiorValor = 22;
    std::string CaraDoValor(int v);

    /// A casa do vetor em que esta o x, num campo de 'largura' - cortada ao vetor.
    int CasaDoX(float x, float largura);

    // ---- O relogio e o desenho ----

    /// O primeiro comando kPrimeiro depois de a fase abrir; o seguinte
    /// kEntreComandos depois de a funcao de fora comecar.
    constexpr float kPrimeiro = 0.4f;
    constexpr float kEntreComandos = 0.6f;

    /// Os valores voam a kVelocidadeDoValor, e a funcao fica kExecutando
    /// acesa antes de fazer o que faz (ou de devolver).
    constexpr float kVelocidadeDoValor = 900.0f;
    constexpr float kExecutando = 0.25f;

    /// O baloes(de, ate): um cacho por casa, um a cada kEntreCachos, cada um
    /// com o aviso da casa acesa da fase 2.
    constexpr float kEntreCachos = 0.12f;

    /// O leque: tantas ponto-e-virgulas quanto o argumento, num leque de
    /// kAberturaDoLeque graus mirado no aluno.
    constexpr float kAberturaDoLeque = 50.0f;

    /// O segue(&aluno): um cacho a cada kEntreSegue, por kDuracaoDoSegue, na
    /// casa em que o aluno estiver.
    constexpr float kEntreSegue = 0.4f;
    constexpr float kDuracaoDoSegue = 2.4f;

    /// Onde ficam os cartoes: o da funcao de fora no meio, kYDoCartao.
    constexpr float kYDoCartao = 390.0f;
    constexpr float kYDoCartaoDeDentro = 300.0f;
    constexpr float kXDoLado = 0.25f;

    /// O professor: nesta altura, num x sorteado a cada comando.
    constexpr float kYDoProfessor = 170.0f;
    constexpr float kXMinimo = 400.0f;
    constexpr float kXMaximo = 800.0f;

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// As chamadas se revezam numa ordem da luta (Variacao::Ordem de kFormas, com o baloes(a, b)
    /// abrindo): a vez k da fase e Proximo(NaOrdem(ordem, k)).

    /// O que o Dificil muda: o leque sai mais rapido, e o segue(&aluno) solta
    /// um cacho a cada 0,3 s em vez de 0,4.
    struct Degrau {
        float entreSegue = kEntreSegue;
        float rapidez = 1.0f;          ///< do leque
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
