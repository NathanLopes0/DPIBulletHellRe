//
// Os lacos da fase 2 do Andre.
//

#pragma once

#include "../Regras/Dificuldade.h"

#include <cstdint>
#include <string>
#include <vector>

/// A fase 2 do Andre e a repeticao, e o chao do campo e um vetor: doze casas, v[0] a v[11], uma por
/// faixa de 100 px, com o indice escrito embaixo.
namespace Laco {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoLaco = "StateTwo";
    constexpr const char* kGatilhoDoIndice = "indice";
    constexpr const char* kGatilhoDoCacho = "subir";
    constexpr const char* kGatilhoDoDespejo = "core";

    /// O vetor: v[0] a v[kCasas - 1].
    constexpr int kCasas = 12;

    enum class Forma { Para, Decrescente, Multiplica, Continue, Aninhado, ForaDoVetor, Infinito };

    /// Um laco. Para: "for (i = inicio; i < 12; i += passo)".
    struct Programa {
        Forma forma = Forma::Para;
        int inicio = 0;
        int passo = 1;
        bool direita = true;    ///< so no ForaDoVetor
    };

    /// O que acontece numa iteracao: a casa i acende, e depois sobe o cacho (Balao), ou nao sobe
    /// nada (Pulado, o continue), ou o i saiu do vetor (Fora.
    enum class Tipo { Balao, Pulado, Fora };
    struct Visita {
        int i = 0;
        Tipo tipo = Tipo::Balao;
        float quando = 0.0f;     ///< segundos desde o comeco do laco
    };

    /// As iteracoes do laco, em ordem, cada uma no seu instante. O while (1) sorteia as casas com
    /// 'sorteio'.
    std::vector<Visita> Execucao(const Programa& p, std::uint32_t sorteio);

    /// Quanto o laco dura, do comeco ao fim: a ultima iteracao mais o capotamento (ForaDoVetor) ou
    /// o limite de tempo (Infinito).
    float Duracao(const Programa& p, const std::vector<Visita>& visitas);

    /// O laco da vez: a forma segue a ordem da fase, e os numeros sao sorteados.
    Programa Proximo(int vez, std::uint32_t sorteio);

    /// O cabecalho do laco no placar: "for (i = 0; i < 12; i += 2)".
    std::string Placar(const Programa& p);

    /// O que o placar diz quando o programa capota, e quando o juiz o mata.
    constexpr const char* kSegfault = "Segmentation fault (core dumped)";
    constexpr const char* kTempoEsgotado = "Time limit exceeded";

    /// O meio da casa i num campo de 'largura'. Vale tambem para v[-1] e v[12], que ficam fora da
    /// tela.
    float XDaCasa(int i, float largura);

    /// A animacao do indice da casa i: apagado, aceso (o i esta nela), pulado (o continue) e o do
    /// segfault.
    std::string CaraDoIndice(int i);
    std::string CaraDoIndiceAceso(int i);
    std::string CaraDoIndicePulado(int i);
    std::string CaraDoIndiceErro(int i);

    /// A animacao de um pedaco da memoria despejada: um byte em hexa.
    constexpr int kBytes = 6;
    std::string CaraDoByte(int k);

    // ---- O relogio e o desenho ----

    /// O primeiro laco comeca kPrimeiro depois de a fase abrir; uma iteracao a cada kPasso.
    constexpr float kPrimeiro = 0.3f;
    constexpr float kPasso = 0.2f;
    constexpr float kEntrePassadas = 0.3f;
    constexpr float kEntreLacos = 0.5f;
    constexpr int kFormas = 7;

    /// O cacho sobe kAviso depois de a casa acender, de kAbaixoDoChao abaixo da borda de baixo, na
    /// velocidade do ataque em fases.json (um teste confere).
    constexpr float kAviso = 0.6f;
    constexpr float kAbaixoDoChao = 40.0f;
    constexpr float kVelocidadeDoCacho = 240.0f;
    constexpr int kBaloesPorCacho = 3;
    constexpr float kLadoDoCacho = 30.0f;

    /// O segfault: as casas ficam vermelhas por kCapotando, e so entao vem o laco seguinte.
    constexpr float kCapotando = 0.8f;
    constexpr float kAcimaDoChao = 70.0f;
    constexpr float kInclinacaoDoDespejo = 15.0f;

    /// O while (1): uma casa a cada kPassoInfinito, ate kLimiteDeTempo.
    constexpr float kPassoInfinito = 0.25f;
    constexpr float kLimiteDeTempo = 2.0f;

    /// Os indices ficam embaixo, na faixa abaixo do chao do aluno: o meio
    /// deles kAcimaDaBorda acima da borda de baixo do campo.
    constexpr float kAcimaDaBorda = 20.0f;

    /// O professor: nesta altura, num x sorteado a cada laco.
    constexpr float kYDoProfessor = 170.0f;
    constexpr float kXMinimo = 300.0f;
    constexpr float kXMaximo = 900.0f;

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// Os lacos se revezam numa ordem da luta (Variacao::Ordem de kFormas, com o for simples
    /// abrindo): a vez k da fase e Proximo(NaOrdem(ordem, k)).

    /// O que o Dificil muda: o cacho sobe mais cedo depois de a casa acender (menos aviso), mais
    /// depressa e mais largo.
    struct Degrau {
        float aviso = kAviso;
        float rapidez = 1.0f;          ///< do cacho e do despejo
        int baloesPorCacho = kBaloesPorCacho;
        float ladoDoCacho = kLadoDoCacho;   ///< de um balao ao do lado
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
