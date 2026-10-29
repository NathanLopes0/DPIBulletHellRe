//
// Quem quer saber do que acontece na batalha.
//

#pragma once

#include <string>

#include "../../Data/Tentativas.h"

class Actor;
class Battle;
class Player;

/**
 * @class OuvinteDaBatalha
 * @brief O que acontece na batalha, anunciado a quem quiser ouvir (padrao
 * OBSERVER).
 *
 * POR QUE OUVINTES. A batalha e quem sabe quando um aluno foi acertado, quando
 * a fase troca, quando um Ponto Extra e pego - e varias coisas querem saber
 * disso: o registro da tentativa (os dados do TCC) e o clarao da moldura do
 * campo. Antes cada uma era escrita DENTRO das regras da batalha, linha por
 * linha (o "++acertosSofridos" do registro e o "Acender" da moldura no meio
 * do laco de colisao). Agora a regra so anuncia (Battle::Avisar), e cada
 * interessado e um ouvinte com o seu estado: dar a batalha um interessado
 * novo nao mexe em regra nenhuma.
 *
 * TODO METODO TEM CORPO VAZIO: cada ouvinte escreve so o que lhe interessa.
 * Os ouvintes sao avisados na ordem em que foram registrados.
 */
class OuvinteDaBatalha {
public:
    virtual ~OuvinteDaBatalha() = default;

    /// A luta comecou: a tela de carregamento sumiu e o chefe vai atacar.
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
    virtual void AoUsarPontoExtra(Battle&, const Player& aluno) {}

    /// O aluno atravessou a parede pela rede do socket() (Battle::AbrirRede).
    virtual void AoAtravessarAParede(Battle&) {}

    /// A tentativa acabou. 'ultimaFaseCompleta': a ultima fase acabou pelo
    /// relogio (e nao pela nota a zero ou por desistencia). Pode vir do
    /// destrutor da batalha: quem ouve nao deve tocar em ator nenhum aqui.
    virtual void AoTerminar(Battle&, Tentativas::Resultado resultado, bool ultimaFaseCompleta) {}
};
