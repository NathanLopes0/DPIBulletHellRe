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

    /// O quadro de luta vai cobrar e pagar as colisoes (Battle::VerificarColisoes).
    virtual void AoAntesDasColisoes(Battle&) {}

    /// O quadro de luta terminou: as colisoes ja pagaram e cobraram.
    virtual void AoFimDoQuadro(Battle&) {}

    /// O chefe entrou na fase 'fase' ("StateOne"...), que o placar anuncia
    /// como 'anuncio'. Tambem ao entrar na primeira.
    virtual void AoTrocarDeFase(Battle&, const std::string& fase, const std::string& anuncio) {}

    /// A fase que acabou foi limpa: o bonus foi pago.
    virtual void AoLimparFase(Battle&) {}

    /// O aluno perdeu 'perda' de nota num acerto sofrido, por 'causa': o tipo do projetil (o nome em
    /// projeteis.json), ou o esbarrao (Devolutiva::kEsbarrao). Vem logo antes do AoSerAcertado.
    virtual void AoPerderNota(Battle&, const Player& aluno, const std::string& causa, float perda) {}

    /// O aluno foi acertado - por um tiro, ou encostando no chefe.
    virtual void AoSerAcertado(Battle&, const Player& aluno, bool porUmTiro) {}

    /// Um tiro de 'quemAtirou' (um aluno ou o orientador) acertou um corpo do chefe. 'ouviu': o chefe
    /// aceitou a pergunta, e ela pagou a nota - o escudo nao ouve.
    virtual void AoAcertarOProfessor(Battle&, const Actor* quemAtirou, bool ouviu) {}

    /// O tiro do aluno acertou o professor de perto - o "!!" (Nota::kFatorDaRespostaDupla). Conta
    /// pela distancia, mesmo que a condicao do experimento nao pague o perto.
    virtual void AoAcertarDePerto(Battle&, const Player& aluno) {}

    /// Um projetil passou rente ao aluno sem acerta-lo (Raspao.h). Conta mesmo que a condicao nao
    /// pague o raspao.
    virtual void AoRaspar(Battle&, const Player& aluno) {}

    /// Na dupla, um projetil destrutivel passou a ir no aluno 'alvo' (Cobertura::AlunoNoCaminho):
    /// para o parceiro, uma chance de cobri-lo. Uma vez por projetil, brilhe ele ou nao.
    virtual void AoIrNoAluno(Battle&, const Player& alvo) {}

    virtual void AoPegarPontoExtra(Battle&, const Player& aluno) {}
    /// Na dupla, o tiro de 'quemCobriu' derrubou um projetil que ia em 'parceiro' (Cobertura.h). Conta
    /// mesmo que a condicao do experimento nao pague a cobertura.
    virtual void AoCobrir(Battle&, const Player& quemCobriu, const Player& parceiro) {}
    virtual void AoUsarPontoExtra(Battle&, const Player& aluno) {}
    /// Na dupla, os dois gastaram o Ponto Extra juntos (PontoExtra::EmSincronia). Conta mesmo que a
    /// condicao do experimento nao solte a onda.
    virtual void AoSincronizar(Battle&) {}

    /// Na dupla, o aluno deu um sinal ao parceiro (Botao 3, Comunicacao.h). Conta mesmo que a
    /// condicao do experimento nao mostre o sinal.
    virtual void AoSinalizar(Battle&, const Player& aluno) {}

    /// No resultado da dupla aprovada, os dois apertaram o Botao 3 juntos: o toca aqui
    /// (Comunicacao::Juntos). Vem depois do AoTerminar - a tentativa ja foi gravada. Conta mesmo que a
    /// condicao do experimento nao mostre o toque.
    virtual void AoTocarAqui(Battle&) {}

    /// A luta acabou de se decidir, aprovada ou nao - antes da ultima fase pagar o bonus e de qualquer
    /// coisa ser gravada.
    virtual void AoDecidir(Battle&, bool aprovado) {}

    /// A tentativa acabou. 'ultimaFaseCompleta': a ultima fase acabou pelo relogio (e nao pela nota
    /// a zero ou por desistencia).
    virtual void AoTerminar(Battle&, Tentativas::Resultado resultado, bool ultimaFaseCompleta) {}
};
