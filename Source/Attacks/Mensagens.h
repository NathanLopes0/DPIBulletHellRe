//
// A fase 3 do Vitor: o fork e a troca de mensagens.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Dificuldade.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FASE 3 DO VITOR E A PROGRAMACAO DISTRIBUIDA. Ele se divide pela nuvem
 * (Nuvem.h) e saem DOIS PROCESSOS do fork() - o cliente e o servidor -, um de cada
 * lado do alto do campo. Processos nao dividem memoria: conversam TROCANDO
 * MENSAGENS. Entre os dois fica o CANAL, um arco que desce pelo campo, e cada
 * mensagem e um envelope que corre por ele.
 *
 * Quatro rodadas, o resto da ementa da INF 310:
 *
 *   assincrona   o servidor manda e NAO ESPERA: os envelopes enchem a caixa
 *                postal do cliente, e o cliente faz receive() um a um - cada
 *                mensagem recebida e processada vira um tiro no aluno
 *   sincrona     o cliente manda e BLOQUEIA ate o servidor receber: o rendezvous.
 *                Quando o envelope chega, os dois andam juntos - e os dois
 *                atiram juntos, um anel cada
 *   rpc          a chamada de procedimento remoto: o cliente chama soma(2, 3) no
 *                servidor, o pedido vai, o servidor calcula, a resposta volta - e
 *                o resultado e um leque de 5 no aluno. Depois soma(3, 4)
 *   impasse      os dois fazem send() sincrono AO MESMO TEMPO, e ninguem
 *                recebe: os envelopes param na porta, os dois esperam - o
 *                deadlock de mensagens. No timeout eles estouram, e os dois
 *                passam a mandar sem esperar
 *
 * Aqui ficam as rodadas (um roteiro de eventos), os lugares dos dois
 * processos, o arco do canal, a caixa postal e os textos; quem move os
 * corpos e os envelopes e atira e o Vitor. Coberto por test_mensagens.cpp.
 */
namespace Mensagens {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDasMensagens = "StateThree";
    constexpr const char* kGatilhoDoEnvelope = "envelope";
    constexpr const char* kGatilhoDoProcessado = "processar";
    constexpr const char* kGatilhoDoEncontro = "encontro";

    /// As caras do envelope (o tipo "Envelope"): correndo, e parado na porta
    /// de quem nao recebe.
    constexpr const char* kCaraDoEnvelope = "Voando";
    constexpr const char* kCaraDoEsperando = "Esperando";

    constexpr int kCliente = 0;
    constexpr int kServidor = 1;
    std::string Nome(int processo);

    // ---- AS RODADAS ---------------------------------------------------------

    enum class Rodada { Assincrona, Sincrona, Rpc, Impasse };
    constexpr int kRodadas = 4;
    Rodada RodadaDaVez(int k);

    enum class Acontece {
        Envia,      ///< um envelope sai de 'de' para o outro
        Recebe,     ///< 'de' tira um envelope da caixa postal e o processa: um tiro
        Encontro,   ///< o rendezvous: os dois atiram um anel
        Responde,   ///< o retorno da rpc sai de 'de' (o servidor)
        Resultado,  ///< o cliente recebe o retorno: um leque de 'valor' no aluno
        Estoura     ///< o timeout do impasse: os envelopes parados estouram
    };

    struct Evento {
        float quando = 0.0f;         ///< segundos desde o comeco da rodada
        Acontece o = Acontece::Envia;
        int de = kCliente;
        int valor = 0;               ///< o resultado da rpc
        float voo = 0.0f;            ///< Envia e Responde: quanto o envelope leva
        bool paraNaPorta = false;    ///< Envia do impasse: ninguem recebe
    };

    /// A RPC: as duas chamadas e o que elas devolvem.
    struct Chamada { int a = 0; int b = 0; };
    constexpr int kChamadas = 2;
    /// As chamadas de sempre: soma(2, 3) e soma(3, 4).
    Chamada ChamadaDaVez(int i);
    int Soma(const Chamada& c);

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA (Variacao.h) ---------------------

    /**
     * @brief A fase desta batalha.
     *
     * A ASSINCRONA ABRE SEMPRE: e a troca de mensagens mais simples, e as
     * outras tres sao o que se pede a mais dela - esperar, devolver um valor,
     * esperar os dois ao mesmo tempo. Essas vem em qualquer ordem. As duas
     * chamadas da rpc sao outras contas, com somas IMPARES (o leque do retorno
     * tem um tiro no meio, mirado no aluno), e os dois processos ficam em
     * outros lugares a cada rodada (Lugar), sempre no alto, o cliente a esquerda.
     */
    struct Plano {
        std::array<Rodada, kRodadas> rodadas{};
        std::array<Chamada, kChamadas> chamadas{};
        std::array<std::array<Ponto, 2>, kRodadas> lugares{};   ///< x em fracao da largura, y em px
    };
    Plano PlanoDeSempre();
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kSomas[] = {5, 7, 9};

    // ---- O DEGRAU (Dificuldade.h) -----------------------------------------------

    /// O que o Dificil muda: a caixa postal da assincrona enche com mais
    /// envelopes (o servidor manda mais depressa, e o cliente recebe mais depressa) e
    /// cada receive() atira um leque de tres; a sincrona tem um rendezvous a
    /// mais (o envelope corre mais depressa); os aneis tem mais tiros, depois
    /// do timeout os dois mandam mais, e o que eles atiram anda mais depressa.
    struct Degrau {
        int sincronos = 3;
        float vooSincrono = 1.1f;
        float entreSincronos = 1.45f;
        int lequeDoRecebido = 1;       ///< cada receive() atira um leque deste tamanho
        float aberturaDoRecebido = 0.0f;
        int assincronos = 12;
        float entreEnvios = 0.18f;
        float entreRecebidos = 0.25f;
        int depoisDoTimeout = 4;
        float entreDepoisDoTimeout = 0.25f;
        int anelDoEncontro = 8;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O roteiro da rodada, em ordem de tempo - o de sempre, ou o desta luta.
    std::vector<Evento> Roteiro(Rodada r);
    std::vector<Evento> Roteiro(Rodada r, const Plano& p, const Degrau& d);

    /// @brief Quanto tempo depois de sair um envelope chega - e, no impasse,
    /// quando ele para na porta (kNaPorta do caminho).
    constexpr float kNaPorta = 0.88f;

    // ---- O RELOGIO ----------------------------------------------------------

    /// As rodadas comecam kComeca depois de a fase abrir: antes disso e o fork
    /// pela nuvem (Nuvem::Fork, que acaba em 1,6 s).
    constexpr float kComeca = 1.8f;
    constexpr float kPorRodada = (Fase::kDuracao - kComeca) / static_cast<float>(kRodadas);

    // ---- O CAMPO ------------------------------------------------------------

    /// Onde fica cada processo na rodada k: o cliente a esquerda, o servidor a
    /// direita, no alto, cada rodada num par de alturas - os de sempre, ou os
    /// desta luta.
    Ponto Lugar(int processo, int k, float largura);
    Ponto Lugar(int processo, int k, float largura, const Plano& p);

    /// @brief O ARCO DO CANAL: de 'de' a 'para', descendo ate 'fundo' no meio.
    /// s vai de 0 (em 'de') a 1 (em 'para').
    Ponto NoArco(Ponto de, Ponto para, float s, float fundo);

    /// @brief O fundo do arco em cada rodada: quanto ele desce pelo campo.
    ///
    /// NAS DUAS RODADAS DE MUITOS ENVELOPES ELE VAI ATE O CHAO (700; o aluno
    /// anda ate 698): o canal corta o campo em dois, e quem estava no meio de
    /// baixo escolhe um lado - o de um dos processos - ou passa entre dois
    /// envelopes. Nas outras ele passa por cima de quem fica embaixo. Raso em
    /// todas, o piloto que vai e vem cruzava o meio sem encostar em nada e
    /// passava com 69,6.
    float Fundo(Rodada r);

    // ---- A CAIXA POSTAL -----------------------------------------------------

    /// @brief Quantos envelopes ha na caixa de 'processo' em 'u': os que ja
    /// chegaram menos os que ele ja recebeu. Pelo roteiro, para o placar.
    int NaCaixa(const std::vector<Evento>& roteiro, int processo, float u);

    // ---- AS VELOCIDADES -----------------------------------------------------

    /// As dos ataques em fases.json (um teste confere).
    constexpr float kVelocidadeDoProcessado = 320.0f;
    constexpr float kVelocidadeDoEncontro = 220.0f;
    constexpr int kAnelDoEncontro = 8;

    // ---- OS TEXTOS ----------------------------------------------------------

    std::string Placar(Rodada r, int naCaixa, int chamada, bool bloqueado, bool estourou);
    std::string Placar(Rodada r, int naCaixa, int chamada, bool bloqueado, bool estourou, const Plano& p);
}
