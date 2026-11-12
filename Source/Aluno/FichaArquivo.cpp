//
// A ponte entre a ficha do aluno e o disco.
//

#include "FichaArquivo.h"

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <system_error>
#include <SDL_log.h>

#include "Ficha.h"
#include "Matricula.h"
#include "../Motor/CaminhosArquivo.h"

namespace {

    /// O nome do arquivo de um aluno. So e chamado com matricula ja validada.
    std::string CaminhoDa(const std::string& matricula) {
        return Caminhos::Save(matricula + ".json");
    }

    /// Valida e devolve a forma canonica, ou vazio quando nao serve.
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

Ficha::Dados Carregar(const std::string& matricula, const Materias::Lista& materias) {

    Ficha::Dados vazio;

    const std::string canonica = Canonica(matricula, "carregar");
    if (canonica.empty()) return vazio;
    vazio.matricula = canonica;

    const std::string caminho = CaminhoDa(canonica);

    const std::optional<std::string> texto = Caminhos::LerTexto(caminho);
    if (!texto) {
        // Nao e erro: e o caso normal de aluno jogando pela primeira vez.
        return vazio;
    }

    const Ficha::Lida lida = Ficha::Desserializar(*texto, materias);

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
        // O arquivo diz ser de outro aluno.
        SDL_Log("FICHA %s: o arquivo %s diz ser da matricula %s. Usando mesmo assim, "
                "mas confira se o arquivo foi renomeado.",
                canonica.c_str(), caminho.c_str(), lida.dados.matricula.c_str());
    }

    // O nome do arquivo manda, inclusive sobre o que o arquivo diz.
    Ficha::Dados saida = lida.dados;
    saida.matricula = canonica;
    return saida;
}

bool Gravar(const Ficha::Dados& dadosPedidos, const Materias::Lista& materias) {

    const std::string canonica = Canonica(dadosPedidos.matricula, "gravar");
    if (canonica.empty()) return false;

    Ficha::Dados dados = dadosPedidos;
    dados.matricula = canonica;

    const std::string caminho = CaminhoDa(canonica);
    const std::string temporario = caminho + ".tmp";

    {
        std::ofstream saida(temporario, std::ios::trunc);
        if (!saida.is_open()) {
            SDL_Log("FICHA %s: nao consegui abrir %s para escrever. O progresso deste "
                    "aluno NAO foi gravado.", canonica.c_str(), temporario.c_str());
            return false;
        }
        saida << Ficha::Serializar(dados, materias);
        if (!saida.good()) {
            SDL_Log("FICHA %s: erro ao escrever %s. O progresso NAO foi gravado.",
                    canonica.c_str(), temporario.c_str());
            return false;
        }
    }   // fecha antes de renomear

    // A troca so acontece com o arquivo novo ja inteiro no disco.
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

    const std::string pasta = std::filesystem::path(CaminhoDa("0")).parent_path().string();

    std::error_code ec;
    for (const auto& entrada : std::filesystem::directory_iterator(pasta, ec)) {
        if (ec) break;
        if (!entrada.is_regular_file()) continue;

        const auto caminho = entrada.path();
        if (caminho.extension() != ".json") continue;   // pula .tmp e o que mais houver

        // O nome do arquivo e a fonte da verdade, e ele passa pela mesma validacao.
        const auto r = Matricula::Validar(caminho.stem().string());
        if (!r.valida) continue;

        matriculas.push_back(r.canonica);
    }

    if (ec) {
        SDL_Log("FICHA: nao consegui listar a pasta de saves %s (%s).",
                pasta.c_str(), ec.message().c_str());
    }

    // Em ordem numerica, e nao alfabetica: "9" antes de "10".
    std::sort(matriculas.begin(), matriculas.end(),
              [](const std::string& a, const std::string& b) {
                  if (a.size() != b.size()) return a.size() < b.size();
                  return a < b;
              });

    return matriculas;
}

std::vector<Exportacao::FichaDeAluno> TodasAsFichas(const Materias::Lista& materias) {
    std::vector<Exportacao::FichaDeAluno> fichas;
    for (const auto& matricula : ListarMatriculas()) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        f.progresso = Carregar(matricula, materias).progresso;
        fichas.push_back(std::move(f));
    }
    return fichas;
}

}
