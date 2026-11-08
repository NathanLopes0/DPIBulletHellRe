//
// As notas da turma em texto, para o professor.
//

#pragma once

#include <string>
#include <vector>

#include "../Curso/Materias.h"
#include "Progresso.h"

/// Nao abre arquivo e nao sabe onde os saves moram - quem junta tudo e Source/ExportacaoArquivo.
namespace Exportacao {

    /// O que se sabe de um aluno: a matricula e o progresso dele.
    struct FichaDeAluno {
        std::string matricula;
        Progresso progresso;
    };

    /// As notas da turma em CSV, com cabecalho.
    std::string ParaCsv(const std::vector<FichaDeAluno>& fichas,
                        const Materias::Lista& materias);

    /// Um campo CSV, com aspas se precisar. Exposto para teste.
    std::string Campo(const std::string& valor);
}
