//
// O mural dos formados: quem passou no TCC, do mais recente ao mais antigo.
//

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Exportacao.h"

/// A conta do mural da tela de atracao: quem se formou e quando. Nao conhece cena, SDL nem disco.
namespace Mural {

    struct Formado {
        std::string matricula;
        /// Quando passou na materia da formatura ("2026-10-02 14:30:00"); vazio num save de antes de a
        /// data existir (Progresso::AprovadaEm).
        std::string quando;
    };

    /// Quem passou na materia 'formatura' (o indice em materias.json), do mais recente ao mais antigo.
    /// Quem se formou sem data vem depois, pela matricula. No maximo 'quantos'.
    std::vector<Formado> Recentes(const std::vector<Exportacao::FichaDeAluno>& fichas, int formatura,
                                  std::size_t quantos);

    /// "02/10/2026", de "2026-10-02 14:30:00"; vazio se 'quando' nao comecar por uma data.
    std::string Data(const std::string& quando);
}
