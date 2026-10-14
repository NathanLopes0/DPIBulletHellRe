//
// A ponte entre a exportacao e o disco.
//

#include "ExportacaoArquivo.h"

#include <filesystem>
#include <fstream>
#include <system_error>
#include <SDL_log.h>

#include "Exportacao.h"
#include "FichaArquivo.h"
#include "CaminhosArquivo.h"

namespace Exportacao {

bool Regravar(const Materias::Lista& materias) {

    std::vector<FichaDeAluno> fichas;
    for (const auto& matricula : FichaArquivo::ListarMatriculas()) {
        FichaDeAluno f;
        f.matricula = matricula;
        // A planilha e de NOTAS: a aparencia vem junto na ficha e e ignorada
        // aqui de proposito, porque nao e assunto do professor.
        f.progresso = FichaArquivo::Carregar(matricula, materias).progresso;
        fichas.push_back(std::move(f));
    }

    const std::string caminho = Caminhos::Save("notas.csv");
    const std::string temporario = caminho + ".tmp";

    {
        std::ofstream saida(temporario, std::ios::trunc);
        if (!saida.is_open()) {
            SDL_Log("EXPORTACAO: nao consegui escrever %s. As fichas dos alunos estao a "
                    "salvo; so a planilha nao foi atualizada.", temporario.c_str());
            return false;
        }
        saida << ParaCsv(fichas, materias);
        if (!saida.good()) {
            SDL_Log("EXPORTACAO: erro ao escrever %s.", temporario.c_str());
            return false;
        }
    }

    // Mesma troca atomica das fichas: ninguem deve abrir uma planilha pela metade.
    std::error_code ec;
    std::filesystem::rename(temporario, caminho, ec);
    if (ec) {
        SDL_Log("EXPORTACAO: nao consegui trocar %s por %s (%s).",
                temporario.c_str(), caminho.c_str(), ec.message().c_str());
        std::filesystem::remove(temporario, ec);
        return false;
    }

    return true;
}

}
