//
// A computacao em nuvem do Vitor: processa, e chove pacote.
//

#pragma once

/// O Vitor da Programacao Concorrente e Distribuida e tambem de Redes, e na fase 2 dele ha nuvens
/// em cima dele - a computacao em nuvem, o servidor.
namespace Nuvem {

    /// O gatilho (FasesDeAtaque.h, GatilhoExiste) que forma as nuvens, no
    /// comeco da fase. Os segmentos que elas mandam sao de Tcp.h.
    constexpr const char* kGatilhoDasNuvens = "nuvens";

    /// O posto das nuvens (BossProjectile::DefinirPosto): e por ele que o
    /// Vitor as acha em campo. Tem de ser um dos nomes de PostoExiste.
    constexpr const char* kPostoDasNuvens = "nuvens";

    /// As duas caras da nuvem, que o Vitor troca conforme o ciclo. Tem de
    /// existir no tipo "Nuvem" de vitor/projeteis.json - um teste confere.
    constexpr const char* kCaraOciosa = "Ociosa";
    constexpr const char* kCaraProcessando = "Processando";

    /// O relogio: quanto a nuvem processa, e quanto descansa depois de mandar.
    struct Ciclo {
        float processa = 2.4f;
        float descansa = 0.8f;
        float t = 0.0f;
    };

    /// A nuvem fora do ar: o Ponto Extra limpa a tela - as nuvens junto -, e elas voltam
    /// kVoltaDasNuvens depois, com o ciclo do comeco.
    constexpr float kVoltaDasNuvens = 3.0f;

    /// A nuvem esta processando (e o placar e as luzinhas avisam)?
    bool Processando(const Ciclo& c);

    /// Um quadro. Devolve true no quadro em que o processamento acaba - e a hora de chover -, uma
    /// vez por volta, mesmo que um quadro longo pule o fim dele.
    bool Avancar(Ciclo& c, float deltaTime);

    /// As fases do Vitor que usam a nuvem: a das nuvens, e a seguinte, a do fork, que se divide por
    /// ela.
    constexpr const char* kFaseDasNuvens = "StateTwo";
    constexpr const char* kFaseDoFork = "StateThree";

    /// O fork pela nuvem, contado do comeco da fase 3: em 'entra' a nuvem chegou em cima dele e ele
    /// some nela; em 'abre' ela abre e os dois corpos aparecem.
    struct Fork {
        float entra = 0.4f;
        float abre = 1.0f;
        float sai = 1.6f;
    };

    /// O corpo aparece no instante t? O original esta a vista ate entrar na
    /// nuvem; o outro nasce dentro dela, e so aparece quando ela abre.
    bool Visivel(const Fork& f, float t, bool nasceuDentro);

    /// Quanto do caminho da nuvem ate o lugar dele o corpo ja andou: 0 ate a nuvem abrir, 1 quando
    /// ela se desfaz, e sem solavanco nas duas pontas.
    float Saida(const Fork& f, float t);
}
