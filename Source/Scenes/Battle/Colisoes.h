//
// As colisoes de um quadro de luta: quem encosta em quem, e o que isso derruba.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../Motor/Math.h"

class Boss;
class BossProjectile;
class CircleColliderComponent;
class EfeitosDaBatalha;
class ExtraPointItem;
class Player;
class PlayerProjectile;

/// As regras de contato da batalha, na ordem em que valem - cada uma ve o que a anterior ja
/// derrubou: o abrigo e o escudo comem projeteis; o projetil do chefe acerta o aluno, ou passa
/// rente; o tiro do aluno derruba projeteis e acerta o professor; o aluno esbarra no professor e
/// pega o Ponto Extra.
///
/// Aqui mora a fisica: quem encosta em quem, o que morre, quem fica invencivel, a faisca de cada
/// coisa. O que cada contato vale - a nota, a devolutiva, a dupla, os ouvintes - e da batalha, que
/// fica sabendo pela interface Regras, na hora e na mesma ordem em que acontece.
class Colisoes {
public:
    /// Quem paga e cobra o que as colisoes acham: a batalha.
    class Regras {
    public:
        virtual ~Regras() = default;

        /// O aluno foi acertado: por um projetil (a 'causa' e o tipo dele, e 'multiplicador' a perda
        /// dele) ou esbarrando no professor (Devolutiva::kEsbarrao, 'porUmTiro' falso).
        virtual void AlunoAcertado(Player& aluno, const std::string& causa, float multiplicador, bool porUmTiro) = 0;

        /// Um projetil passou rente ao aluno sem acerta-lo, a primeira vez (Raspao.h). 'onde': a
        /// borda do aluno, do lado do projetil.
        virtual void AlunoRaspou(Player& aluno, const Vector2& onde) = 0;

        /// O tiro de um aluno derrubou um projetil do chefe. Devolve se foi uma cobertura (Cobertura.h),
        /// que tem a faisca dela no lugar da derrubada.
        virtual bool ProjetilDerrubado(const PlayerProjectile& tiro, const BossProjectile& projetil) = 0;

        /// Um tiro acertou um corpo do professor. 'ouviu': o chefe aceitou a pergunta - o escudo nao
        /// aceita.
        virtual void ProfessorAcertado(const PlayerProjectile& tiro, const CircleColliderComponent& corpo,
                                       bool ouviu) = 0;

        /// O aluno pegou um Ponto Extra. 'primeiroDoQuadro': o primeiro deste quadro (o som toca um so).
        virtual void PontoExtraPego(Player& aluno, bool primeiroDoQuadro) = 0;
    };

    /// Os vetores sao os da batalha: valem enquanto ela vive, e as colisoes os leem a cada quadro.
    Colisoes(const std::vector<Player*>& jogadores, const std::vector<Boss*>& corpos,
             const std::vector<ExtraPointItem*>& pontosExtras, EfeitosDaBatalha& efeitos, Regras& regras);

    /// As colisoes de um quadro. 'chefe': o corpo original, que fica sabendo do que se derruba.
    void Verificar(Boss* chefe, const std::vector<std::unique_ptr<BossProjectile>>& doChefe,
                   const std::vector<std::unique_ptr<PlayerProjectile>>& dosAlunos);

private:
    /// O abrigo come os projeteis do chefe.
    void Abrigo(const std::vector<std::unique_ptr<BossProjectile>>& doChefe);
    /// O escudo do Ponto Extra come os projeteis do chefe.
    void Escudo(const std::vector<std::unique_ptr<BossProjectile>>& doChefe);
    /// Os projeteis do chefe acertam os alunos, ou passam rente.
    void TiroDoChefeNoAluno(const std::vector<std::unique_ptr<BossProjectile>>& doChefe);
    void Raspar(Player& aluno, const CircleColliderComponent& colisor, BossProjectile& projetil,
                const CircleColliderComponent& colisorDoProjetil);
    /// O tiro do aluno derruba (ou murcha em) projeteis do chefe.
    void TiroDoAlunoNosProjeteis(Boss* chefe, const std::vector<std::unique_ptr<PlayerProjectile>>& dosAlunos,
                                 const std::vector<std::unique_ptr<BossProjectile>>& doChefe);
    /// O ultimo golpe num projetil do chefe: ele cai, e o chefe fica sabendo.
    void Derrubar(Boss* chefe, PlayerProjectile& tiro, BossProjectile& projetil);
    /// O tiro do aluno acerta o professor.
    void TiroDoAlunoNoChefe(const std::vector<std::unique_ptr<PlayerProjectile>>& dosAlunos);
    /// O aluno encosta no professor.
    void Contato();
    /// O aluno pega um Ponto Extra.
    void PontosExtras();

    const std::vector<Player*>& mJogadores;
    const std::vector<Boss*>& mCorpos;
    const std::vector<ExtraPointItem*>& mPontosExtras;
    EfeitosDaBatalha& mEfeitos;
    Regras& mRegras;
};
