//
// A ponte entre o registro das tentativas e o disco.
//

#include "TentativasArquivo.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <SDL_log.h>

#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Relogio.h"

namespace TentativasArquivo {

namespace {

    std::string LerTudo(const std::string& caminho) {
        std::ifstream entrada(caminho, std::ios::binary);
        if (!entrada.is_open()) return "";
        std::ostringstream conteudo;
        conteudo << entrada.rdbuf();
        return conteudo.str();
    }

    bool Regravar(const std::string& caminho, const std::string& conteudo);

    /// O arquivo de um formato anterior passa para o atual antes de receber linhas novas: so o
    /// cabecalho muda, e as linhas antigas ficam como estavam (Tentativas::TrocarCabecalho). Um
    /// arquivo que nao e de formato anterior nenhum e guardado de lado, com outro nome, e o novo
    /// comeca do zero - nada se perde, e nada fica com colunas trocadas.
    void AtualizarFormato(const std::string& caminho, const std::string& cabecalho) {
        const std::string conteudo = LerTudo(caminho);
        if (conteudo.empty()) return;
        const auto atualizado = Tentativas::TrocarCabecalho(conteudo, cabecalho);
        if (atualizado) {
            if (*atualizado != conteudo && !Regravar(caminho, *atualizado)) {
                SDL_Log("TENTATIVAS: nao consegui atualizar o cabecalho de %s.", caminho.c_str());
            }
            return;
        }
        std::filesystem::path antigo(caminho);
        antigo.replace_filename(antigo.stem().string() + "_de_outro_formato_" +
                                Tentativas::NovoId(Relogio::Agora(), 0).substr(0, 15) + ".csv");
        std::error_code ec;
        std::filesystem::rename(caminho, antigo, ec);
        SDL_Log("TENTATIVAS: %s tinha outras colunas; foi guardado como %s.", caminho.c_str(),
                antigo.string().c_str());
    }

    /// Acrescenta, e nao reescreve: o que ja esta no arquivo nunca e tocado - so o cabecalho,
    /// quando o formato muda (AtualizarFormato).
    bool Acrescentar(const std::string& caminho, const std::string& cabecalho,
                     const std::vector<std::string>& linhas) {
        std::error_code ec;
        if (std::filesystem::exists(caminho, ec)) AtualizarFormato(caminho, cabecalho);
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

    /// O resumo e derivado: refeito inteiro, com a mesma troca atomica da planilha de notas -
    /// ninguem abre um resumo pela metade.
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

        // O resumo le de volta o que acabou de ser gravado.
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
