//
// A fase 3 do Goulart: o escalonamento do disco.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "Fase.h"

/// A fase 3 do Goulart e a entrada e saida: o braco do disco. Os cilindros sao uma trilha no alto
/// do campo, os pedidos de leitura caem nela, e o professor e o braco - ele anda pela trilha e,
/// onde para, le o bloco, que desce na coluna do pedido ate o chao. Uma linha liga o braco ao
/// pedido que ele vai ler, e a coluna dele fica tracejada: quem entende o algoritmo da etapa sabe
/// onde nao ficar.
///   - FCFS: na ordem de chegada, e o braco vai e volta pela trilha inteira;
///   - SSTF: o mais perto primeiro - anda pouco, mas o pedido longe passa fome (fica vermelho), e o
///     bloco dele, quando enfim e lido, desce largo;
///   - SCAN, o elevador: vai num sentido ate a ponta atendendo o que encontra, e volta.
/// O placar compara o quanto o braco andou com o que o FCFS andaria nos mesmos pedidos.
namespace Disco {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateThree";
    enum class Algoritmo { FCFS, SSTF, SCAN };
    constexpr std::array<Algoritmo, 3> kOrdem = {Algoritmo::FCFS, Algoritmo::SSTF, Algoritmo::SCAN};
    constexpr std::array<const char*, 3> kEtapas = {"FCFS", "SSTF", "SCAN"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDoPedido = "requisicao";
    constexpr const char* kGatilhoDaLeitura = "leitura";

    const char* Nome(Algoritmo a);

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    constexpr int kCilindros = 200;

    /// O x do cilindro 'c' numa trilha que comeca em 'x' e tem 'largura', e o cilindro do x.
    float XDoCilindro(int c, float x, float largura);
    int CilindroDoX(float px, float x, float largura);

    /// O cilindro para onde o braco em 'cabeca' vai agora, pelo algoritmo, entre os 'pedidos'
    /// (na ordem de chegada). No SCAN, 'sentido' (+1 para o fim, -1 para o comeco) e o do braco:
    /// sem pedido adiante, o alvo e a ponta, e la o sentido vira. Sem pedido nenhum, -1.
    int Alvo(int cabeca, const std::vector<int>& pedidos, Algoritmo a, int& sentido);

    /// O quanto o braco anda, em cilindros, atendendo 'pedidos' (na ordem de chegada) a partir de
    /// 'cabeca'. No SCAN ele comeca no 'sentido' e vai ate a ponta antes de voltar.
    int Movimento(int cabeca, std::vector<int> pedidos, Algoritmo a, int sentido = -1);

    /// O placar: "SSTF: 236 cilindros em 9 pedidos  ·  em FCFS seriam 640" (no FCFS, sem a
    /// comparacao).
    std::string Placar(Algoritmo a, int andou, int atendidos, int fcfsSeria);

    /// O pedido que espera mais que isto passa fome: fica vermelho, e o bloco dele desce largo.
    constexpr float kFome = 2.6f;

    /// O braco le o pedido a menos disto (px) dele.
    constexpr float kAlcance = 8.0f;

    /// A trilha deixa esta margem (px) de cada lado do campo: o professor, que e o braco, nao chega
    /// a borda. E o elevador conta a ponta como alcancada a kPertoDaPonta cilindros dela.
    constexpr float kMargemDaTrilha = 105.0f;
    constexpr int kPertoDaPonta = 2;

    /// O que o Dificil muda: pedidos mais seguidos, e mais deles na trilha.
    struct Degrau {
        float entrePedidos = 0.55f;
        int maisPedidos = 7;
        float queda = 330.0f;   ///< a velocidade do bloco lido, descendo na coluna do pedido
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
