//
// As materias do curso, lidas de texto JSON.
//

#pragma once

#include <string>
#include <vector>

#include "../Aluno/Progresso.h"

/// Transforma texto em lista de materias e responde perguntas sobre ela; nao abre arquivo, nao
/// conhece Game nem SDL.
namespace Materias {

    enum class TipoDeDesbloqueio {
        Sempre,              ///< a porta de entrada do curso
        AprovadoEm,          ///< Aprovacao em todas as materias listadas.
        AprovadasNaColuna    ///< N aprovacoes entre as materias de uma coluna
    };

    struct Desbloqueio {
        TipoDeDesbloqueio tipo = TipoDeDesbloqueio::Sempre;

        std::vector<std::string> materias;   ///< AprovadoEm: os codigos exigidos
        int quantas = 0;                     ///< AprovadasNaColuna
        int coluna = 0;                      ///< AprovadasNaColuna
    };

    struct Materia {
        /// A chave estavel. Vai para o save do aluno e para a exportacao do professor.
        std::string codigo;

        /// O que aparece no botao. Pode mudar sem efeito nenhum sobre os saves.
        std::string nome;

        /// O nome da disciplina: "Banco de Dados". Pode faltar - e so nao mostrar.
        std::string nomeCompleto;

        /// Quem da a materia. Independente de 'chefe': o TCC e do Hugo e nao tem batalha.
        std::string professor;

        /// A posicao na grade da selecao de fase, a partir de 0. A coluna tambem e o que as regras
        /// de desbloqueio usam, entao ela e declarada uma vez.
        int coluna = 0;

        /// Qual chefe esta materia usa. Vazio = batalha ainda nao feita, que e o caso de seis das
        /// onze hoje.
        std::string chefe;
        /// Se a batalha desta materia pede um orientador antes de comecar - o TCC. Quem pode
        /// orientar e quem decide entre eles e Orientacao.h.
        bool orientacao = false;

        Desbloqueio desbloqueio;
    };

    /// A lista lida, mais as consultas que o jogo faz sobre ela.
    struct Lista {
        std::vector<Materia> materias;
        std::vector<std::string> problemas;

        [[nodiscard]] int Quantas() const { return static_cast<int>(materias.size()); }

        /// O indice de um codigo, ou -1 se nao existe.
        [[nodiscard]] int IndiceDe(const std::string& codigo) const;

        /// A materia nesse indice, ou nullptr se estiver fora da lista.
        [[nodiscard]] const Materia* Por(int indice) const;

        /// O codigo nesse indice, ou vazio.
        [[nodiscard]] std::string CodigoDe(int indice) const;

        /// Os indices das materias de uma coluna, na ordem da lista.
        [[nodiscard]] std::vector<int> DaColuna(int coluna) const;

        /// Quantas colunas a grade tem (a maior coluna usada, mais um).
        [[nodiscard]] int QuantasColunas() const;

        /// Esta materia esta liberada para este progresso? Nao recorre: "aprovado em X" olha a nota
        /// de X, e nao se X esta desbloqueada.
        [[nodiscard]] bool Desbloqueada(int indice, const Progresso& progresso) const;

        /// O que falta para esta materia abrir, numa frase para o aluno. Vazio quando a materia
        /// abre sempre - nao ha o que explicar.
        [[nodiscard]] std::string ExigenciaDe(int indice) const;

        /// Se toda materia desta coluna depende apenas da coluna de tras.
        [[nodiscard]] bool ColunaDependeDaAnterior(int coluna) const;
    };

    /// Interpreta o texto de materias.json. Nunca lanca.
    Lista LerMaterias(const std::string& textoJson);

    /// Se o texto serve como codigo de materia (letras e numeros, sem espaco).
    bool CodigoServe(const std::string& codigo);
}
