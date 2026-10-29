//
// O registro da tentativa: os dados do TCC, anotados enquanto a luta acontece.
//

#pragma once

#include "OuvinteDaBatalha.h"
#include "../../Tentativas.h"

/**
 * @class RegistroDaTentativa
 * @brief Ouve a batalha (OuvinteDaBatalha) e monta a Tentativa (Tentativas.h),
 * que vai para os CSV no fim (TentativasArquivo).
 *
 * TUDO E ANOTADO NO INSTANTE EM QUE ACONTECE - o acerto, a fase, o Ponto
 * Extra -, nunca reconstruido no fim. So a nota minima e maxima, os tiros e os
 * poderes sao lidos a cada quadro (AoFimDoQuadro).
 *
 * O REGISTRO SO COMECA QUANDO A LUTA COMECA (AoComecar): quem desiste na tela
 * de carregamento nao jogou. E grava UMA vez: o fim da batalha, a pausa e o
 * destrutor (a janela fechada) podem anunciar o fim, e so o primeiro conta.
 */
class RegistroDaTentativa : public OuvinteDaBatalha {
public:
    void AoComecar(Battle& b) override;
    void AoPassarOTempo(Battle& b, float deltaTime, bool emPausa) override;
    void AoFimDoQuadro(Battle& b) override;
    void AoTrocarDeFase(Battle& b, const std::string& fase, const std::string& anuncio) override;
    void AoLimparFase(Battle& b) override;
    void AoSerAcertado(Battle& b, const Player& aluno, bool porUmTiro) override;
    void AoAcertarOProfessor(Battle& b, const Actor* quemAtirou) override;
    void AoPegarPontoExtra(Battle& b, const Player& aluno) override;
    void AoUsarPontoExtra(Battle& b, const Player& aluno) override;
    void AoTerminar(Battle& b, Tentativas::Resultado resultado, bool ultimaFaseCompleta) override;

private:
    /// Anotando: depois de AoComecar e antes de gravar.
    [[nodiscard]] bool Anotando() const { return mComecou && !mGravada; }
    /// O registro do aluno 'quem', ou nulo (o orientador, um ator qualquer).
    Tentativas::Jogador* De(const Battle& b, const Actor* quem);

    Tentativas::Tentativa mTentativa;
    bool mComecou = false;
    bool mGravada = false;
    /// Segundos de luta, sem carregamento e sem pausa; e os de pausa.
    float mRelogio = 0.0f;
    float mRelogioDaPausa = 0.0f;
};
