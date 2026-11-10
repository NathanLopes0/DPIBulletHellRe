//
// O registro da tentativa: os dados do TCC, anotados enquanto a luta acontece.
//

#pragma once

#include "OuvinteDaBatalha.h"
#include "../../Pesquisa/Tentativas.h"

/// Ouve a batalha (OuvinteDaBatalha) e monta a Tentativa (Tentativas.h), que vai para os CSV no fim
/// (TentativasArquivo).
class RegistroDaTentativa : public OuvinteDaBatalha {
public:
    void AoComecar(Battle& b) override;
    void AoPassarOTempo(Battle& b, float deltaTime, bool emPausa) override;
    void AoFimDoQuadro(Battle& b) override;
    void AoTrocarDeFase(Battle& b, const std::string& fase, const std::string& anuncio) override;
    void AoLimparFase(Battle& b) override;
    void AoSerAcertado(Battle& b, const Player& aluno, bool porUmTiro) override;
    void AoAcertarOProfessor(Battle& b, const Actor* quemAtirou) override;
    void AoAcertarDePerto(Battle& b, const Player& aluno) override;
    void AoRaspar(Battle& b, const Player& aluno) override;
    void AoCobrir(Battle& b, const Player& quemCobriu, const Player& parceiro) override;
    void AoIrNoAluno(Battle& b, const Player& alvo) override;
    void AoPegarPontoExtra(Battle& b, const Player& aluno) override;
    void AoUsarPontoExtra(Battle& b, const Player& aluno) override;
    void AoSincronizar(Battle& b) override;
    void AoSinalizar(Battle& b, const Player& aluno) override;
    void AoTocarAqui(Battle& b) override;
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
    /// Na dupla, a distancia entre os dois somada a cada quadro de luta, pesada pelo tempo: dividida
    /// por mRelogio, e a distancia media.
    float mDistanciaVezesTempo = 0.0f;
    /// Os toca aqui do resultado, contados para numerar as linhas de tentativas_toques.csv.
    int mToques = 0;
};
