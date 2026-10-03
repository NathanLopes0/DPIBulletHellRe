//
// As notas da turma em texto, para o professor.
//

#pragma once

#include <string>
#include <vector>

#include "Materias.h"
#include "Progresso.h"

/**
 * CAMADA PURA: monta o texto a partir das fichas que recebe. Nao abre arquivo e
 * nao sabe onde os saves moram - quem junta tudo e Source/ExportacaoArquivo.
 *
 * UMA LINHA POR ALUNO-MATERIA, e nao uma por aluno com uma coluna por materia.
 * A escolha foi pensando no ranking em tela que vem depois: ordenar e filtrar
 * linhas nao exige remontar nada, enquanto uma tabela larga exigiria uma coluna
 * nova a cada materia que o curso ganhar.
 */
namespace Exportacao {

    /// O que se sabe de um aluno: a matricula e o progresso dele.
    struct FichaDeAluno {
        std::string matricula;
        Progresso progresso;
    };

    /**
     * @brief As notas da turma em CSV, com cabecalho.
     *
     * Colunas: matricula, materia (o codigo estavel), nome, recorde, retomada,
     * aprovado, quando.
     *
     * O CODIGO e o NOME saem os dois de proposito: o codigo e o que nao muda e
     * serve para cruzar dados, o nome e o que o professor le. Materia que o aluno
     * nunca jogou NAO vira linha - linha vazia poluiria o ranking.
     *
     * As linhas saem em ordem de matricula e, dentro dela, na ordem das materias,
     * para que duas exportacoes do mesmo estado sejam o mesmo arquivo.
     */
    std::string ParaCsv(const std::vector<FichaDeAluno>& fichas,
                        const Materias::Lista& materias);

    /// @brief Um campo CSV, com aspas se precisar. Exposto para teste.
    std::string Campo(const std::string& valor);
}
