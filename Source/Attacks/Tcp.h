//
// A fase 2 do Vitor: as nuvens e o controle de congestionamento do TCP.
//

#pragma once

#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 2 do Vitor e redes: a camada de transporte. As duas nuvens em cima dele sao o servidor, e
/// cada chuva e uma janela de segmentos TCP.
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

    // ---- A conexao ----

    enum class Estado { SlowStart, Evitacao };

    constexpr int kJanelaInicial = 1;
    constexpr int kSsthreshInicial = 16;
    constexpr int kCapacidade = 10;
    constexpr int kMenorSsthresh = 2;

    struct Conexao {
        int cwnd = kJanelaInicial;
        int ssthresh = kSsthreshInicial;
        int capacidade = kCapacidade;  ///< Do roteador, por RTT (Plano)
        Estado estado = Estado::SlowStart;
        int aRetransmitir = 0;       ///< perdidos na janela anterior
        int rtt = 0;                 ///< quantas janelas ja foram mandadas
    };

    /// Uma janela: os primeiros sao os retransmitidos, depois os novos; os ultimos 'perdidos' sao
    /// os que o roteador descarta (descarte de cauda), e o 'ruido' e o que o enlace perdeu no
    /// caminho (-1: nenhum).
    struct Janela {
        int retransmitidos = 0;
        int novos = 0;
        int perdidos = 0;
        int ruido = -1;
        [[nodiscard]] int Total() const { return retransmitidos + novos; }
        [[nodiscard]] bool Perdido(int i) const { return i >= Total() - perdidos; }
        [[nodiscard]] bool NoRuido(int i) const { return i == ruido && !Perdido(i); }
        [[nodiscard]] bool Retransmitido(int i) const { return i < retransmitidos; }
        /// Os da cauda e o do ruido.
        [[nodiscard]] int Perdas() const { return perdidos + (ruido >= 0 && NoRuido(ruido) ? 1 : 0); }
    };

    /// Manda a janela deste RTT: cwnd segmentos, os perdidos da janela anterior primeiro, e
    /// descarta o que passar da capacidade.
    Janela Enviar(Conexao& c);

    /// O que os ACKs da janela fizeram com ela.
    enum class Desfecho { Dobrou, Cresceu, Cortou };

    /// O fim do RTT: chegaram os ACKs. Com perda, ACKs duplicados: ssthresh = cwnd / 2 e a janela
    /// cai para ele.
    Desfecho Reconhecer(Conexao& c, const Janela& j);

    /// O timeout: ssthresh = cwnd / 2, e cwnd volta a 1, no slow start.
    void Timeout(Conexao& c);

    /// O ruido do enlace: com chance 'chance', um segmento que o roteador aceitaria se perde no
    /// caminho - e o TCP nao sabe a diferenca, corta a janela do mesmo jeito. So na evitacao (a
    /// fase mostra o slow start inteiro: ali quem corta e o roteador cheio), numa janela de
    /// kMenorJanelaDoRuido ou mais, e nunca nos tres ultimos aceitos: o que se perde precisa de tres
    /// depois dele para os 3 ACKs duplicados. Devolve se perdeu.
    bool Ruido(Janela& j, const Conexao& c, Variacao::Sorteio& s, float chance);
    constexpr int kMenorJanelaDoRuido = 4;
    /// Onde o do ruido estoura: entre estas fracoes do caminho da nuvem ao roteador.
    constexpr float kMenorQuebra = 0.35f;
    constexpr float kMaiorQuebra = 0.75f;

    // ---- O relogio ----

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

    /// O RTT nao e sempre o mesmo: a nuvem processa o de sempre (e o aviso), mas descansa de
    /// (1 - kDesvioDoRtt) a (1 + kDesvioDoRtt) de kDescansa - nunca menos do que a 'janela' que
    /// acabou de sair leva para sair inteira: a nuvem nao chove enquanto processa.
    float Descanso(Variacao::Sorteio& s, int janela);
    constexpr float kDesvioDoRtt = 0.4f;

    /// O enlace de saida do roteador tambem nao tem ritmo fixo: cada encaminhado sai de
    /// (1 - kDesvioDoEnlace) a (1 + kDesvioDoEnlace) do passo do degrau depois do anterior.
    constexpr float kDesvioDoEnlace = 0.3f;

    /// O roteador encaminha um a cada kEntreEncaminhados - o enlace de saida.
    constexpr float kEntreEncaminhados = 0.06f;

    // ---- O campo ----

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    /// O roteador: no meio da largura, kAlturaDoRoteador abaixo do alto - bem abaixo do professor,
    /// que varre o alto do campo com as nuvens.
    Ponto Roteador(float largura, float fracao = 0.5f);
    constexpr float kAlturaDoRoteador = 400.0f;
    constexpr float kChegou = 14.0f;     ///< a distancia em que o segmento entra nele

    /// As velocidades sao as dos ataques em fases.json (um teste confere).
    constexpr float kVelocidadeDoSegmento = 320.0f;
    constexpr float kVelocidadeDoEncaminhado = 340.0f;

    /// Quem fica parado leva um, quem anda sempre igual leva o outro; quem muda de rumo no meio do
    /// caminho escapa dos dois. Metade do caminho nao servia.
    float Antecipacao(int i, bool retransmitido);

    /// Para onde mirar: o aluno em 'aluno', andando a 'velocidade', um segmento saindo de 'de' a
    /// 'rapidez' px/s, antecipando 'quanto' do caminho.
    Ponto Mira(Ponto de, Ponto aluno, Ponto velocidade, float rapidez, float quanto);
    constexpr float kVelocidadeDoDescarte = 220.0f;
    constexpr int kAnelDoDescarte = 8;

    /// O giro do anel do descarte no terco k: no 0 um caco vai no aluno, no 1 e no 2 ele fica a um e
    /// a dois tercos do espaco entre dois cacos - tres aneis em tercos diferentes fecham o circulo.
    /// O Vitor sorteia o terco de cada anel.
    float GiroDoDescarte(int k, int anel = kAnelDoDescarte);

    // ---- O grafico do cwnd ----

    /// A direita, logo abaixo da faixa das nuvens: o eixo do tempo em RTTs e o do cwnd, com a linha
    /// da capacidade.
    struct Retangulo { float x = 0.0f; float y = 0.0f; float w = 0.0f; float h = 0.0f; };
    Retangulo Grafico(float largura);
    constexpr float kTopoDoGrafico = 132.0f;
    constexpr float kMargemDaVarredura = 380.0f;
    constexpr float kLarguraDoGrafico = 250.0f;
    constexpr float kAlturaDoGrafico = 104.0f;
    constexpr int kRttsNoGrafico = 12;
    constexpr int kMaiorNoGrafico = 16;

    /// Onde o ponto (rtt, cwnd) cai no grafico. O rtt passa do ultimo: fica no ultimo; o cwnd passa
    /// do teto: fica no teto.
    Ponto NoGrafico(int rtt, int cwnd, float largura);

    // ---- Os textos ----

    /// O placar: "cwnd = 8 · slow start", "cwnd = 9 · evitação",
    /// "3 ACKs duplicados · cwnd = 8", "timeout · cwnd = 1".
    std::string Placar(const Conexao& c);
    std::string PlacarDaPerda(const Conexao& c);
    std::string PlacarDoTimeout(const Conexao& c);

    /// O roteador: quantos desta janela ele ja recebeu, contra a capacidade -
    /// "roteador · 7/10". Em 10/10, o que chega e descartado.
    std::string Fila(int recebidos, int capacidade = kCapacidade);

    // ---- O que muda de uma luta para a outra (Variacao.h) ----

    /// O dente de serra muda com os dois numeros. O grafico desenha a linha vermelha na capacidade
    /// sorteada.
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

    // ---- O degrau (Dificuldade.h) ----

    constexpr float kChanceDoRuido = 0.2f;

    /// O TCP e o mesmo.
    struct Degrau {
        float entreEncaminhados = kEntreEncaminhados;
        float rapidez = 1.0f;            ///< do encaminhado e do caco
        int anelDoDescarte = kAnelDoDescarte;
        int menosCapacidade = 0;         ///< o roteador da conta de menos: mais perdas, mais aneis
        float chanceDoRuido = kChanceDoRuido;   ///< de uma janela perder um segmento no caminho
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// A rede da luta no modo: a capacidade do plano menos a do degrau, nunca
    /// abaixo de kMenorCapacidade.
    Plano NoDegrau(Plano p, const Degrau& d);
    constexpr int kMenorCapacidade = 6;
}
