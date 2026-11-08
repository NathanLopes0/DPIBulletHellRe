//
// Atalhos que so os testes usam.
//

#pragma once

#include <string>
#include <vector>

#include "../Source/Personagem/Personagens.h"
#include "../Source/Aluno/Progresso.h"
#include "../Source/Attacks/Compilador.h"

/// O jogo nao precisa destes, e por isso eles nao moram em Source/.
namespace Ajudantes {

    /// A aparencia padrao: a primeira peca e a primeira cor de cada categoria
    /// - e o que Resolver da para quem nao pediu nada.
    inline Personagens::Aparencia Padrao(const Personagens::Catalogo& c) {
        return c.Resolver(Personagens::Aparencia{});
    }

    /// A ultima nota (a "retomada" da planilha) de uma materia, ou zero.
    inline float UltimaNota(const Progresso& p, const int materia) {
        for (const auto& e : p.Entradas()) {
            if (e.materia == materia) return e.retomada;
        }
        return 0.0f;
    }

    /// O codigo da linha: a instrucao de cada no em pos-ordem.
    inline std::vector<std::string> Codigo(const Compilador::Arvore& a) {
        std::vector<std::string> codigo;
        for (const int n : Compilador::PosOrdem(a)) {
            const std::string i = Compilador::Instrucao(a, n);
            if (!i.empty()) codigo.push_back(i);
        }
        return codigo;
    }
}
