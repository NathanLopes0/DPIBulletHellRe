//
// A fase 1 do Luizinho: as amarracoes.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A que declaracao um nome se refere. O x e procurado no bloco em que e usado e, se ali nao
/// houver, no de fora, ate achar quem o declara - e quem responde e esse bloco, atirando o valor
/// dele. No escopo estatico, "o de fora" e o que envolve o bloco no texto; no dinamico, o do LISP
/// antigo, e quem chamou a funcao.
namespace Amarracao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDasAmarracoes = "StateOne";
    constexpr const char* kGatilhoDaResposta = "resposta";

    /// O programa no campo: o global, a funcao f com um bloco dentro dela, e a funcao g.
    enum class Bloco { Global, F, Interno, G };
    constexpr int kBlocos = 4;

    /// O bloco que envolve 'b' no texto. O global e pai de si mesmo.
    Bloco Pai(Bloco b);

    /// O valor de x que cada bloco declara (0: nao declara). O global e g sempre declaram; f nunca,
    /// e e por isso que a chamada de f muda com a regra; o bloco de dentro de f, as vezes.
    struct Programa {
        std::array<int, kBlocos> x{};
        [[nodiscard]] bool Declara(Bloco b) const;
        [[nodiscard]] int Valor(Bloco b) const;
    };

    /// O programa de uma luta: um valor diferente em cada bloco que declara x.
    constexpr int kMenorValor = 3;
    constexpr int kMaiorValor = 9;
    constexpr float kChanceDoInterno = 0.65f;
    Programa SortearPrograma(Variacao::Sorteio& s);

    enum class Regra { Estatico, Dinamico };

    /// Um atira(x): o bloco em que ele roda, a regra, e se e a chamada de f por g.
    struct Comando {
        Bloco onde = Bloco::G;
        Regra regra = Regra::Estatico;
        bool chamada = false;
    };

    /// O proximo atira(x) da fase: num bloco sorteado, nunca no mesmo do anterior.
    Comando Proximo(std::uint32_t sorteio, Bloco anterior);

    /// A do fim: g() chama f(), e roda o atira(x) de f, na regra dada.
    Comando Chamada(Regra r);

    /// A busca do x: os blocos olhados, do de dentro para fora, ate o que declara - o ultimo. No
    /// estatico, sobe pelo texto; no dinamico, de f vai para quem a chamou, g.
    std::vector<Bloco> Busca(const Programa& p, const Comando& c);
    Bloco Dono(const Programa& p, const Comando& c);

    /// O canto do painel de cada bloco: "int x = 4;  // global", "void f() {", "{ int x = 6;" (so
    /// "{" se ele nao declara) e "void g() { int x = 5;".
    std::string Declaracao(const Programa& p, Bloco b);

    /// O placar: a linha que roda e, achado o x, de onde ele veio na linha de baixo. Na chamada de f
    /// por g, a linha de baixo diz a regra, e depois o x que ela achou.
    std::string Placar(const Programa& p, const Comando& c, bool achou);

    // ---- O campo ----

    struct Retangulo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        [[nodiscard]] bool Contem(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
    };
    struct Ponto { float x = 0.0f; float y = 0.0f; };

    /// O painel de cada bloco num campo de 'largura' x 'altura', com f a esquerda ou a direita. O
    /// global e o campo inteiro; o bloco de dentro fica dentro de f.
    Retangulo Area(Bloco b, bool fAEsquerda, float largura, float altura);

    /// O meio do canto com a declaracao, de 'larguraDoCanto' de largura: e dali que o bloco
    /// responde. O do global fica no alto, do lado oposto ao placar.
    Ponto OndeFicaOCanto(Bloco b, bool fAEsquerda, float largura, float altura, float larguraDoCanto);
    constexpr float kAlturaDoCanto = 26.0f;

    /// Onde o professor roda o atira(x): dentro do bloco, e fora dos que estao dentro dele.
    Ponto OndeFicaOProfessor(Bloco b, bool fAEsquerda, float largura, float altura);

    // ---- A resposta ----

    /// O bloco responde atirando o valor de x: x tiros num leque mirado no aluno, kEntreTiros
    /// graus um do outro, em 'rajadas' rajadas - cada uma com um desvio sorteado de ate meio tiro,
    /// para o leque nao deixar vao parado onde o aluno esta.
    float DesvioDaRajada(float sorte);
    constexpr float kEntreTiros = 9.0f;
    float AberturaDoLeque(int tiros);
    constexpr float kVelocidade = 225.0f;

    // ---- O relogio ----

    /// Cada atira(x): o placar escreve e o professor vai ao bloco (kAntesDaBusca), a busca olha um
    /// bloco a cada kPorSalto, o dono acende e responde kAntesDaResposta depois, uma rajada a cada
    /// kEntreRajadas, e o proximo vem depois de uma pausa sorteada. A chamada de f por g, nas duas
    /// regras, comeca no primeiro comando que caberia depois de kInicioDaChamada.
    constexpr float kPrimeiro = 1.0f;
    constexpr float kAntesDaBusca = 0.55f;
    constexpr float kPorSalto = 0.27f;
    constexpr float kAntesDaResposta = 0.25f;
    constexpr int kRajadas = 3;
    constexpr float kEntreRajadas = 0.32f;
    constexpr float kMenorPausa = 0.3f;
    constexpr float kMaiorPausa = 0.6f;
    constexpr float kInicioDaChamada = 12.5f;

    /// Do comeco do comando a ultima rajada.
    float Duracao(const Programa& p, const Comando& c, float porSalto = kPorSalto, int rajadas = kRajadas);

    /// O que o Dificil muda: rajadas mais rapidas e a busca mais ligeira. O numero de rajadas e o
    /// mesmo - com o respiro curto do Dificil depois de um acerto, uma a mais derrubava quem nao
    /// desvia ainda na fase 1.
    struct Degrau {
        int rajadas = kRajadas;
        float velocidade = kVelocidade;
        float porSalto = kPorSalto;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
