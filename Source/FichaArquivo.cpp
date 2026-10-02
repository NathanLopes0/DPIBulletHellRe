//
// A ponte entre a ficha do aluno e o disco.
//

#include "FichaArquivo.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <system_error>
#include <SDL_log.h>

#include "Ficha.h"
#include "Matricula.h"
#include "CaminhosArquivo.h"

namespace {

    /// O nome do arquivo de um aluno. So e chamado com matricula JA validada.
    std::string CaminhoDa(const std::string& matricula) {
        return Caminhos::Save(matricula + ".json");
    }

    /// Valida e devolve a forma canonica, ou vazio quando nao serve.
    ///
    /// Toda entrada deste modulo passa por aqui: e a unica barreira entre texto
    /// digitado e um nome de arquivo. Sem ela, uma "matricula" com barra ou ".."
    /// escreveria fora da pasta de saves.
    std::string Canonica(const std::string& matricula, const char* operacao) {
        const auto r = Matricula::Validar(matricula);
        if (!r.valida) {
            SDL_Log("FICHA: %s recusado - \"%s\" nao e uma matricula valida (%s)",
                    operacao, matricula.c_str(), Matricula::MensagemDeErro(r.erro).c_str());
            return "";
        }
        return r.canonica;
    }
}

namespace FichaArquivo {

Progresso Carregar(const std::string& matricula) {

    Progresso vazio;

    const std::string canonica = Canonica(matricula, "carregar");
    if (canonica.empty()) return vazio;

    const std::string caminho = CaminhoDa(canonica);

    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        // Nao e erro: e o caso normal de aluno jogando pela primeira vez.
        return vazio;
    }

    std::ostringstream buffer;
    buffer << arquivo.rdbuf();

    const Ficha::Lida lida = Ficha::Desserializar(buffer.str());

    for (const auto& p : lida.problemas) {
        SDL_Log("FICHA %s: %s", canonica.c_str(), p.c_str());
    }

    if (!lida.ok) {
        SDL_Log("FICHA %s: o arquivo %s nao pode ser lido e o aluno vai comecar do zero. "
                "O arquivo NAO foi apagado - se o progresso importar, da para tentar "
                "consertar a mao.", canonica.c_str(), caminho.c_str());
        return vazio;
    }

    if (lida.dados.matricula != canonica) {
        // O arquivo diz ser de outro aluno. Acontece se alguem renomear um save.
        // Continuo com o progresso, mas aviso: o nome do arquivo e quem manda.
        SDL_Log("FICHA %s: o arquivo %s diz ser da matricula %s. Usando mesmo assim, "
                "mas confira se o arquivo foi renomeado.",
                canonica.c_str(), caminho.c_str(), lida.dados.matricula.c_str());
    }

    return lida.dados.progresso;
}

bool Gravar(const std::string& matricula, const Progresso& progresso) {

    const std::string canonica = Canonica(matricula, "gravar");
    if (canonica.empty()) return false;

    Ficha::Dados dados;
    dados.matricula = canonica;
    dados.progresso = progresso;

    const std::string caminho = CaminhoDa(canonica);
    const std::string temporario = caminho + ".tmp";

    {
        std::ofstream saida(temporario, std::ios::trunc);
        if (!saida.is_open()) {
            SDL_Log("FICHA %s: nao consegui abrir %s para escrever. O progresso deste "
                    "aluno NAO foi gravado.", canonica.c_str(), temporario.c_str());
            return false;
        }
        saida << Ficha::Serializar(dados);
        if (!saida.good()) {
            SDL_Log("FICHA %s: erro ao escrever %s. O progresso NAO foi gravado.",
                    canonica.c_str(), temporario.c_str());
            return false;
        }
    }   // fecha antes de renomear

    // A troca so acontece com o arquivo novo JA inteiro no disco. Se o jogo morrer
    // antes daqui, o save antigo continua valendo; se morrer depois, o novo esta
    // completo. O que nao pode existir e um save pela metade.
    std::error_code ec;
    std::filesystem::rename(temporario, caminho, ec);
    if (ec) {
        SDL_Log("FICHA %s: nao consegui trocar %s por %s (%s). O save anterior foi "
                "preservado.", canonica.c_str(), temporario.c_str(), caminho.c_str(),
                ec.message().c_str());
        std::filesystem::remove(temporario, ec);
        return false;
    }

    return true;
}

bool Existe(const std::string& matricula) {
    const std::string canonica = Canonica(matricula, "consultar");
    if (canonica.empty()) return false;

    std::error_code ec;
    return std::filesystem::exists(CaminhoDa(canonica), ec) && !ec;
}

std::vector<std::string> ListarMatriculas() {

    std::vector<std::string> matriculas;

    // Caminhos::Save cria a pasta, entao pedir um caminho qualquer garante que ela
    // existe antes da varredura.
    const std::string pasta = std::filesystem::path(CaminhoDa("0")).parent_path().string();

    std::error_code ec;
    for (const auto& entrada : std::filesystem::directory_iterator(pasta, ec)) {
        if (ec) break;
        if (!entrada.is_regular_file()) continue;

        const auto caminho = entrada.path();
        if (caminho.extension() != ".json") continue;   // pula .tmp e o que mais houver

        // O NOME DO ARQUIVO e a fonte da verdade, e ele passa pela mesma validacao:
        // um arquivo solto na pasta nao vira aluno.
        const auto r = Matricula::Validar(caminho.stem().string());
        if (!r.valida) continue;

        matriculas.push_back(r.canonica);
    }

    if (ec) {
        SDL_Log("FICHA: nao consegui listar a pasta de saves %s (%s).",
                pasta.c_str(), ec.message().c_str());
    }

    // Em ordem numerica, e nao alfabetica: "9" antes de "10". A pasta ja devolve
    // em ordem de texto, e "10" viria antes de "9".
    std::sort(matriculas.begin(), matriculas.end(),
              [](const std::string& a, const std::string& b) {
                  if (a.size() != b.size()) return a.size() < b.size();
                  return a < b;
              });

    return matriculas;
}

}
