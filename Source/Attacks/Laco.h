//
// Os lacos da fase 2 do Andre.
//

#pragma once

#include "../Gameplay/Dificuldade.h"

#include <cstdint>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A FASE 2 DO ANDRE E A REPETICAO, e o chao do campo e um VETOR: doze casas,
 * v[0] a v[11], uma por faixa de 100 px, com o indice escrito embaixo. Cada
 * laco anda pelo vetor com o i dele - a casa da vez acende -, e cada iteracao
 * solta um cacho de baloes do BOCA que SOBE do chao naquela casa. O aluno le o
 * cabecalho do laco no placar (onde comeca, ate onde vai, de quanto em quanto)
 * e fica numa casa por onde o i nao passa.
 *
 *   for (i = 0; i < 12; i += 2)          as casas pares, da esquerda para a
 *                                        direita
 *   for (i = 11; i >= 0; i -= 3)         de tras para a frente
 *   for (i = 1; i < 12; i *= 2)          1, 2, 4, 8: o passo cresce
 *   for (...) if (i % 3 == 0) continue;  o i passa por todas, e as do
 *                                        continue ficam sem balao
 *   for (k...) for (i = k; ...; i += 3)  o laco aninhado: tres passadas, cada
 *                                        uma uma casa para o lado
 *   for (i = 0; i <= 12; i += 4)         o <= que passa do fim: v[12] nao
 *                                        existe, e o programa capota -
 *                                        "Segmentation fault (core dumped)" -
 *                                        e a memoria despejada voa da borda
 *   while (1) v[rand() % 12] = balao;    o laco que nao para: casas
 *                                        sorteadas, ate o juiz do BOCA matar
 *                                        o programa ("Time limit exceeded") e
 *                                        os baloes dele sumirem
 *
 * Quem desenha, move e solta e o Andre; aqui ficam os lacos, as visitas de
 * cada um no tempo, as casas e os textos. Coberto por test_laco.cpp.
 */
namespace Laco {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoLaco = "StateTwo";
    constexpr const char* kGatilhoDoIndice = "indice";
    constexpr const char* kGatilhoDoCacho = "subir";
    constexpr const char* kGatilhoDoDespejo = "core";

    /// O vetor: v[0] a v[kCasas - 1].
    constexpr int kCasas = 12;

    enum class Forma { Para, Decrescente, Multiplica, Continue, Aninhado, ForaDoVetor, Infinito };

    /// Um laco. Para: "for (i = inicio; i < 12; i += passo)". Decrescente: "i
    /// -= passo" a partir de inicio. Multiplica: "i *= passo". Continue: pula
    /// as casas com i % passo == 0. Aninhado: passo passadas, a k-esima
    /// comecando em k. ForaDoVetor: ate v[12] (direita) ou v[-1] (esquerda).
    struct Programa {
        Forma forma = Forma::Para;
        int inicio = 0;
        int passo = 1;
        bool direita = true;    ///< so no ForaDoVetor
    };

    /// O que acontece numa iteracao: a casa i acende, e depois sobe o cacho
    /// (Balao), ou nao sobe nada (Pulado, o continue), ou o i saiu do vetor
    /// (Fora: o segfault).
    enum class Tipo { Balao, Pulado, Fora };
    struct Visita {
        int i = 0;
        Tipo tipo = Tipo::Balao;
        float quando = 0.0f;     ///< segundos desde o comeco do laco
    };

    /// @brief As iteracoes do laco, em ordem, cada uma no seu instante. O
    /// while (1) sorteia as casas com 'sorteio'.
    std::vector<Visita> Execucao(const Programa& p, std::uint32_t sorteio);

    /// @brief Quanto o laco dura, do comeco ao fim: a ultima iteracao mais o
    /// capotamento (ForaDoVetor) ou o limite de tempo (Infinito).
    float Duracao(const Programa& p, const std::vector<Visita>& visitas);

    /// @brief O laco da vez: a forma segue a ordem da fase, e os numeros sao
    /// sorteados.
    Programa Proximo(int vez, std::uint32_t sorteio);

    /// @brief O cabecalho do laco no placar: "for (i = 0; i < 12; i += 2)".
    std::string Placar(const Programa& p);

    /// O que o placar diz quando o programa capota, e quando o juiz o mata.
    constexpr const char* kSegfault = "Segmentation fault (core dumped)";
    constexpr const char* kTempoEsgotado = "Time limit exceeded";

    /// @brief O meio da casa i num campo de 'largura'. Vale tambem para v[-1]
    /// e v[12], que ficam fora da tela.
    float XDaCasa(int i, float largura);

    /// @brief A animacao do indice da casa i: apagado, aceso (o i esta nela),
    /// pulado (o continue) e o do segfault.
    std::string CaraDoIndice(int i);
    std::string CaraDoIndiceAceso(int i);
    std::string CaraDoIndicePulado(int i);
    std::string CaraDoIndiceErro(int i);

    /// @brief A animacao de um pedaco da memoria despejada: um byte em hexa.
    constexpr int kBytes = 6;
    std::string CaraDoByte(int k);

    // ---- O RELOGIO E O DESENHO ---------------------------------------------

    /// O primeiro laco comeca kPrimeiro depois de a fase abrir; uma iteracao
    /// a cada kPasso; no laco aninhado, kEntrePassadas a mais entre uma
    /// passada e a outra; kEntreLacos entre um laco e o seguinte. Os sete
    /// cabem na fase (Fase::kDuracao), ate com os numeros mais compridos (ha
    /// teste); o tempo que sobra recomeca a ordem.
    constexpr float kPrimeiro = 0.3f;
    constexpr float kPasso = 0.2f;
    constexpr float kEntrePassadas = 0.3f;
    constexpr float kEntreLacos = 0.5f;
    constexpr int kFormas = 7;

    /// O cacho sobe kAviso depois de a casa acender, de kAbaixoDoChao abaixo
    /// da borda de baixo, na velocidade do ataque em fases.json (um teste
    /// confere): tres baloes, um no meio da casa e um de cada lado.
    constexpr float kAviso = 0.6f;
    constexpr float kAbaixoDoChao = 40.0f;
    constexpr float kVelocidadeDoCacho = 240.0f;
    constexpr int kBaloesPorCacho = 3;
    constexpr float kLadoDoCacho = 30.0f;

    /// O segfault: as casas ficam vermelhas por kCapotando, e so entao vem o
    /// laco seguinte. A memoria sai da borda num leque para dentro do campo,
    /// kAcimaDoChao acima da borda de baixo e um pouco para cima
    /// (kInclinacaoDoDespejo graus): metade dele nao se perderia por baixo.
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

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// Os lacos se revezam numa ORDEM DA LUTA (Variacao::Ordem de kFormas, com
    /// o for simples abrindo): a vez k da fase e Proximo(NaOrdem(ordem, k)).

    /// O que o Dificil muda: o cacho sobe mais cedo depois de a casa acender
    /// (menos aviso), mais depressa e mais largo - cinco baloes, de borda a
    /// borda da casa, e nao tres no meio -, e a memoria do segfault voa mais
    /// rapido.
    struct Degrau {
        float aviso = kAviso;
        float rapidez = 1.0f;          ///< do cacho e do despejo
        int baloesPorCacho = kBaloesPorCacho;
        float ladoDoCacho = kLadoDoCacho;   ///< de um balao ao do lado
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
