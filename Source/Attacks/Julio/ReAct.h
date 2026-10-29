//
// O ciclo de um agente de IA: pensa, age, observa.
//

#pragma once

#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA.
 *
 * Na fase 2 do Julio o professor convoca AGENTES - robos que se espalham pelo
 * campo e agem por ele. Um agente de LLM trabalha num ciclo, o ReAct: PENSA
 * (raciocina sobre o que fazer), AGE (chama uma ferramenta) e OBSERVA (le o
 * resultado), e de novo. O placar diz em que passo os agentes estao, e e no
 * AGE que todos mandam a ferramenta ao mesmo tempo - o ciclo e o aviso.
 *
 * Quem faz isso no jogo e o Julio (o placar e o gatilho, Boss::Disparar); aqui
 * fica o relogio, coberto por test_react.cpp.
 */
namespace ReAct {

    /// O gatilho que o AGE puxa (FasesDeAtaque.h, GatilhoExiste).
    constexpr const char* kGatilhoDeAgir = "agir";

    /// O posto dos agentes (BossProjectile::DefinirPosto), de onde as
    /// ferramentas saem. Tem de ser um dos nomes de PostoExiste.
    constexpr const char* kPostoDosAgentes = "agentes";

    enum class Passo { Pensa, Age, Observa };

    /// O relogio: quanto dura cada passo, e onde ele esta.
    constexpr float kPensa = 1.0f;
    constexpr float kAge = 0.5f;
    constexpr float kObserva = 0.6f;
    struct Ciclo {
        float pensa = kPensa;
        float age = kAge;
        float observa = kObserva;
        float t = 0.0f;
    };

    /// O passo em que o relogio esta.
    Passo PassoDe(const Ciclo& c);

    /// Um quadro. Devolve true no quadro em que o ciclo ENTRA no Age - uma vez
    /// por volta, mesmo que um quadro muito longo pule o passo inteiro.
    bool Avancar(Ciclo& c, float deltaTime);

    // ---- O QUE MUDA DE UMA VOLTA PARA A OUTRA, E O DEGRAU -------------------

    /// O PENSA DE CADA VOLTA E SORTEADO, de 70% a 130% do do degrau: o agente
    /// pensa mais ou menos a cada vez, e quem diz quando ele age e o placar - o
    /// ritmo nao se decora.
    float PensaDaVolta(Variacao::Sorteio& s, float pensa);

    /// O que o Dificil muda: o agente pensa 0,8 s em media (e nao 1,0) e
    /// observa 0,5 s (e nao 0,6) - uma ferramenta a cada 1,8 s, e nao 2,1.
    struct Degrau {
        float pensa = kPensa;
        float observa = kObserva;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    // ---- PARA ONDE OS AGENTES VAO --------------------------------------------

    /// A CADA VOLTA OS AGENTES REPLANEJAM: no comeco do Pensa cada um ganha um
    /// lugar novo e voa para la enquanto pensa, para agir de outro ponto. Os
    /// lugares sao na faixa do meio do campo, ate kAlcance do aluno para cada
    /// lado, a kMargem das bordas e a kSeparacao uns dos outros. Antes iam a
    /// 260 px do professor num leque fixo e ficavam la: com o professor perto
    /// de uma borda, o agente da ponta parava fora da tela, onde o tiro do
    /// aluno nao chega.
    constexpr float kYMinimo = 170.0f;
    constexpr float kYMaximo = 360.0f;
    constexpr float kMargem = 90.0f;
    constexpr float kAlcance = 360.0f;
    constexpr float kSeparacao = 150.0f;

    /// Quanto o agente fica em campo, e quao depressa ele chega ao lugar
    /// (Ancora::Aproximar).
    constexpr float kVidaDoAgente = 10.0f;
    constexpr float kRapidezDoAgente = 5.0f;

    struct Ponto {
        float x = 0.0f;
        float y = 0.0f;
    };

    /// @brief Os 'n' lugares desta volta, para um aluno em 'xDoAluno' num campo
    /// de 'largura'. Sorteia candidatos e fica com o primeiro que respeita a
    /// separacao; se nenhum respeitar, com o mais afastado dos ja escolhidos.
    std::vector<Ponto> Destinos(Variacao::Sorteio& s, float xDoAluno, int n, float largura);
}

