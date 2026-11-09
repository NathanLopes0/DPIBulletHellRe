//
// A fase 3 do Luizinho: o programa Termina.
//

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"

/// Suponha que exista um programa, o Termina, que olha outro programa e a entrada dele e diz se ele
/// para. Para lacos simples ele acerta - e o professor mostra. Mas entao da para escrever o
/// Paradoxo, que pergunta ao Termina sobre ele mesmo e faz o contrario da resposta: se o Termina
/// diz que ele para, ele repete para sempre; se diz que repete, ele para. Nao ha resposta certa, e
/// o Termina nao pode existir.
namespace Termina {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoTermina = "StateThree";
    constexpr const char* kGatilhoDaVolta = "volta";
    constexpr const char* kGatilhoDaContradicao = "contradicao";

    /// Os lacos que o professor roda, cada um com uma entrada n.
    enum class Programa {
        Contagem,       ///< enquanto (n > 0) n = n - 1;   sempre para
        DeDoisEmDois,   ///< enquanto (n != 0) n = n - 2;  para se n for par
        Metade,         ///< enquanto (n > 1) n = n / 2;   sempre para
        Crescente,      ///< enquanto (n > 0) n = n + 1;   nunca para
        DeTresEmTres    ///< enquanto (n != 10) n = n + 3; para se der 10 no caminho
    };
    constexpr int kProgramas = 5;

    /// O laco, numa linha: "enquanto (n != 0) n = n - 2;".
    std::string Codigo(Programa p);

    /// Uma volta do laco: o n seguinte, se a condicao ainda vale; nada, se o laco acabou.
    std::optional<int> Passo(Programa p, int n);

    /// Se o laco para, pela conta - para estes, da para saber sem rodar.
    bool Para(Programa p, int n);

    /// Os n que o contador mostra, volta a volta, ate o laco parar ou ate 'limite' voltas.
    std::vector<int> Valores(Programa p, int n, int limite);

    struct Rodada {
        Programa programa = Programa::Contagem;
        int n = 5;
    };

    /// A proxima rodada: um programa sorteado, nunca o da anterior, e uma entrada da faixa dele - a
    /// que decide, nos que dependem dela, se ele para.
    Rodada Proxima(std::uint32_t sorteio, int anterior = -1);

    /// O placar: o laco e, embaixo, a pergunta ao Termina - "Termina(P, 7) = ?" ate ele responder.
    std::string Placar(const Rodada& r, std::optional<bool> veredito);

    /// A caixa do Termina no campo: "Termina(P, 7)", e a resposta dele.
    std::string Pergunta(const Rodada& r);
    std::string Resposta(std::optional<bool> veredito);

    // ---- O Paradoxo ----

    /// O placar do Paradoxo: o programa e, embaixo, o que ele faz com a resposta da vez - ou, no
    /// fim, que o Termina nao pode existir.
    std::string PlacarDoParadoxo(std::optional<bool> veredito, bool contradicao);

    /// O Termina responde, o Paradoxo faz o contrario, o Termina muda de ideia... kViradas vezes,
    /// cada uma mais depressa, e entao ele se contradiz.
    constexpr int kViradas = 6;
    float DuracaoDaVirada(int k);
    float DuracaoDoParadoxo();

    /// A resposta da virada k: ele comeca dizendo que o Paradoxo para (e o Paradoxo repete).
    bool VereditoDaVirada(int k);

    // ---- A espiral ----

    /// Cada volta do laco e um anel de 'bracos' tiros saindo do professor, girado kGiroPorVolta a
    /// mais que o da volta anterior: o laco que roda desenha uma espiral.
    constexpr int kBracos = 15;
    constexpr float kGiroPorVolta = 14.0f;
    constexpr float kVelocidade = 180.0f;

    /// O anel da contradicao: o Termina estoura.
    constexpr int kCacosDaContradicao = 18;

    // ---- O relogio ----

    /// Cada rodada: o placar escreve, o Termina pensa (kPensando) e responde, o laco da uma volta a
    /// cada kPorVolta - e, se nao para, roda de kMenorCtrlC a kMaiorCtrlC ate o professor apertar
    /// Ctrl+C. O Paradoxo comeca na primeira rodada depois de kInicioDoParadoxo; depois dele, o
    /// laco infinito roda ate a fase acabar.
    constexpr float kPrimeira = 1.0f;
    constexpr float kPensando = 1.0f;
    constexpr float kPorVolta = 0.19f;
    constexpr float kMenorCtrlC = 2.6f;
    constexpr float kMaiorCtrlC = 3.4f;
    constexpr float kDepois = 0.4f;
    constexpr float kInicioDoParadoxo = 8.5f;
    constexpr int kMaisVoltas = 64;

    /// O que o Dificil muda: anel mais cheio, voltas mais seguidas e tiros mais rapidos.
    struct Degrau {
        int bracos = kBracos;
        float porVolta = kPorVolta;
        float velocidade = kVelocidade;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
