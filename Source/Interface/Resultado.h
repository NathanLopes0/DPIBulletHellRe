//
// A tela de resultado da batalha: o resumo e o que fazer em seguida.
//

#pragma once

#include <string>
#include <vector>

#include "Devolutiva.h"

/// A luta acabou: em vez de mostrar aprovade por quatro segundos e voltar sozinha para as materias,
/// a batalha mostra o resumo dela.
namespace Resultado {

    /// As opcoes, na ordem da tela. A primeira e a que abre em foco.
    enum class Opcao { TentarDeNovo, Voltar };
    constexpr Opcao kOpcoes[] = {Opcao::TentarDeNovo, Opcao::Voltar};
    constexpr int kQuantas = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    const char* Texto(Opcao o);

    /// A opcao 'passo' casas depois da 'atual', dando a volta.
    int Mover(int atual, int passo);

    /// O que a batalha sabe no fim dela.
    struct Resumo {
        float nota = 0.0f;
        bool aprovado = false;
        /// O aluno tem ficha gravada (o visitante nao tem recorde a bater).
        bool temFicha = false;
        float recordeAntes = 0.0f;
        int fasesLimpas = 0;
        int acertosSofridos = 0;

        /// A devolutiva (Devolutiva.h), que so quem reprovou ve: a fase em que a nota mais caiu...
        std::optional<Devolutiva::Fase> piorFase;
        /// ...e o que mais acertou, da causa que mais custou (ate Devolutiva::kCausasNaTela).
        std::vector<Devolutiva::Causa> causas;

        /// Na dupla, a parte de cada aluno, na ordem do painel (a justica)...
        std::vector<Devolutiva::Parte> partes;
        /// ...e o titulo de cada um (Devolutiva::Titulos). Vazio, a parte e uma linha so, como antes
        /// dos titulos (Experimento.h, sem-titulos).
        std::vector<std::string> titulos;

        /// Quem se formou nesta luta: passou agora na materia da formatura (Personagens::Formatura)
        /// e ganhou o capelo. Na dupla, o nome de cada um, como em Devolutiva::Parte::quem.
        std::vector<std::string> formados;
        /// O capelo ficou gravado na ficha de todos eles (o visitante nao tem ficha).
        bool formaturaGravada = false;

        /// O que esta luta deu alem da nota (Mencoes::FraseDasNovas, e o orientador novo no
        /// curriculo): uma linha dourada, so de quem passou. Vazia quando nao deu nada.
        std::string mencoes;

        /// As horas complementares que a luta deu (Horas::FraseDasNovas): outra linha dourada, depois
        /// da das mencoes. Vazia sem nenhuma.
        std::string horas;

        /// Na prova da semana, o lugar de quem jogou no ranking dela ("2\xc2\xba de 5"), que vai ao
        /// lado da nota; vazio fora dela, e para o visitante.
        std::string provaDaSemana;
        /// O rival da turma (Rival::FraseDoResultado): quem esta nota passou no ranking da materia, ou
        /// quanto faltou para o mais perto. Vazia quando nao ha o que dizer.
        std::string rival;
    };

    /// O titulo: "APROVADE!" ou "REPROVADE".
    const char* Titulo(bool aprovado);

    /// O titulo deste resumo: o de cima, ou "FORMADE!" quando alguem se formou nesta luta.
    const char* Titulo(const Resumo& r);

    /// Alguem se formou nesta luta (Resumo::formados)?
    bool Formou(const Resumo& r);

    /// A nota desta luta passou o recorde? So para quem tem ficha, e por mais que o arredondamento
    /// da tela (0,01).
    bool NovoRecorde(const Resumo& r);

    /// As linhas do resumo, de cima para baixo: a nota, o recorde (ou o aviso de recorde novo), as
    /// fases limpas e os acertos sofridos. Quem reprovou ve a devolutiva: a fase em que a nota mais
    /// caiu e, numa linha so dos desenhos (LinhaDasCausas), o que mais o acertou. Quem se formou ve
    /// quem ganhou o capelo (LinhaDaFormatura). Na dupla, a parte de cada aluno (LinhaDaParte): com os
    /// titulos, duas linhas - "89384, quem mais cobriu" e os numeros embaixo -; sem eles, uma.
    std::vector<std::string> Linhas(const Resumo& r);

    /// Quantas Linhas um resumo tem no maximo: as quatro de sempre, a do rival, as duas da devolutiva e
    /// as quatro da dupla com titulos - ou, para quem passou, a da formatura, a das mencoes e a das horas
    /// no lugar das duas da devolutiva.
    constexpr int kLinhasNoMaximo = 12;

    /// Quantas linhas a parte de cada aluno ocupa: duas com o titulo, uma sem.
    int LinhasPorParte(const Resumo& r);

    /// A linha de quem ganhou o capelo, que vai dourada; -1 quando ninguem se formou.
    int LinhaDaFormatura(const Resumo& r);

    /// A linha das mencoes novas, dourada tambem, logo depois da da formatura; -1 sem nenhuma.
    int LinhaDasMencoes(const Resumo& r);

    /// A linha das horas complementares, dourada, depois da das mencoes; -1 sem nenhuma.
    int LinhaDasHoras(const Resumo& r);
    /// A linha do rival da turma, dourada, depois das horas (e antes da devolutiva); -1 sem ela.
    int LinhaDoRival(const Resumo& r);

    /// A linha (vazia) em que a batalha desenha o que mais acertou o aluno; -1 quando nao ha.
    int LinhaDasCausas(const Resumo& r);

    /// A primeira linha da parte do aluno 'quem' na dupla, que vai na cor dele com as outras
    /// (LinhasPorParte); -1 quando nao ha.
    int LinhaDaParte(const Resumo& r, size_t quem);

    /// Qual das Linhas vai em destaque (dourada): a do recorde novo, ou -1 quando nao ha nenhuma.
    int LinhaEmDestaque(const Resumo& r);

    /// Segundos antes de o menu aceitar o botao. O botao 1 e o do tiro: quem estava atirando quando
    /// a luta acabou nao pode escolher sem querer.
    constexpr float kTravaDoMenu = 0.8f;

    /// No gabinete, o resultado volta sozinho para as materias.
    constexpr float kEsperaNoGabinete = 20.0f;
    bool VoltaSozinha(bool arcade, float segundosNaTela);
}
