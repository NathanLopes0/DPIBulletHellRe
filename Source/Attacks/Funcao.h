//
// As funcoes da fase 3 do Andre.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A FASE 3 DO ANDRE SAO AS FUNCOES. O professor e o main: cada comando dele
 * e uma chamada, escrita no placar, e cada funcao chamada e um CARTAO no meio
 * do campo com a assinatura dela ("void leque(int n)"). O vetor do chao da
 * fase 2 continua la, e e nele que as funcoes trabalham.
 *
 * Os argumentos sao VALORES que voam do professor ate o cartao da funcao - e
 * so quando chegam a funcao executa. Uma funcao que devolve alguma coisa manda
 * o retorno de volta para QUEM CHAMOU, e o placar troca a chamada pelo valor
 * dela ("baloes(dobro(3), 11);" vira "baloes(6, 11);"):
 *
 *   baloes(2, 5);                     sobe um cacho de cada casa, de v[2] a v[5]
 *   leque(5);                         cinco ponto-e-virgulas miradas no aluno,
 *                                     saindo do cartao
 *   baloes(dobro(3), 11);             o 3 vai ao dobro, o 6 volta, e so entao
 *                                     o baloes e chamado
 *   leque(meio(3, 8));                (3 + 8) / 2 e 5: a divisao de inteiros
 *   baloes(aluno - 1, aluno + 1);     PASSAGEM POR VALOR: a casa do aluno e
 *                                     COPIADA no instante da chamada, e o valor
 *                                     voa ate a funcao - quem saiu dali ja nao
 *                                     esta onde a copia diz
 *   segue(&aluno);                    PASSAGEM POR REFERENCIA: a funcao recebe
 *                                     o ENDERECO do aluno, uma linha liga o
 *                                     cartao a ele, e cada cacho sobe na casa
 *                                     em que ele esta AGORA
 *
 * Quem desenha, move e solta e o Andre; aqui ficam os comandos, as contas, as
 * casas, os textos e o relogio. Coberto por test_funcao.cpp.
 */
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

    /// Um comando. Baloes: baloes(a, b). Leque: leque(a). BaloesDoDobro:
    /// baloes(dobro(a), b). LequeDoMeio: leque(meio(a, b)). PorValor e
    /// PorReferencia nao tem numeros: o numero e a casa do aluno.
    struct Comando {
        Forma forma = Forma::Baloes;
        int a = 0;
        int b = 0;
    };

    /// O vetor do chao e o da fase 2 (Laco::kCasas): casas 0 a kCasas - 1.
    constexpr int kCasas = 12;

    /// @brief As contas das funcoes que devolvem valor. meio e a divisao de
    /// inteiros: meio(3, 8) e 5.
    int Dobro(int x);
    int Meio(int a, int b);

    /// @brief A funcao de fora - a que faz o ataque -, e a de dentro, cujo
    /// retorno vira argumento dela (a mesma, se o comando nao tem uma).
    Nome Externa(const Comando& c);
    Nome Interna(const Comando& c);
    bool TemInterna(const Comando& c);

    /// @brief Os argumentos que vao a funcao de dentro (os do comando inteiro,
    /// se nao ha funcao de dentro), e o que ela devolve.
    std::vector<int> ArgumentosDaInterna(const Comando& c);
    int Retorno(const Comando& c);

    /// @brief Os argumentos que chegam a funcao de fora, ja com o retorno da de
    /// dentro no lugar dela - e, por valor, com a casa do aluno no instante da
    /// chamada. Vazio no segue(&aluno), que recebe um endereco.
    std::vector<int> ArgumentosDaExterna(const Comando& c, int casaDoAluno);

    /// @brief As casas em que o baloes(de, ate) solta um cacho: de 'de' a 'ate',
    /// cortadas ao vetor. Vazio se de > ate - o laco de dentro nao roda.
    std::vector<int> CasasDosBaloes(int de, int ate);

    /// @brief O comando da vez: a forma segue a ordem da fase, e os numeros sao
    /// sorteados.
    Comando Proximo(int vez, std::uint32_t sorteio);

    /// @brief O placar: a chamada como foi escrita ("baloes(dobro(3), 11);"),
    /// e depois que os valores chegaram ("baloes(6, 11);"; por valor, com a
    /// copia: "aluno = 6: baloes(5, 7);").
    std::string Placar(const Comando& c);
    std::string PlacarComValores(const Comando& c, int casaDoAluno);

    /// @brief A assinatura no cartao da funcao, e as animacoes dele.
    std::string Assinatura(Nome f);
    std::string CaraDoCartao(Nome f);
    std::string CaraDoCartaoAceso(Nome f);

    /// @brief A animacao do valor v que voa: de kMenorValor (a casa do aluno
    /// menos um, com ele em v[0]) a kMaiorValor (o dobro de 11).
    constexpr int kMenorValor = -1;
    constexpr int kMaiorValor = 22;
    std::string CaraDoValor(int v);

    /// @brief A casa do vetor em que esta o x, num campo de 'largura' -
    /// cortada ao vetor.
    int CasaDoX(float x, float largura);

    // ---- O RELOGIO E O DESENHO ---------------------------------------------

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

    /// Onde ficam os cartoes: o da funcao de fora no meio, kYDoCartao; o da de
    /// dentro de lado (kXDoLado da largura, de um lado ou do outro),
    /// kYDoCartaoDeDentro.
    constexpr float kYDoCartao = 390.0f;
    constexpr float kYDoCartaoDeDentro = 300.0f;
    constexpr float kXDoLado = 0.25f;

    /// O professor: nesta altura, num x sorteado a cada comando.
    constexpr float kYDoProfessor = 170.0f;
    constexpr float kXMinimo = 400.0f;
    constexpr float kXMaximo = 800.0f;
}
