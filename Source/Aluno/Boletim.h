//
// O boletim do semestre: as materias do aluno, periodo a periodo, com a melhor nota, a situacao e
// as tentativas - e o CR.
//

#pragma once

#include <string>
#include <vector>

#include "Progresso.h"
#include "../Curso/Materias.h"

/// O que a tela do boletim mostra (TelaDoBoletim), montado do progresso de um aluno e da grade do
/// curso. Nao conhece tela, disco nem SDL.
namespace Boletim {

    /// Em que pe a materia esta para o aluno.
    enum class Situacao {
        Aprovade,    ///< a melhor nota chegou a aprovacao
        Reprovade,   ///< jogou, e a melhor nota ficou abaixo
        ACursar,     ///< aberta, e ainda nao jogou
        Fechada      ///< falta o que a abre (Materias::Lista::Desbloqueada)
    };

    /// Uma materia no boletim.
    struct Linha {
        int materia = 0;            ///< o indice na grade
        std::string codigo;         ///< "INF 213", o nome do botao
        std::string nome;           ///< "Estruturas de Dados"; pode faltar
        std::string professor;
        int periodo = 1;            ///< a coluna da grade, contada de 1
        bool optativa = false;      ///< as optativas vem por ultimo, numa secao so delas
        float melhor = 0.0f;        ///< a maior nota, em qualquer modo e nivel; 0 se nunca jogou
        int tentativas = 0;         ///< provas terminadas; 0 tambem num save de antes da contagem
        int reprovacoes = 0;
        bool noDificil = false;     ///< aprovado tambem no Dificil (sozinho ou em dupla)
        Situacao situacao = Situacao::ACursar;
    };

    struct Dados {
        std::vector<Linha> linhas;  ///< na ordem da grade: periodo a periodo
        float cr = 0.0f;            ///< a media das melhores notas das materias cursadas
        int cursadas = 0;
        int aprovadas = 0;          ///< as obrigatorias aprovadas (as optativas nao contam)
        int total = 0;              ///< as obrigatorias do curso, como na selecao de fases
    };

    /// O boletim de quem tem este progresso, nesta grade.
    Dados Montar(const Progresso& progresso, const Materias::Lista& materias);

    /// "Aprovade", "Reprovade", "A cursar", "Fechada".
    const char* NomeDa(Situacao s);

    /// "3", ou "-" quando a materia nunca foi terminada (ou o save e de antes da contagem).
    std::string Tentativas(const Linha& l);

    /// "74.32", ou "-" sem nota.
    std::string Nota(float n);

    /// "1o periodo", com o "o" sobrescrito.
    std::string Periodo(int n);
    /// O nome da secao da linha: o periodo, ou "Optativas".
    std::string Secao(const Linha& l);
    /// O texto com no maximo 'maximo' letras (contadas em UTF-8): o que passa vira reticencias.
    std::string Encurtar(const std::string& texto, size_t maximo);
}
