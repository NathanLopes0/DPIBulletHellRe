//
// A fase 2 do Vitor: as nuvens e o controle de congestionamento do TCP.
//

#pragma once

#include <string>
#include <vector>

#include "../Dificuldade.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * A FASE 2 DO VITOR E REDES: a camada de transporte. As duas NUVENS em cima
 * dele sao o servidor, e cada chuva e uma JANELA de segmentos TCP. Os
 * segmentos voam das nuvens ate o ROTEADOR, no meio do campo, e o roteador os
 * encaminha ao aluno, um atras do outro, mirados nele.
 *
 * O ROTEADOR TEM CAPACIDADE: por RTT ele da conta de kCapacidade segmentos.
 * O que passar disso e DESCARTADO ali mesmo - o segmento estoura num anel de
 * cacos na porta do roteador. E a janela segue o controle de
 * congestionamento de verdade:
 *
 *   slow start          a janela DOBRA a cada RTT: 1, 2, 4, 8, 16...
 *   evitacao            passado o ssthresh, ela cresce de UM em um
 *   ACKs duplicados     houve perda: ssthresh = cwnd / 2, e a janela cai para
 *                       ele (o Reno); o que se perdeu e RETRANSMITIDO primeiro
 *                       na janela seguinte
 *   timeout             o Ponto Extra derruba as nuvens: quando elas voltam,
 *                       a conexao recomeca do slow start, com cwnd = 1
 *
 * O resultado e o DENTE DE SERRA, desenhado num grafico do cwnd no canto da
 * tela enquanto a fase anda.
 *
 * Aqui ficam a conexao, as janelas, o roteador, os relogios, o grafico e os
 * textos; quem forma as nuvens, manda, enfileira, descarta e encaminha e o
 * Vitor. Coberto por test_tcp.cpp.
 */
namespace Tcp {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoTcp = "StateTwo";
    constexpr const char* kGatilhoDoSegmento = "segmento";      ///< da nuvem ao roteador
    constexpr const char* kGatilhoDoEncaminhado = "encaminhar"; ///< do roteador ao aluno
    constexpr const char* kGatilhoDoDescarte = "descarte";      ///< o anel do descartado

    /// As caras do segmento (o tipo "Pacote"): o novo, o retransmitido e o
    /// caco do descartado.
    constexpr const char* kCaraDoNovo = "Piscando";
    constexpr const char* kCaraDoRetransmitido = "Retransmitido";
    constexpr const char* kCaraDoDescartado = "Descartado";

    // ---- A CONEXAO ----------------------------------------------------------

    enum class Estado { SlowStart, Evitacao };

    constexpr int kJanelaInicial = 1;
    constexpr int kSsthreshInicial = 16;
    constexpr int kCapacidade = 10;
    constexpr int kMenorSsthresh = 2;

    struct Conexao {
        int cwnd = kJanelaInicial;
        int ssthresh = kSsthreshInicial;
        int capacidade = kCapacidade;  ///< do roteador, por RTT (Plano)
        Estado estado = Estado::SlowStart;
        int aRetransmitir = 0;       ///< perdidos na janela anterior
        int rtt = 0;                 ///< quantas janelas ja foram mandadas
    };

    /// Uma janela: os primeiros sao os retransmitidos, depois os novos; os
    /// ULTIMOS 'perdidos' sao os que o roteador descarta (descarte de cauda).
    struct Janela {
        int retransmitidos = 0;
        int novos = 0;
        int perdidos = 0;
        [[nodiscard]] int Total() const { return retransmitidos + novos; }
        [[nodiscard]] bool Perdido(int i) const { return i >= Total() - perdidos; }
        [[nodiscard]] bool Retransmitido(int i) const { return i < retransmitidos; }
    };

    /// @brief Manda a janela deste RTT: cwnd segmentos, os perdidos da janela
    /// anterior primeiro, e descarta o que passar da capacidade.
    Janela Enviar(Conexao& c);

    /// O que os ACKs da janela fizeram com ela.
    enum class Desfecho { Dobrou, Cresceu, Cortou };

    /// @brief O fim do RTT: chegaram os ACKs. Com perda, ACKs duplicados:
    /// ssthresh = cwnd / 2 e a janela cai para ele. Sem perda, ela dobra no
    /// slow start (e passa a evitacao ao chegar no ssthresh) ou cresce de um.
    Desfecho Reconhecer(Conexao& c, const Janela& j);

    /// @brief O timeout: ssthresh = cwnd / 2, e cwnd volta a 1, no slow start.
    void Timeout(Conexao& c);

    // ---- O RELOGIO ----------------------------------------------------------

    /// As nuvens se formam kFormam depois de a fase abrir; o primeiro RTT
    /// comeca ali.
    constexpr float kFormam = 0.6f;

    /// O RTT: a nuvem processa (as luzinhas acendem - e o aviso), manda a
    /// janela, e descansa. E o ciclo da nuvem (Nuvem::Ciclo) com estes tempos.
    constexpr float kProcessa = 1.0f;
    constexpr float kDescansa = 0.75f;
    constexpr float kRtt = kProcessa + kDescansa;

    /// Os segmentos de uma janela saem um a cada kEntreSegmentos, alternando
    /// as duas nuvens.
    constexpr float kEntreSegmentos = 0.05f;

    /// O roteador encaminha um a cada kEntreEncaminhados - o enlace de saida.
    constexpr float kEntreEncaminhados = 0.06f;

    // ---- O CAMPO ------------------------------------------------------------

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    /// @brief O roteador: no meio da largura, kAlturaDoRoteador abaixo do alto
    /// - bem abaixo do professor, que varre o alto do campo com as nuvens.
    Ponto Roteador(float largura, float fracao = 0.5f);
    constexpr float kAlturaDoRoteador = 400.0f;
    constexpr float kChegou = 14.0f;     ///< a distancia em que o segmento entra nele

    /// As velocidades sao as dos ataques em fases.json (um teste confere).
    constexpr float kVelocidadeDoSegmento = 320.0f;
    constexpr float kVelocidadeDoEncaminhado = 340.0f;

    /// O ROTEADOR MIRA EM DOIS LUGARES: os segmentos novos se alternam entre
    /// onde o aluno ESTA e onde ele ESTARA quando o segmento chegar, andando
    /// como anda agora; o retransmitido (a retransmissao rapida) vai sempre
    /// adiante. Quem fica parado leva um, quem anda sempre igual leva o outro;
    /// quem muda de rumo no meio do caminho escapa dos dois.
    ///
    /// METADE DO CAMINHO NAO SERVIA. Medido: o segmento chegava uns 40 px
    /// atras de quem seguia o professor (que anda a 90 px/s), e o piloto que
    /// persegue passou a fase sem acerto nenhum.
    float Antecipacao(int i, bool retransmitido);

    /// @brief Para onde mirar: o aluno em 'aluno', andando a 'velocidade', um
    /// segmento saindo de 'de' a 'rapidez' px/s, antecipando 'quanto' do
    /// caminho. Uma conta so, de primeira ordem (o tempo de voo ate onde ele
    /// esta agora).
    Ponto Mira(Ponto de, Ponto aluno, Ponto velocidade, float rapidez, float quanto);
    constexpr float kVelocidadeDoDescarte = 220.0f;
    constexpr int kAnelDoDescarte = 8;

    /// @brief O meio do anel do descarte de numero k: cada um gira um pouco
    /// em relacao ao anterior, para os cacos de uma perda grande cobrirem as
    /// frestas uns dos outros.
    float GiroDoDescarte(int k, int anel = kAnelDoDescarte);

    // ---- O GRAFICO DO cwnd --------------------------------------------------

    /// A direita, logo abaixo da faixa das nuvens: o eixo do tempo em RTTs e o
    /// do cwnd, com a linha da capacidade. NAO NO CANTO DE CIMA: o professor
    /// varre o alto com uma nuvem de cada lado, e no canto a nuvem da direita
    /// passava por cima do grafico. Com a varredura parando a kMargemDaVarredura
    /// das bordas (fases.json), nem ele nem ela chegam aqui.
    struct Retangulo { float x = 0.0f; float y = 0.0f; float w = 0.0f; float h = 0.0f; };
    Retangulo Grafico(float largura);
    constexpr float kTopoDoGrafico = 132.0f;
    constexpr float kMargemDaVarredura = 380.0f;
    constexpr float kLarguraDoGrafico = 250.0f;
    constexpr float kAlturaDoGrafico = 104.0f;
    constexpr int kRttsNoGrafico = 12;
    constexpr int kMaiorNoGrafico = 16;

    /// @brief Onde o ponto (rtt, cwnd) cai no grafico. O rtt passa do ultimo:
    /// fica no ultimo; o cwnd passa do teto: fica no teto.
    Ponto NoGrafico(int rtt, int cwnd, float largura);

    // ---- OS TEXTOS ----------------------------------------------------------

    /// O placar: "cwnd = 8 · slow start", "cwnd = 9 · evitação",
    /// "3 ACKs duplicados · cwnd = 8", "timeout · cwnd = 1".
    std::string Placar(const Conexao& c);
    std::string PlacarDaPerda(const Conexao& c);
    std::string PlacarDoTimeout(const Conexao& c);

    /// O roteador: quantos desta janela ele ja recebeu, contra a capacidade -
    /// "roteador · 7/10". Em 10/10, o que chega e descartado.
    std::string Fila(int recebidos, int capacidade = kCapacidade);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA (Variacao.h) ---------------------

    /**
     * @brief A rede desta batalha: a capacidade do roteador (8, 10 ou 12 por
     * RTT), o ssthresh com que a conexao abre (8, 12 ou 16) e onde o roteador
     * fica na largura (de 40% a 60%).
     *
     * O dente de serra muda com os dois numeros - com o ssthresh baixo a
     * janela sai do slow start antes de encher o roteador e a primeira perda
     * vem na evitacao, devagar; com a capacidade baixa as perdas vem cedo e
     * vem muitas -, e e o mesmo controle de congestionamento em toda luta. O
     * grafico desenha a linha vermelha na capacidade sorteada.
     */
    struct Plano {
        int capacidade = kCapacidade;
        int ssthreshInicial = kSsthreshInicial;
        float fracaoDoRoteador = 0.5f;
    };
    Plano PlanoDeSempre();
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kCapacidades[] = {8, 10, 12};
    constexpr int kSsthreshs[] = {8, 12, 16};
    constexpr float kMenorFracaoDoRoteador = 0.4f;
    constexpr float kMaiorFracaoDoRoteador = 0.6f;

    /// A conexao que abre a fase com o plano.
    Conexao Abrir(const Plano& p);

    // ---- O DEGRAU (Dificuldade.h) -----------------------------------------------

    /// O que o Dificil muda: o enlace de saida do roteador e mais rapido (sai
    /// mais segmento por segundo, e cada um anda mais depressa), o roteador da
    /// conta de dois a menos por RTT - perde-se mais - e o descarte estoura em
    /// mais cacos. O TCP e o mesmo.
    struct Degrau {
        float entreEncaminhados = kEntreEncaminhados;
        float rapidez = 1.0f;            ///< do encaminhado e do caco
        int anelDoDescarte = kAnelDoDescarte;
        int menosCapacidade = 0;         ///< o roteador da conta de menos: mais perdas, mais aneis
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// A rede da luta no modo: a capacidade do plano menos a do degrau, nunca
    /// abaixo de kMenorCapacidade.
    Plano NoDegrau(Plano p, const Degrau& d);
    constexpr int kMenorCapacidade = 6;
}
