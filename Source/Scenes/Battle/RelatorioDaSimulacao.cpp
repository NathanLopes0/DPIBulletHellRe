//
// O relatorio da simulacao: a nota a cada 60 quadros de luta, e o fim.
//

#include "RelatorioDaSimulacao.h"

#include <cstdio>
#include <cstdlib>

#include "Battle.h"

void RelatorioDaSimulacao::AoAntesDasColisoes(Battle& b) {
    if (++mQuadros % 60 != 0) return;
    std::printf("q%05d nota=%.4f sofridos=%d\n", mQuadros, static_cast<double>(b.NotaAtual()),
                b.AcertosSofridos());
    std::fflush(stdout);
}

void RelatorioDaSimulacao::AoAcertarOProfessor(Battle&, const Actor*, const bool ouviu) {
    if (ouviu) ++mAcertosDados;
}

void RelatorioDaSimulacao::AoDecidir(Battle& b, const bool aprovado) {
    std::printf("FIM nota=%.4f acertos_sofridos=%d acertos_dados=%d aprovado=%d\n",
                static_cast<double>(b.NotaAtual()), b.AcertosSofridos(), mAcertosDados, aprovado ? 1 : 0);
    std::fflush(stdout);
    // Antes de gravar: a medida e so da batalha, e seguir para o resultado gastaria tempo e
    // escreveria fichas e tentativas de teste na pasta de saves.
    if (mSairNoFim) std::exit(0);
}
