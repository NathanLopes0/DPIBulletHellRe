//
// As materias do curso, lidas de texto JSON.
//

#pragma once

#include <string>
#include <vector>

#include "Progresso.h"

/**
 * CAMADA PURA. Transforma texto em lista de materias e responde perguntas sobre
 * ela; nao abre arquivo, nao conhece Game nem SDL.
 *
 * POR QUE ISTO EXISTE. A lista de materias era um enum em Game.h e mais cinco
 * lugares em C++ que repetiam pedacos dela - o mapa de chefes, os nomes em tela,
 * as colunas (duplicadas, com nomes trocados entre os dois arquivos), a
 * quantidade e o tamanho de cada coluna. Mudar uma materia exigia acertar os
 * seis, e esquecer um nao dava erro de compilacao.
 *
 * E, pior: a POSICAO no enum era a chave gravada no save do aluno. Reordenar a
 * lista reinterpretava todos os saves em silencio. Aqui a chave e o CODIGO, um
 * texto que nao muda quando a ordem muda.
 */
namespace Materias {

    enum class TipoDeDesbloqueio {
        Sempre,              ///< a porta de entrada do curso
        AprovadoEm,          ///< aprovacao em TODAS as materias listadas
        AprovadasNaColuna    ///< N aprovacoes entre as materias de uma coluna
    };

    struct Desbloqueio {
        TipoDeDesbloqueio tipo = TipoDeDesbloqueio::Sempre;

        std::vector<std::string> materias;   ///< AprovadoEm: os codigos exigidos
        int quantas = 0;                     ///< AprovadasNaColuna
        int coluna = 0;                      ///< AprovadasNaColuna
    };

    struct Materia {
        /// A CHAVE ESTAVEL. Vai para o save do aluno e para a exportacao do
        /// professor. Reordenar a lista nao mexe nela; trocar o codigo e, para o
        /// jogo, aposentar a materia e criar outra.
        std::string codigo;

        /// O que aparece no botao. Pode mudar sem efeito nenhum sobre os saves.
        std::string nome;

        /// A posicao na grade da selecao de fase, a partir de 0. A coluna tambem e
        /// o que as regras de desbloqueio usam, entao ela e declarada UMA vez.
        int coluna = 0;

        /// Qual chefe esta materia usa. Vazio = materia ainda sem chefe, que e o
        /// caso de seis das dez hoje.
        std::string chefe;

        Desbloqueio desbloqueio;
    };

    /**
     * @brief A lista lida, mais as consultas que o jogo faz sobre ela.
     *
     * As consultas moram aqui, e nao espalhadas por quem usa, porque e assim que
     * "quais materias tem a coluna 1" deixa de existir em duas versoes.
     */
    struct Lista {
        std::vector<Materia> materias;
        std::vector<std::string> problemas;

        [[nodiscard]] int Quantas() const { return static_cast<int>(materias.size()); }

        /// @brief O indice de um codigo, ou -1 se nao existe.
        ///
        /// O INDICE E DE USO INTERNO E NAO DURAVEL: ele muda quando a lista muda, e
        /// e exatamente por isso que o que vai para o disco e o codigo.
        [[nodiscard]] int IndiceDe(const std::string& codigo) const;

        /// @brief A materia nesse indice, ou nullptr se estiver fora da lista.
        [[nodiscard]] const Materia* Por(int indice) const;

        /// @brief O codigo nesse indice, ou vazio.
        [[nodiscard]] std::string CodigoDe(int indice) const;

        /// @brief Os indices das materias de uma coluna, na ordem da lista.
        [[nodiscard]] std::vector<int> DaColuna(int coluna) const;

        /// @brief Quantas colunas a grade tem (a maior coluna usada, mais um).
        [[nodiscard]] int QuantasColunas() const;

        /**
         * @brief Esta materia esta liberada para este progresso?
         *
         * Nao recorre: "aprovado em X" olha a NOTA de X, e nao se X esta
         * desbloqueada. Entao duas materias que se exigem mutuamente nao causam
         * laco - elas so ficam ambas fechadas, que e o resultado correto.
         */
        [[nodiscard]] bool Desbloqueada(int indice, const Progresso& progresso) const;

        /**
         * @brief O que falta para esta materia abrir, numa frase para o aluno.
         *
         * Vazio quando a materia abre sempre - nao ha o que explicar.
         *
         * NAO olha o progresso: isto e a REGRA, e nao o estado. Quem chama ja
         * sabe que a materia esta fechada (foi por isso que perguntou), e uma
         * frase que mudasse conforme o que o aluno ja fez precisaria repetir a
         * conta que Desbloqueada ja faz.
         *
         * Usa o NOME de cada materia exigida, e nao o codigo: e "INF 213" que o
         * aluno ve no botao ao lado, enquanto o que vai para o disco e
         * "INF213". A coluna aparece contada a partir de 1, que e como se conta
         * coluna olhando para a tela.
         */
        [[nodiscard]] std::string ExigenciaDe(int indice) const;
    };

    /**
     * @brief Interpreta o texto de materias.json.
     *
     * Nunca lanca. Uma materia com problema grave e descartada e as outras
     * continuam valendo; um problema leve vira aviso e o campo volta ao padrao.
     *
     * Confere tambem duas coisas que nenhum compilador pegaria: que todo codigo
     * citado numa regra de desbloqueio existe, e que nenhuma regra e IMPOSSIVEL -
     * pedir duas aprovacoes numa coluna de uma materia so deixaria aquela materia
     * fechada para sempre, sem erro nenhum em jogo.
     */
    Lista LerMaterias(const std::string& textoJson);

    /// @brief Se o texto serve como codigo de materia (letras e numeros, sem espaco).
    bool CodigoServe(const std::string& codigo);
}
