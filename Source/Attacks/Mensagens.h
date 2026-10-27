//
// A fase 3 do Vitor: o fork e a troca de mensagens.
//

#pragma once

#include <string>
#include <vector>

#include "Fase.h"

/**
 * CAMADA PURA.
 *
 * A FASE 3 DO VITOR E A PROGRAMACAO DISTRIBUIDA. Ele se divide pela nuvem
 * (Nuvem.h) e saem DOIS PROCESSOS - o pai e o filho do fork() -, um de cada
 * lado do alto do campo. Processos nao dividem memoria: conversam TROCANDO
 * MENSAGENS. Entre os dois fica o CANAL, um arco que desce pelo campo, e cada
 * mensagem e um envelope que corre por ele.
 *
 * Quatro rodadas, o resto da ementa da INF 310:
 *
 *   assincrona   o filho manda e NAO ESPERA: os envelopes enchem a caixa
 *                postal do pai, e o pai faz receive() um a um - cada
 *                mensagem recebida e processada vira um tiro no aluno
 *   sincrona     o pai manda e BLOQUEIA ate o filho receber: o rendezvous.
 *                Quando o envelope chega, os dois andam juntos - e os dois
 *                atiram juntos, um anel cada
 *   rpc          a chamada de procedimento remoto: o pai chama soma(2, 3) no
 *                filho, o pedido vai, o filho calcula, a resposta volta - e
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

    constexpr int kPai = 0;
    constexpr int kFilho = 1;
    std::string Nome(int processo);

    // ---- AS RODADAS ---------------------------------------------------------

    enum class Rodada { Assincrona, Sincrona, Rpc, Impasse };
    constexpr int kRodadas = 4;
    Rodada RodadaDaVez(int k);

    enum class Acontece {
        Envia,      ///< um envelope sai de 'de' para o outro
        Recebe,     ///< 'de' tira um envelope da caixa postal e o processa: um tiro
        Encontro,   ///< o rendezvous: os dois atiram um anel
        Responde,   ///< o retorno da rpc sai de 'de' (o filho)
        Resultado,  ///< o pai recebe o retorno: um leque de 'valor' no aluno
        Estoura     ///< o timeout do impasse: os envelopes parados estouram
    };

    struct Evento {
        float quando = 0.0f;         ///< segundos desde o comeco da rodada
        Acontece o = Acontece::Envia;
        int de = kPai;
        int valor = 0;               ///< o resultado da rpc
        float voo = 0.0f;            ///< Envia e Responde: quanto o envelope leva
        bool paraNaPorta = false;    ///< Envia do impasse: ninguem recebe
    };

    /// O roteiro da rodada, em ordem de tempo.
    std::vector<Evento> Roteiro(Rodada r);

    /// A RPC: as duas chamadas e o que elas devolvem.
    struct Chamada { int a = 0; int b = 0; };
    constexpr int kChamadas = 2;
    Chamada ChamadaDaVez(int i);
    int Soma(const Chamada& c);

    /// @brief Quanto tempo depois de sair um envelope chega - e, no impasse,
    /// quando ele para na porta (kNaPorta do caminho).
    constexpr float kNaPorta = 0.88f;

    // ---- O RELOGIO ----------------------------------------------------------

    /// As rodadas comecam kComeca depois de a fase abrir: antes disso e o fork
    /// pela nuvem (Nuvem::Fork, que acaba em 1,6 s).
    constexpr float kComeca = 1.8f;
    constexpr float kPorRodada = (Fase::kDuracao - kComeca) / static_cast<float>(kRodadas);

    // ---- O CAMPO ------------------------------------------------------------

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    /// Onde fica cada processo na rodada k: o pai a esquerda, o filho a
    /// direita, no alto, cada rodada num par de alturas.
    Ponto Lugar(int processo, int k, float largura);

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
}
