//
// Os sistemas de tipos da fase 2 do Luizinho.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 2 do Luizinho sao os sistemas de tipos: a mesma conta em linguagens diferentes, e o que
/// cada uma faz quando os tipos nao batem.
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

    /// Como o valor se escreve no codigo: 7, "34".
    std::string Escrito(const Valor& v);

    enum class Operacao { Soma, Produto };
    std::string Simbolo(Operacao op);

    enum class Forma { JsSoma, PhpSoma, PythonSoma, JsProduto, PythonRepete, HaskellSoma };
    constexpr int kFormas = 6;

    /// Um comando: a forma e os dois algarismos dela. JsSoma, PhpSoma, PythonSoma e HaskellSoma: a
    /// + "b".
    struct Comando {
        Forma forma = Forma::JsSoma;
        int a = 3;
        int b = 4;
    };

    Linguagem LinguagemDe(Forma f);
    Operacao OperacaoDe(Forma f);

    /// Os operandos como o codigo os escreve.
    Valor Esquerdo(const Comando& c);
    Valor Direito(const Comando& c);

    enum class Desfecho { Valor, ErroDeExecucao, ErroDeCompilacao };
    struct Resultado {
        Desfecho desfecho = Desfecho::Valor;
        Valor valor;
        bool coagidoEsquerdo = false;
        bool coagidoDireito = false;
        Valor esquerdoDepois;
        Valor direitoDepois;
    };

    /// A conta, do jeito da linguagem.
    Resultado Avaliar(const Comando& c);

    /// O comando da vez: a forma segue a ordem da fase (OrdemDaFase), e os algarismos sao
    /// sorteados.
    Comando Proximo(int vez, std::uint32_t sorteio);

    /// A forma do comando 'vez': a soma do JavaScript e a do php primeiro - a mesma expressao, dois
    /// resultados -, e no fim a tipagem estatica, a que protege.
    Forma OrdemDaFase(int vez);

    /// O placar: a linguagem e como ela tipa, e na linha de baixo a expressao - e, depois da conta,
    /// o resultado ("3 + \"4\" -> \"34\"").
    std::string Placar(const Comando& c);
    std::string PlacarComResultado(const Comando& c);

    // ---- As caras ----

    /// O chip do valor: "Numero7" (de 0 a kMaiorNumero), "Texto4" (o algarismo entre aspas, o
    /// operando).
    constexpr int kMaiorNumero = 20;
    std::string CaraDoOperando(const Valor& v);
    std::string CaraDoNumero(int n);
    std::string CaraDaLetra(char c);

    /// O operador: "Mais", "Vezes"; vermelho quando a conta da erro.
    std::string CaraDoOperador(Operacao op, bool erro);

    /// Os estilhacos: o do TypeError e o da compilacao.
    constexpr const char* kCaraDoTypeError = "TypeError";
    constexpr const char* kCaraDaCompilacao = "Compilacao";

    /// As letras da palavra do resultado texto, com as aspas nas pontas: "34" -> { '"', '3', '4',
    /// '"' }.
    std::vector<char> Letras(const Valor& texto);

    // ---- A geometria ----

    float AberturaDoAnel(int n);
    float CentroDoAnel(int n, float alvo);

    /// Os desvios das letras da palavra em relacao ao meio dela.
    struct Ponto { float x = 0.0f; float y = 0.0f; };
    std::vector<Ponto> DesviosDaPalavra(int letras, float direcao);
    constexpr float kEntreLetras = 26.0f;

    /// Onde a conta roda: em cima do aluno, nesta altura, cortado as margens do campo.
    Ponto OndeFicaOOperador(float xDoAluno, float largura);
    constexpr float kYDoOperador = 470.0f;
    constexpr float kMargemDoOperador = 120.0f;

    /// Os operandos param de cada lado do operador.
    constexpr float kLadoDoOperando = 34.0f;

    /// O professor: nesta altura, num x sorteado a cada comando, balancando kBalanco para cada lado
    /// a cada kPeriodoDoBalanco.
    constexpr float kYDoProfessor = 160.0f;
    constexpr float kXMinimo = 300.0f;
    constexpr float kXMaximo = 900.0f;
    constexpr float kBalanco = 110.0f;
    constexpr float kPeriodoDoBalanco = 3.0f;

    // ---- O relogio ----

    /// Um comando a cada kPorComando, o primeiro kPrimeiro depois de a fase
    /// abrir (antes dele o placar diz o latim).
    constexpr int kComandos = 8;
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorComando = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kComandos);

    /// Quando cada conta comeca: a primeira em kPrimeiro, e cada outra ate kFolgaDoHorario antes ou
    /// depois do ritmo de kPorComando - sorteado, para a fase nao ter compasso -, nunca menos de
    /// kMenorIntervalo depois da anterior.
    std::vector<float> Horarios(std::uint32_t sorteio);
    constexpr float kFolgaDoHorario = 0.6f;
    constexpr float kMenorIntervalo = 1.7f;

    /// Os operandos saem do professor kSaida depois de o comando abrir, voam kVoo ate o operador, e
    /// o convertido troca de chip a kCoercao do voo.
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

    // ---- O que muda de uma luta para a outra ----

    /// A ordem dos comandos desta luta (o comando k e Proximo(ordem[k], ...)): a soma do JavaScript
    /// abre - a coercao mais conhecida - e o Haskell fecha.
    std::vector<int> OrdemDosComandos(Variacao::Sorteio& s);

    // ---- O degrau ----

    /// O que o Dificil muda: os operandos voam em 0,5 s (e nao 0,6) - a conta
    /// roda antes -, e o resultado sai 20% mais rapido.
    struct Degrau {
        float voo = kVoo;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
