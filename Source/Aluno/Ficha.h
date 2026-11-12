//
// A ficha do aluno: a matricula mais o progresso dele, em texto.
//

#pragma once

#include <string>
#include <vector>

#include "../Curso/Materias.h"
#include "../Personagem/Personagens.h"
#include "Progresso.h"

/// Transforma ficha em texto e texto em ficha; nao abre arquivo, nao sabe onde os saves moram e nao
/// conhece SDL. Quem grava e Source/FichaArquivo.
namespace Ficha {

    /// A versao do formato, gravada em todo arquivo.
    inline constexpr int kVersaoAtual = 11;

    struct Dados {
        std::string matricula;   ///< A forma canonica, ver Matricula.h.
        Progresso progresso;

        /// As escolhas de aparencia, como estavam no arquivo. Nao sao conferidas contra o catalogo
        /// aqui: esta camada transforma texto em dado e nada mais.
        Personagens::Aparencia aparencia;
    };

    struct Lida {
        bool ok = false;
        Dados dados;

        /// Frases prontas dizendo o que esta errado. Vazio quando ok.
        std::vector<std::string> problemas;
    };

    /// A ficha como texto JSON.
    std::string Serializar(const Dados& dados, const Materias::Lista& materias);

    /// Le o texto de uma ficha. Nunca lanca.
    Lida Desserializar(const std::string& texto, const Materias::Lista& materias);
}
