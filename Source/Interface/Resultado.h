//
// A tela de resultado da batalha: o resumo e o que fazer em seguida.
//

#pragma once

#include <string>
#include <vector>

/**
 * CAMADA PURA. Coberto por test_resultado.cpp.
 *
 * A LUTA ACABOU: em vez de mostrar APROVADE por quatro segundos e voltar
 * sozinha para as materias, a batalha mostra o resumo dela - a nota, se bateu
 * o recorde, as fases limpas, os acertos sofridos - e pergunta o que fazer:
 * tentar de novo a mesma materia, ou voltar. Quem reprovou por pouco quer a
 * segunda chance a um botao de distancia.
 *
 * Quem desenha e a Battle; aqui ficam as opcoes, os textos e a regra do
 * recorde.
 */
namespace Resultado {

    /// As opcoes, na ordem da tela. A primeira e a que abre em foco.
    enum class Opcao { TentarDeNovo, Voltar };
    constexpr Opcao kOpcoes[] = {Opcao::TentarDeNovo, Opcao::Voltar};
    constexpr int kQuantas = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    const char* Texto(Opcao o);

    /// @brief A opcao 'passo' casas depois da 'atual', dando a volta.
    int Mover(int atual, int passo);

    /// O que a batalha sabe no fim dela.
    struct Resumo {
        float nota = 0.0f;
        bool aprovado = false;
        /// O aluno tem ficha gravada (o visitante nao tem recorde a bater).
        bool temFicha = false;
        /// O recorde dele nesta materia ANTES desta luta.
        float recordeAntes = 0.0f;
        int fasesLimpas = 0;
        int acertosSofridos = 0;
    };

    /// O titulo: "APROVADE!" ou "REPROVADE".
    const char* Titulo(bool aprovado);

    /// @brief A nota desta luta passou o recorde? So para quem tem ficha, e por
    /// mais que o arredondamento da tela (0,01).
    bool NovoRecorde(const Resumo& r);

    /// @brief As linhas do resumo, de cima para baixo: a nota, o recorde (ou o
    /// aviso de recorde novo), as fases limpas e os acertos sofridos.
    std::vector<std::string> Linhas(const Resumo& r);

    /// Quantas Linhas um resumo tem no maximo: e quantas o painel reserva.
    constexpr int kLinhasNoMaximo = 4;

    /// @brief Qual das Linhas vai em destaque (dourada): a do recorde novo, ou
    /// -1 quando nao ha nenhuma.
    int LinhaEmDestaque(const Resumo& r);

    /// @brief Segundos antes de o menu aceitar o botao. O botao 1 e o do tiro:
    /// quem estava atirando quando a luta acabou nao pode escolher sem querer.
    constexpr float kTravaDoMenu = 0.8f;

    /**
     * @brief NO GABINETE, o resultado volta sozinho para as materias.
     *
     * A ociosidade do gabinete nao vale dentro da batalha (Game::AtualizarGabinete:
     * uma luta em andamento nao pode ser interrompida), e a tela de resultado e
     * parte da batalha. Sem isto, um aluno que vai embora sem escolher deixaria
     * a maquina parada no resumo dele para sempre. Voltando para as materias, a
     * ociosidade de sempre assume dali.
     *
     * No computador, nao: quem levantou volta e encontra o resumo onde deixou.
     */
    constexpr float kEsperaNoGabinete = 20.0f;
    bool VoltaSozinha(bool arcade, float segundosNaTela);
}
