//
// A ponte entre o registro das tentativas e o disco.
//

#include "TentativasArquivo.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <SDL_log.h>

#include "../Core/CaminhosArquivo.h"

namespace TentativasArquivo {

namespace {

    std::string LerTudo(const std::string& caminho) {
        std::ifstream entrada(caminho, std::ios::binary);
        if (!entrada.is_open()) return "";
        std::ostringstream conteudo;
        conteudo << entrada.rdbuf();
        return conteudo.str();
    }

    /// ACRESCENTA, e nao reescreve: o que ja esta no arquivo nunca e tocado.
    bool Acrescentar(const std::string& caminho, const std::string& cabecalho,
                     const std::vector<std::string>& linhas) {
        std::error_code ec;
        const bool novo = !std::filesystem::exists(caminho, ec)
                       || std::filesystem::file_size(caminho, ec) == 0;
        std::ofstream saida(caminho, std::ios::app | std::ios::binary);
        if (!saida.is_open()) {
            SDL_Log("TENTATIVAS: nao consegui abrir %s.", caminho.c_str());
            return false;
        }
        if (novo) saida << cabecalho << '\n';
        for (const auto& linha : linhas) saida << linha << '\n';
        if (!saida.good()) {
            SDL_Log("TENTATIVAS: erro ao escrever %s.", caminho.c_str());
            return false;
        }
        return true;
    }

    /// O RESUMO E DERIVADO: refeito inteiro, com a mesma troca atomica da
    /// planilha de notas - ninguem abre um resumo pela metade.
    bool Regravar(const std::string& caminho, const std::string& conteudo) {
        const std::string temporario = caminho + ".tmp";
        {
            std::ofstream saida(temporario, std::ios::trunc | std::ios::binary);
            if (!saida.is_open()) return false;
            saida << conteudo;
            if (!saida.good()) return false;
        }
        std::error_code ec;
        std::filesystem::rename(temporario, caminho, ec);
        if (ec) {
            SDL_Log("TENTATIVAS: nao consegui trocar %s (%s).", caminho.c_str(), ec.message().c_str());
            std::filesystem::remove(temporario, ec);
            return false;
        }
        return true;
    }
}

bool Gravar(const Tentativas::Tentativa& tentativa) {
    try {
        const std::string caminho = Caminhos::Save("tentativas.csv");
        auto linhas = Tentativas::Ler(LerTudo(caminho));

        std::vector<int> vezes;
        for (const auto& j : tentativa.jogadores) {
            vezes.push_back(j.visitante ? 0 : Tentativas::ProximaVez(linhas, j.matricula, tentativa.materia));
        }
        const auto novas = Tentativas::LinhasDaTentativa(tentativa, vezes);

        bool ok = Acrescentar(caminho, Tentativas::CabecalhoDasTentativas(), novas);
        ok = Acrescentar(Caminhos::Save("tentativas_fases.csv"), Tentativas::CabecalhoDasFases(),
                         Tentativas::LinhasDasFases(tentativa)) && ok;

        // O resumo le de volta o que acabou de ser gravado: o que esta nele e
        // exatamente o que esta no arquivo, nunca uma conta a parte.
        linhas = Tentativas::Ler(LerTudo(caminho));
        ok = Regravar(Caminhos::Save("tentativas_resumo.csv"), Tentativas::Resumo(linhas)) && ok;

        SDL_Log("TENTATIVAS: %s %s (%s), nota %.2f, gravada em %s.", tentativa.materia.c_str(),
                Tentativas::Nome(tentativa.resultado), tentativa.modo.c_str(),
                static_cast<double>(tentativa.notaFinal), caminho.c_str());
        return ok;
    } catch (const std::exception& e) {
        SDL_Log("TENTATIVAS: a tentativa nao foi gravada (%s).", e.what());
        return false;
    }
}

}
