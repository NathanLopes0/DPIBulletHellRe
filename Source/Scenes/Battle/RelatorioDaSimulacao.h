//
// O relatorio da simulacao: a nota a cada 60 quadros de luta, e o fim.
//

#pragma once

#include "OuvinteDaBatalha.h"

/// Ouve a batalha (OuvinteDaBatalha) e escreve, na saida padrao, o que a medicao compara entre duas
/// versoes do jogo (--relatorio, Simulacao.h):
///
///     q00060 nota=40.0000 sofridos=0
///     FIM nota=88.7911 acertos_sofridos=10 acertos_dados=508 aprovado=1
///
/// O texto e o mesmo do arnes que era remendado no codigo, letra por letra: as saidas gravadas antes
/// dele continuam comparaveis com as de agora (Ferramentas/linha-de-base.sh).
class RelatorioDaSimulacao : public OuvinteDaBatalha {
public:
    /// 'sairNoFim': fecha o jogo assim que a luta se decide (--sair-no-fim), antes de gravar nada.
    explicit RelatorioDaSimulacao(bool sairNoFim) : mSairNoFim(sairNoFim) {}

    void AoAntesDasColisoes(Battle& b) override;
    void AoAcertarOProfessor(Battle& b, const Actor* quemAtirou, bool ouviu) override;
    void AoDecidir(Battle& b, bool aprovado) override;

private:
    bool mSairNoFim;
    /// Quadros de luta: as vezes que as colisoes foram verificadas.
    int mQuadros = 0;
    /// Os tiros que o chefe ouviu - de aluno ou de orientador.
    int mAcertosDados = 0;
};
