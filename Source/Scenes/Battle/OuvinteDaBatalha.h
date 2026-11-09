//
// Quem quer saber do que acontece na batalha.
//

#pragma once

#include <string>

#include "../../Pesquisa/Tentativas.h"

class Actor;
class Battle;
class Player;

/// O que acontece na batalha, anunciado a quem quiser ouvir (padrao observer). Por que ouvintes.
class OuvinteDaBatalha {
public:
    virtual ~OuvinteDaBatalha() = default;

    virtual void AoComecar(Battle&) {}

    /// Passou um quadro: de luta, ou com a pausa aberta.
    virtual void AoPassarOTempo(Battle&, float deltaTime, bool emPausa) {}

    /// O quadro de luta terminou: as colisoes ja pagaram e cobraram.
    virtual void AoFimDoQuadro(Battle&) {}

    /// O chefe entrou na fase 'fase' ("StateOne"...), que o placar anuncia
    /// como 'anuncio'. Tambem ao entrar na primeira.
    virtual void AoTrocarDeFase(Battle&, const std::string& fase, const std::string& anuncio) {}

    /// A fase que acabou foi limpa: o bonus foi pago.
    virtual void AoLimparFase(Battle&) {}

    /// O aluno foi acertado - por um tiro, ou encostando no chefe.
    virtual void AoSerAcertado(Battle&, const Player& aluno, bool porUmTiro) {}

    /// Um tiro de 'quemAtirou' (um aluno ou o orientador) acertou um corpo do
    /// chefe - pagando a nota ou nao (o escudo).
    virtual void AoAcertarOProfessor(Battle&, const Actor* quemAtirou) {}

    virtual void AoPegarPontoExtra(Battle&, const Player& aluno) {}
    /// Na dupla, o tiro de 'quemCobriu' derrubou um projetil que ia em 'parceiro' (Cobertura.h).
    virtual void AoCobrir(Battle&, const Player& quemCobriu, const Player& parceiro) {}
    virtual void AoUsarPontoExtra(Battle&, const Player& aluno) {}

    /// O aluno atravessou a parede pela rede do socket() (Battle::AbrirRede).
    virtual void AoAtravessarAParede(Battle&) {}

    /// A tentativa acabou. 'ultimaFaseCompleta': a ultima fase acabou pelo relogio (e nao pela nota
    /// a zero ou por desistencia).
    virtual void AoTerminar(Battle&, Tentativas::Resultado resultado, bool ultimaFaseCompleta) {}
};
