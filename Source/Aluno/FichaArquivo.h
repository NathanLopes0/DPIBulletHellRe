//
// A ponte entre a ficha do aluno e o disco.
//

#pragma once

#include <string>
#include <vector>

#include "Ficha.h"
#include "../Curso/Materias.h"
#include "Progresso.h"
#include "Exportacao.h"

/// Separado de Ficha.cpp pelo mesmo motivo dos outros pares do projeto: aquele e puro e entra no
/// alvo de testes.
namespace FichaArquivo {

    /// Carrega o progresso de uma matricula.
    Ficha::Dados Carregar(const std::string& matricula, const Materias::Lista& materias);

    /// Grava a ficha. Devolve false se nao conseguiu.
    bool Gravar(const Ficha::Dados& dados, const Materias::Lista& materias);

    /// Se ja existe ficha para esta matricula.
    bool Existe(const std::string& matricula);

    /// Todas as matriculas com ficha gravada, em ordem. Para a exportacao do professor, e para o
    /// ranking em tela quando ele existir.
    std::vector<std::string> ListarMatriculas();

    /// As fichas de todos os alunos gravados, como o ranking as le (o rival da turma, Rival.h).
    std::vector<Exportacao::FichaDeAluno> TodasAsFichas(const Materias::Lista& materias);
}
