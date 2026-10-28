//
// O golpe do orientador no TCC: o da materia que ele da.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "Math.h"
#include "Variacao.h"

/**
 * CAMADA PURA. Coberto por test_golpe.cpp.
 *
 * NO TCC O ORIENTADOR LUTA DO LADO DO ALUNO (Orientador), e cada professor
 * ataca com a materia dele. O tiro e sempre o do aluno - a mesma nota por
 * acerto -, e todos os golpes tem o mesmo peso, uns dois tiros por segundo no
 * chefe: o que muda e o DESENHO, e o que ele ensina.
 *
 *   Andre       for (i = 0; i < 4; i++): quatro tiros em fila, um por volta
 *   Salles      a pilha: empilha quatro tiros em cima dele e desempilha do
 *               ultimo para o primeiro - LIFO
 *   Ricardo     o barramento: oito fios lado a lado, e so os bits 1 do byte
 *               da vez atiram, todos paralelos
 *   Thiago      a transacao: cada escrita fica numa linha, esperando, e o
 *               COMMIT manda todas de uma vez
 *   Luizinho    o ponteiro: dois tiros saem de lado e apontam para o chefe
 *               ate acha-lo, fazendo a curva
 *   Vladimir    a gramatica S -> S S: um tiro que vira dois, que viram quatro
 *   Julio       a descida do gradiente: o tiro oscila em volta do caminho e
 *               converge, cada vez menos
 *   Maria Lucia o Observer: ela nao atira sozinha - a cada kNotificacoes tiros
 *               do aluno, e notificada e atira um
 *   Lucas       o fetch: o tiro vai para o lado, ao servidor, e a resposta
 *               volta e cai no chefe
 *   Vitor       a janela do TCP: rajadas que crescem - 1, 2, 4, e de um em
 *               um ate 6 - e caem pela metade na perda, o dente de serra
 *
 * O que nao e professor de materia (ou um nome desconhecido) atira o leque de
 * antes: tres tiros a cada 1,5 s.
 */
namespace Golpe {

    enum class Materia {
        Generico, Andre, Salles, Ricardo, Thiago, Luizinho, Vladimir, Julio, MariaLucia, Lucas, Vitor
    };

    /// @brief A materia pelo nome do professor em materias.json ("Andre",
    /// "Maria Lucia", com ou sem acento). Desconhecido: Generico.
    Materia DoProfessor(const std::string& professor);

    constexpr float kVelocidade = 800.0f;

    /// Um tiro da rodada. Tudo e relativo: a hora ao comeco da rodada, o lugar
    /// ao orientador, o rumo a direcao do chefe.
    struct Tiro {
        float quando = 0.0f;
        float dx = 0.0f;
        float dy = 0.0f;
        float desvio = 0.0f;        ///< graus a partir da direcao do chefe
        bool paralelo = false;      ///< o rumo e o do orientador ao chefe, e nao o do lugar do tiro
        float velocidade = kVelocidade;
        float espera = 0.0f;        ///< parado no lugar, junto do orientador, antes de partir
        float curva = 0.0f;         ///< graus por segundo de curva na direcao do chefe
        float reMira = -1.0f;       ///< nesse instante do voo, aponta de novo para o chefe
        float onda = 0.0f;          ///< amplitude da oscilacao de lado, que se apaga
        float divide = -1.0f;       ///< nesse instante do voo, vira dois
        int geracoes = 0;           ///< quantas vezes os filhos ainda se dividem

        /// O orientador precisa acompanhar o tiro depois de solta-lo.
        [[nodiscard]] bool Guiado() const {
            return espera > 0.0f || curva > 0.0f || reMira > 0.0f || onda > 0.0f || divide > 0.0f;
        }
    };

    struct Rodada {
        float duracao = 1.5f;
        std::vector<Tiro> tiros;
    };

    /// @brief A rodada 'k' (0 e a primeira) do golpe da materia 'm'. A da Maria
    /// Lucia nao tem tiro: o dela vem do aluno (Notificacoes).
    Rodada DaVez(Materia m, int k, Variacao::Sorteio& s);

    // ---- AS CONTAS DE CADA QUADRO ------------------------------------------

    /// @brief A direcao (comprimento 1) de 'de' para 'alvo', girada 'desvio'
    /// graus. Com o alvo em cima do atirador, para cima.
    Vector2 Rumo(const Vector2& de, const Vector2& alvo, float desvio);

    /// @brief A 'velocidade' girada na direcao de 'paraOAlvo' no maximo
    /// 'grausPorSegundo' * 'dt', sem mudar o modulo. O ponteiro.
    Vector2 Curvar(const Vector2& velocidade, const Vector2& paraOAlvo, float grausPorSegundo, float dt);

    /// @brief O deslocamento de lado do tiro do gradiente aos 't' segundos de
    /// voo: uma senoide que se apaga (kTempoDaOnda), comecando do zero.
    float Oscilacao(float t, float amplitude);
    constexpr float kTempoDaOnda = 0.22f;
    constexpr float kVoltasDaOnda = 3.0f;    ///< por segundo

    /// O Observer: a cada quantos tiros do aluno ela atira um.
    constexpr int kNotificacoes = 4;

    /// @brief Quantos tiros ela deve agora, com o aluno tendo dado 'doAluno'
    /// tiros desde o comeco e ela 'dela'.
    int Notificacoes(int doAluno, int dela);

    /// @brief A janela do TCP na rodada 'k': o slow start 1, 2, 4; depois de
    /// um em um ate 6; a perda corta pela metade (3) e o dente recomeca.
    int Janela(int k);

    /// @brief Os bits do byte do barramento: oito, com exatamente kBitsUm em 1.
    std::array<bool, 8> Byte(Variacao::Sorteio& s);
    constexpr int kBitsUm = 3;

    /// Na gramatica, quanto os dois filhos abrem um do outro.
    constexpr float kAberturaDosFilhos = 9.0f;
}
