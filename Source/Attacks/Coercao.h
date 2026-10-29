//
// Os sistemas de tipos da fase 2 do Luizinho.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Dificuldade.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FASE 2 DO LUIZINHO SAO OS SISTEMAS DE TIPOS: a mesma conta em linguagens
 * diferentes, e o que cada uma faz quando os tipos nao batem. O placar diz a
 * linguagem, como ela tipa, e a expressao:
 *
 *   JavaScript - tipagem fraca       3 + "4"     o 3 VIRA TEXTO: "34"
 *   PHP - tipagem fraca              3 + "4"     o "4" VIRA NUMERO: 7
 *   Python - forte e dinamica        3 + "4"     ninguem vira nada: TypeError,
 *                                                quando a conta RODA
 *   JavaScript - tipagem fraca       "6" * "2"   o * so existe para numero:
 *                                                os dois viram, e da 12
 *   Python - forte e dinamica        "3" * 4     texto vezes numero e repetir:
 *                                                "3333", sem converter nada
 *   Haskell - forte e estatica       3 + "4"     recusado ANTES de rodar: o
 *                                                erro e de compilacao
 *
 * No campo, o operador aparece em cima do aluno e os dois operandos voam do
 * professor ate ele - o numero num chip azul, o texto num verde, com aspas. A
 * COERCAO SE VE: no meio do voo, o operando convertido troca de chip. Na
 * chegada, o resultado ataca do jeito do tipo dele: um NUMERO n e um anel de n
 * chips com o n escrito; um TEXTO e a palavra, letra por letra entre aspas,
 * voando para o aluno; o TypeError e um anel de estilhacos no operador - onde
 * a conta rodou. O erro de compilacao estoura no PROFESSOR, antes de os
 * operandos sairem dele: devagar e longe do aluno, que a tipagem estatica
 * protegeu.
 *
 * Quem desenha e move e o Luizinho; aqui ficam as linguagens, a conta, os
 * textos, as caras e o relogio. Coberto por test_coercao.cpp.
 */
namespace Coercao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDosTipos = "StateTwo";
    constexpr const char* kGatilhoDoOperando = "operando";
    constexpr const char* kGatilhoDoOperador = "operador";
    constexpr const char* kGatilhoDoAnel = "anel";
    constexpr const char* kGatilhoDoTexto = "texto";
    constexpr const char* kGatilhoDoErro = "erro";
    constexpr const char* kGatilhoDaCompilacao = "compilacao";

    enum class Linguagem { JavaScript, PHP, Python, Haskell };
    enum class Tipagem { Fraca, ForteDinamica, ForteEstatica };
    Tipagem TipagemDe(Linguagem l);
    std::string Nome(Linguagem l);

    /// Um valor: um numero, ou um texto feito de algarismos.
    enum class Tipo { Numero, Texto };
    struct Valor {
        Tipo tipo = Tipo::Numero;
        int numero = 0;
        std::string texto;
    };
    Valor Numero(int n);
    Valor Texto(const std::string& s);

    /// @brief Como o valor se escreve no codigo: 7, "34".
    std::string Escrito(const Valor& v);

    enum class Operacao { Soma, Produto };
    std::string Simbolo(Operacao op);

    enum class Forma { JsSoma, PhpSoma, PythonSoma, JsProduto, PythonRepete, HaskellSoma };
    constexpr int kFormas = 6;

    /// Um comando: a forma e os dois algarismos dela. JsSoma, PhpSoma,
    /// PythonSoma e HaskellSoma: a + "b". JsProduto: "a" * "b". PythonRepete:
    /// "a" * b.
    struct Comando {
        Forma forma = Forma::JsSoma;
        int a = 3;
        int b = 4;
    };

    Linguagem LinguagemDe(Forma f);
    Operacao OperacaoDe(Forma f);

    /// @brief Os operandos como o codigo os escreve.
    Valor Esquerdo(const Comando& c);
    Valor Direito(const Comando& c);

    enum class Desfecho { Valor, ErroDeExecucao, ErroDeCompilacao };
    struct Resultado {
        Desfecho desfecho = Desfecho::Valor;
        Valor valor;
        /// O operando que a linguagem converteu, e no que ele virou.
        bool coagidoEsquerdo = false;
        bool coagidoDireito = false;
        Valor esquerdoDepois;
        Valor direitoDepois;
    };

    /// @brief A conta, do jeito da linguagem.
    Resultado Avaliar(const Comando& c);

    /// @brief O comando da vez: a forma segue a ordem da fase (OrdemDaFase), e
    /// os algarismos sao sorteados.
    Comando Proximo(int vez, std::uint32_t sorteio);

    /// @brief A forma do comando 'vez': a soma do JavaScript e a do PHP primeiro
    /// - a mesma expressao, dois resultados -, e no fim a tipagem estatica, a
    /// que protege. As fracas voltam no meio com outros numeros.
    Forma OrdemDaFase(int vez);

    /// @brief O placar: a linguagem e como ela tipa, e na linha de baixo a
    /// expressao - e, depois da conta, o resultado ("3 + \"4\"  ->  \"34\"").
    std::string Placar(const Comando& c);
    std::string PlacarComResultado(const Comando& c);

    // ---- AS CARAS -----------------------------------------------------------

    /// O chip do valor: "Numero7" (de 0 a kMaiorNumero), "Texto4" (o
    /// algarismo entre aspas, o operando). As letras da palavra vao sem aspas,
    /// "Letra4", e as aspas sao chips proprios, "Aspas".
    constexpr int kMaiorNumero = 20;
    std::string CaraDoOperando(const Valor& v);
    std::string CaraDoNumero(int n);
    std::string CaraDaLetra(char c);

    /// O operador: "Mais", "Vezes"; vermelho quando a conta da erro.
    std::string CaraDoOperador(Operacao op, bool erro);

    /// Os estilhacos: o do TypeError e o da compilacao.
    constexpr const char* kCaraDoTypeError = "TypeError";
    constexpr const char* kCaraDaCompilacao = "Compilacao";

    /// @brief As letras da palavra do resultado texto, com as aspas nas
    /// pontas: "34" -> { '"', '3', '4', '"' }.
    std::vector<char> Letras(const Valor& texto);

    // ---- A GEOMETRIA --------------------------------------------------------

    /// @brief O anel de n: a abertura total do leque (a WaveAttack reparte de
    /// ponta a ponta, entao o anel cheio e 360 menos um passo) e o angulo
    /// central que poe UM chip exatamente na direcao 'alvo' (graus) - com n
    /// par, meio passo de giro, senao o alvo ficaria na fresta do meio.
    float AberturaDoAnel(int n);
    float CentroDoAnel(int n, float alvo);

    /// @brief Os desvios das letras da palavra em relacao ao meio dela: lado a
    /// lado, de frente para quem a recebe (perpendiculares a 'direcao', em
    /// graus), a kEntreLetras uma da outra.
    struct Ponto { float x = 0.0f; float y = 0.0f; };
    std::vector<Ponto> DesviosDaPalavra(int letras, float direcao);
    constexpr float kEntreLetras = 26.0f;

    /// @brief Onde a conta roda: em cima do aluno, nesta altura, cortado as
    /// margens do campo.
    Ponto OndeFicaOOperador(float xDoAluno, float largura);
    constexpr float kYDoOperador = 470.0f;
    constexpr float kMargemDoOperador = 120.0f;

    /// Os operandos param de cada lado do operador.
    constexpr float kLadoDoOperando = 34.0f;

    /// O professor: nesta altura, num x sorteado a cada comando, BALANCANDO
    /// kBalanco para cada lado a cada kPeriodoDoBalanco. Parado no alto ele
    /// seria tiro de graca para quem fica embaixo dele entre uma conta e outra.
    constexpr float kYDoProfessor = 160.0f;
    constexpr float kXMinimo = 300.0f;
    constexpr float kXMaximo = 900.0f;
    constexpr float kBalanco = 110.0f;
    constexpr float kPeriodoDoBalanco = 3.0f;

    // ---- O RELOGIO ----------------------------------------------------------

    /// Um comando a cada kPorComando, o primeiro kPrimeiro depois de a fase
    /// abrir (antes dele o placar diz o latim).
    constexpr int kComandos = 8;
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorComando = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kComandos);

    /// Os operandos saem do professor kSaida depois de o comando abrir, voam
    /// kVoo ate o operador, e o convertido troca de chip a kCoercao do voo. A
    /// conta roda na chegada. Na tipagem estatica eles nem saem: kChecagem
    /// depois de nascerem, o compilador os recusa ali mesmo.
    constexpr float kSaida = 0.3f;
    constexpr float kVoo = 0.6f;
    constexpr float kCoercao = 0.55f;
    constexpr float kChecagem = 0.35f;
    constexpr float kConta = kSaida + kVoo;

    /// As velocidades sao as dos ataques em fases.json (um teste confere): o
    /// projetil da WaveAttack nasce na velocidade do arquivo.
    constexpr float kVelocidadeDoAnel = 240.0f;
    constexpr float kVelocidadeDoTexto = 420.0f;
    constexpr float kVelocidadeDoErro = 260.0f;
    constexpr float kVelocidadeDaCompilacao = 150.0f;

    /// Os aneis de erro: quantos estilhacos.
    constexpr int kEstilhacosDoErro = 20;
    constexpr int kEstilhacosDaCompilacao = 10;

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA ---------------------------------

    /// A ORDEM DOS COMANDOS desta luta (o comando k e Proximo(ordem[k], ...)):
    /// a soma do JavaScript abre - a coercao mais conhecida - e o Haskell
    /// fecha - a tipagem que protege -, e as seis do meio vem em qualquer
    /// ordem.
    std::vector<int> OrdemDosComandos(Variacao::Sorteio& s);

    // ---- O DEGRAU -------------------------------------------------------------

    /// O que o Dificil muda: os operandos voam em 0,5 s (e nao 0,6) - a conta
    /// roda antes -, e o resultado sai 20% mais rapido.
    struct Degrau {
        float voo = kVoo;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
