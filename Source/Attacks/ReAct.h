//
// O ciclo de um agente de IA: pensa, age, observa.
//

#pragma once

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
    struct Ciclo {
        float pensa = 1.0f;
        float age = 0.5f;
        float observa = 0.6f;
        float t = 0.0f;
    };

    /// O passo em que o relogio esta.
    Passo PassoDe(const Ciclo& c);

    /// Um quadro. Devolve true no quadro em que o ciclo ENTRA no Age - uma vez
    /// por volta, mesmo que um quadro muito longo pule o passo inteiro.
    bool Avancar(Ciclo& c, float deltaTime);
}
