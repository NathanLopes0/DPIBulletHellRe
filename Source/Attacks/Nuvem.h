//
// A computacao em nuvem do Vitor: processa, e chove pacote.
//

#pragma once

/**
 * CAMADA PURA.
 *
 * O Vitor da Programacao Concorrente e Distribuida e tambem de Redes, e na fase
 * 2 dele ha NUVENS em cima dele - a computacao em nuvem. Elas trabalham num
 * ciclo: PROCESSAM (as luzinhas acendem, e o placar avisa) e, quando acabam,
 * mandam o resultado para baixo - uma chuva de PACOTES de rede. O
 * processamento e o aviso: quem le a nuvem sabe quando vai chover.
 *
 * Quem faz isso no jogo e o Vitor (os gatilhos, a cara das nuvens, o placar);
 * aqui fica o relogio, coberto por test_nuvem.cpp.
 */
namespace Nuvem {

    /// Os gatilhos (FasesDeAtaque.h, GatilhoExiste): o que forma as nuvens, no
    /// comeco da fase, e o que faz chover.
    constexpr const char* kGatilhoDasNuvens = "nuvens";
    constexpr const char* kGatilhoDaChuva = "chover";

    /// O posto das nuvens (BossProjectile::DefinirPosto), de onde os pacotes
    /// saem. Tem de ser um dos nomes de PostoExiste.
    constexpr const char* kPostoDasNuvens = "nuvens";

    /// As duas caras da nuvem, que o Vitor troca conforme o ciclo. Tem de
    /// existir no tipo "Nuvem" de vitor/projeteis.json - um teste confere.
    constexpr const char* kCaraOciosa = "Ociosa";
    constexpr const char* kCaraProcessando = "Processando";

    /// O relogio: quanto a nuvem processa, e quanto descansa depois de chover.
    struct Ciclo {
        float processa = 2.4f;
        float descansa = 0.8f;
        float t = 0.0f;
    };

    /// A nuvem esta processando (e o placar e as luzinhas avisam)?
    bool Processando(const Ciclo& c);

    /// Um quadro. Devolve true no quadro em que o processamento ACABA - e a
    /// hora de chover -, uma vez por volta, mesmo que um quadro longo pule o
    /// fim dele.
    bool Avancar(Ciclo& c, float deltaTime);
}
