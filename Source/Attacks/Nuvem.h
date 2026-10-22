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
 * E E PELA NUVEM QUE ELE SE DIVIDE na fase 3: as duas nuvens se juntam em cima
 * dele, ele entra, e dela saem dois - os dois processos de um fork.
 *
 * Quem faz isso no jogo e o Vitor (os gatilhos, a cara das nuvens, o placar, os
 * corpos); aqui ficam os relogios, cobertos por test_nuvem.cpp.
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

    /// As fases do Vitor que usam a nuvem: a das nuvens, e a seguinte, a do
    /// fork, que se divide por ela. Um teste confere em vitor/fases.json que a
    /// do fork vem logo depois da das nuvens e tem clone - se nao, o Vitor
    /// desfaz as nuvens no fim da fase e o fork volta a ser o de sempre.
    constexpr const char* kFaseDasNuvens = "StateTwo";
    constexpr const char* kFaseDoFork = "StateThree";

    /// O FORK PELA NUVEM, contado do comeco da fase 3: em 'entra' a nuvem
    /// chegou em cima dele e ele some nela; em 'abre' ela abre e os dois corpos
    /// aparecem; em 'sai' eles chegaram cada um ao seu lugar, e a nuvem se
    /// desfaz. Ate la nenhum dos dois anda sozinho nem ataca.
    struct Fork {
        float entra = 0.4f;
        float abre = 1.0f;
        float sai = 1.6f;
    };

    /// O corpo aparece no instante t? O original esta a vista ate entrar na
    /// nuvem; o outro nasce dentro dela, e so aparece quando ela abre.
    bool Visivel(const Fork& f, float t, bool nasceuDentro);

    /// Quanto do caminho da nuvem ate o lugar dele o corpo ja andou: 0 ate a
    /// nuvem abrir, 1 quando ela se desfaz, e sem solavanco nas duas pontas -
    /// ele sai devagar, anda, e chega devagar.
    float Saida(const Fork& f, float t);
}
