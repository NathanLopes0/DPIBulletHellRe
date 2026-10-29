//
// A fase 2 do Vladimir: a construcao de um compilador, de ponta a ponta.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"
#include "../Engine/Fase.h"

/**
 * CAMADA PURA. Coberto por test_compilador.cpp.
 *
 * O VLADIMIR TAMBEM DA CONSTRUCAO DE COMPILADORES, e a fase 2 e um compilador
 * trabalhando sobre uma linha de codigo gerada a cada rodada - "x = a + b * 2;":
 *
 *   lexico      o scanner varre a linha e a quebra em TOKENS; cada token
 *               reconhecido cai em coluna. O automato da fase 1 e exatamente
 *               isto: um analisador lexico e um automato finito.
 *   sintatico   o parser monta a ARVORE SINTATICA com a precedencia da
 *               gramatica - o * antes do + -, de baixo para cima; cada
 *               reducao de operador solta um leque.
 *   codigo      a arvore e percorrida em pos-ordem, e cada no gera a
 *               instrucao de uma maquina de pilha - LOAD a, PUSH 2, MUL, ADD,
 *               STORE x -, que sai mirada no aluno.
 *
 * OS IDENTIFICADORES SAO a E b, as letras do Vladimir: caem como as letras
 * caem, o "b" persegue, e o Programa de Intercambio os troca como troca toda
 * letra em voo.
 *
 * Quem desenha, move e atira e o Vladimir; aqui moram a linha, os tokens, a
 * arvore, o codigo, as posicoes e o relogio de cada rodada.
 */
namespace Compilador {

    /// Os nomes que o codigo e os dados dividem.
    constexpr const char* kFaseDoCompilador = "StateTwo";
    constexpr const char* kGatilhoDoLexema = "lexema";       ///< o token que nao e letra, caindo
    constexpr const char* kGatilhoDoA = "ler_a";             ///< o identificador a, caindo
    constexpr const char* kGatilhoDoB = "ler_b";             ///< o identificador b, caindo
    constexpr const char* kGatilhoDoNo = "arvore";           ///< o no que sobe para a arvore
    constexpr const char* kGatilhoDaReducao = "reduzir";     ///< o leque de cada reducao
    constexpr const char* kGatilhoDaInstrucao = "emitir";    ///< a instrucao de cada no
    constexpr const char* kGatilhoDoStore = "store";         ///< o anel do STORE, no fim
    constexpr const char* kPostoDaArvore = "arvore";         ///< os nos da arvore em campo


    // ---- O LEXICO -------------------------------------------------------------

    enum class Tipo { Id, Num, Op, Igual, Fim };
    struct Token {
        Tipo tipo = Tipo::Id;
        std::string texto;
    };

    /// @brief O scanner: a linha em tokens - letras sao identificadores, um
    /// algarismo e um numero, + - * sao operadores, = e ; sao eles mesmos. O
    /// espaco separa e nao vira token.
    std::vector<Token> Lexico(const std::string& linha);

    // ---- O SINTATICO ----------------------------------------------------------

    struct No {
        Tipo tipo = Tipo::Id;
        std::string texto;
        int esq = -1;
        int dir = -1;
        int token = -1;   ///< o indice do token de onde o no saiu - o parser o pega da linha
    };

    struct Arvore {
        std::vector<No> nos;
        int raiz = -1;
    };

    /// @brief O parser de "id = expressao ;", com a precedencia de sempre: * antes
    /// de + e -, e da esquerda para a direita entre iguais. A raiz e o =, com o
    /// alvo a esquerda e a expressao a direita.
    Arvore Sintatico(const std::vector<Token>& tokens);

    /// @brief A ORDEM DO PARSER, de baixo para cima: os indices dos nos em
    /// pos-ordem - as folhas antes de quem as junta, e o = por ultimo.
    std::vector<int> PosOrdem(const Arvore& a);

    // ---- O CODIGO -------------------------------------------------------------

    /// @brief A instrucao do no, numa maquina de pilha: "LOAD a", "PUSH 2",
    /// "ADD", "SUB", "MUL", "STORE x". O alvo do = nao gera nada sozinho.
    std::string Instrucao(const Arvore& a, int no);

    // ---- A LINHA DE CADA RODADA ----------------------------------------------

    /// @brief Uma linha "x = ...;" com 2 ou 3 operadores sorteados entre + - *, e
    /// folhas a, b ou um algarismo - a e b aparecem as duas, sempre.
    std::string Gerar(Variacao::Sorteio& s);

    // ---- AS CARAS --------------------------------------------------------------

    /// A animacao do token no tipo "Lexema" de vladimir/projeteis.json: "X",
    /// "Igual", "Mais", "Menos", "Vezes", "Fim", "N1" a "N9".
    std::string CaraDoLexema(const std::string& texto);

    /// A animacao da instrucao: "LOAD", "PUSH", "ADD", "SUB", "MUL", "STORE".
    std::string CaraDaInstrucao(const std::string& instrucao);

    /// A animacao do no da arvore: a do lexema dele, menos as folhas a e b, que
    /// tem ficha propria na cor da letra - "IdA" e "IdB". O no e enfeite, e nao
    /// a letra que persegue: o nome nao e "A" nem "B" para o intercambio nao o
    /// confundir com uma.
    std::string CaraDoNo(const Arvore& a, int no);

    // ---- AS POSICOES -----------------------------------------------------------

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    /// A linha do codigo, no alto, e a arvore embaixo dela: um nivel a cada
    /// kEntreNiveis, e as colunas pela ordem em que os nos aparecem na linha.
    constexpr float kYDaLinha = 130.0f;
    constexpr float kTopoDaArvore = 220.0f;
    constexpr float kEntreNiveis = 64.0f;
    constexpr float kEntreColunas = 96.0f;

    std::vector<Ponto> PosicoesDaLinha(const std::vector<Token>& tokens, float largura);
    std::vector<Ponto> PosicoesDaArvore(const Arvore& a, float largura);

    // ---- O RELOGIO ---------------------------------------------------------------

    /// Tres rodadas por fase: a primeira kPrimeira depois de abrir, uma a cada
    /// kRodada. Dentro dela, um token a cada kPorToken, um no a cada kPorNo,
    /// uma instrucao a cada kPorInstrucao (no Normal), com uma pausa entre as
    /// etapas.
    constexpr int kRodadas = 3;
    constexpr float kPrimeira = 0.6f;
    constexpr float kRodada = (Fase::kDuracao - kPrimeira) / static_cast<float>(kRodadas);
    constexpr float kPorToken = 0.18f;
    constexpr float kPorNo = 0.14f;
    constexpr float kPorInstrucao = 0.3f;
    constexpr float kPausa = 0.25f;

    enum class Etapa { Lexico, Sintatico, Codigo, Pronto };

    /// Quando cada etapa comeca, desde o comeco da rodada.
    struct Marcos {
        float sintatico = 0.0f;
        float codigo = 0.0f;
        float pronto = 0.0f;
    };
    Marcos MarcosDa(int tokens, int nos, float porInstrucao = kPorInstrucao);
    Etapa EtapaEm(float t, const Marcos& m);

    // ---- O PLACAR -------------------------------------------------------------------

    /// @brief O placar: a linha, e embaixo a etapa - "l\xc3\xa9xico", "sint\xc3\xa1tico" ou a
    /// instrucao que acabou de sair.
    std::string Placar(const std::string& linha, Etapa e, const std::string& instrucao = "");


    // ---- A LUTA SORTEADA -----------------------------------------------------------

    /// @brief As linhas das tres rodadas, sorteadas antes do primeiro quadro
    /// (Variacao.h): cada luta compila outro programa.
    struct Plano {
        std::array<std::string, kRodadas> linhas;
    };
    Plano Sortear(Variacao::Sorteio& s);

    // ---- O DEGRAU ------------------------------------------------------------------


    /// @brief O que o Dificil muda: uma instrucao a cada 0,24 s (e nao 0,3).
    struct Degrau {
        float porInstrucao = kPorInstrucao;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
