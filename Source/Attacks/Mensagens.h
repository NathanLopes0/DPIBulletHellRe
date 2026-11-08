//
// A fase 3 do Vitor: o fork e a troca de mensagens.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 3 do Vitor e a programacao distribuida.
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

    // ---- As rodadas ----

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

    /// A rpc: as duas chamadas e o que elas devolvem.
    struct Chamada { int a = 0; int b = 0; };
    constexpr int kChamadas = 2;
    /// As chamadas de sempre: soma(2, 3) e soma(3, 4).
    Chamada ChamadaDaVez(int i);
    int Soma(const Chamada& c);

    struct Ponto { float x = 0.0f; float y = 0.0f; };

    // ---- O que muda de uma luta para a outra (Variacao.h) ----

    /// A fase desta batalha.
    struct Plano {
        std::array<Rodada, kRodadas> rodadas{};
        std::array<Chamada, kChamadas> chamadas{};
        std::array<std::array<Ponto, 2>, kRodadas> lugares{};   ///< x em fracao da largura, y em px
    };
    Plano PlanoDeSempre();
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kSomas[] = {5, 7, 9};

    // ---- O degrau (Dificuldade.h) ----

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
    std::vector<Evento> Roteiro(Rodada r, const Plano& p = PlanoDeSempre(), const Degrau& d = Degrau{});

    /// Quanto tempo depois de sair um envelope chega - e, no impasse, quando ele para na porta
    /// (kNaPorta do caminho).
    constexpr float kNaPorta = 0.88f;

    // ---- O relogio ----

    /// As rodadas comecam kComeca depois de a fase abrir: antes disso e o fork
    /// pela nuvem (Nuvem::Fork, que acaba em 1,6 s).
    constexpr float kComeca = 1.8f;
    constexpr float kPorRodada = (Fase::kDuracao - kComeca) / static_cast<float>(kRodadas);

    // ---- O campo ----

    /// Onde fica cada processo na rodada k: o cliente a esquerda, o servidor a direita, no alto,
    /// cada rodada num par de alturas.
    Ponto Lugar(int processo, int k, float largura, const Plano& p = PlanoDeSempre());

    /// O arco do canal: de 'de' a 'para', descendo ate 'fundo' no meio. s vai de 0 (em 'de') a 1
    /// (em 'para').
    Ponto NoArco(Ponto de, Ponto para, float s, float fundo);

    /// O fundo do arco em cada rodada: quanto ele desce pelo campo.
    float Fundo(Rodada r);

    // ---- A caixa postal ----

    /// Quantos envelopes ha na caixa de 'processo' em 'u': os que ja chegaram menos os que ele ja
    /// recebeu. Pelo roteiro, para o placar.
    int NaCaixa(const std::vector<Evento>& roteiro, int processo, float u);

    // ---- As velocidades ----

    /// As dos ataques em fases.json (um teste confere).
    constexpr float kVelocidadeDoProcessado = 320.0f;
    constexpr float kVelocidadeDoEncontro = 220.0f;
    constexpr int kAnelDoEncontro = 8;

    // ---- Os textos ----

    std::string Placar(Rodada r, int naCaixa, int chamada, bool bloqueado, bool estourou,
                       const Plano& p = PlanoDeSempre());
}
