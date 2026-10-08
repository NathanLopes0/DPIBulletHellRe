//
// As materias do curso, lidas de texto JSON.
//

#include "Materias.h"

#include <algorithm>
#include <set>

#include "JsonDeDados.h"

namespace {

    std::string Texto(const nlohmann::json& j, const char* nome, const std::string& padrao = "") {
        return (j.contains(nome) && j[nome].is_string()) ? j[nome].get<std::string>() : padrao;
    }

    int Inteiro(const nlohmann::json& j, const char* nome, const int padrao) {
        return (j.contains(nome) && j[nome].is_number_integer()) ? j[nome].get<int>() : padrao;
    }
}

namespace Materias {

bool CodigoServe(const std::string& codigo) {
    if (codigo.empty()) return false;
    for (const char c : codigo) {
        const bool letra  = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        const bool numero = (c >= '0' && c <= '9');
        if (!letra && !numero) return false;
    }
    return true;
}

int Lista::IndiceDe(const std::string& codigo) const {
    for (size_t i = 0; i < materias.size(); ++i) {
        if (materias[i].codigo == codigo) return static_cast<int>(i);
    }
    return -1;
}

const Materia* Lista::Por(const int indice) const {
    if (indice < 0 || indice >= Quantas()) return nullptr;
    return &materias[static_cast<size_t>(indice)];
}

std::string Lista::CodigoDe(const int indice) const {
    const Materia* m = Por(indice);
    return m ? m->codigo : std::string();
}

std::vector<int> Lista::DaColuna(const int coluna) const {
    std::vector<int> saida;
    for (size_t i = 0; i < materias.size(); ++i) {
        if (materias[i].coluna == coluna) saida.push_back(static_cast<int>(i));
    }
    return saida;
}

int Lista::QuantasColunas() const {
    int maior = -1;
    for (const auto& m : materias) maior = std::max(maior, m.coluna);
    return maior + 1;
}

std::string Lista::ExigenciaDe(const int indice) const {

    const Materia* m = Por(indice);
    if (!m) return {};

    const Desbloqueio& regra = m->desbloqueio;

    switch (regra.tipo) {

        case TipoDeDesbloqueio::Sempre:
            return {};

        case TipoDeDesbloqueio::AprovadoEm: {
            if (regra.materias.empty()) return {};

            std::string frase = "Precisa passar em ";
            for (size_t i = 0; i < regra.materias.size(); ++i) {
                if (i > 0) frase += (i + 1 == regra.materias.size()) ? " e " : ", ";

                // O nome em tela quando ele existe; o codigo e o ultimo
                // recurso, para uma regra que cite materia inexistente nao
                // virar frase vazia (a leitura ja relatou o problema).
                const Materia* exigida = Por(IndiceDe(regra.materias[i]));
                frase += exigida ? exigida->nome : regra.materias[i];
            }
            return frase;
        }

        case TipoDeDesbloqueio::AprovadasNaColuna: {
            std::string frase = (regra.quantas == 1)
                                    ? "Precisa de 1 aprovacao "
                                    : "Precisa de " + std::to_string(regra.quantas) + " aprovacoes ";

            // "A coluna anterior" so quando ela e MESMO a anterior. Nada no
            // formato obriga isso, e uma frase errada seria pior que uma seca.
            frase += (regra.coluna == m->coluna - 1)
                         ? "na coluna anterior"
                         : "na coluna " + std::to_string(regra.coluna + 1);
            return frase;
        }
    }

    return {};
}

bool Lista::Desbloqueada(const int indice, const Progresso& progresso) const {

    const Materia* m = Por(indice);
    if (!m) return false;

    switch (m->desbloqueio.tipo) {

        case TipoDeDesbloqueio::Sempre:
            return true;

        case TipoDeDesbloqueio::AprovadoEm: {
            // TODAS as exigidas. Um codigo que nao existe ja foi relatado na
            // leitura; aqui ele simplesmente nao esta aprovado, o que mantem a
            // materia fechada em vez de abrir por engano.
            for (const auto& exigida : m->desbloqueio.materias) {
                const int i = IndiceDe(exigida);
                if (i < 0 || !progresso.Aprovado(i)) return false;
            }
            return true;
        }

        case TipoDeDesbloqueio::AprovadasNaColuna: {
            const auto daColuna = DaColuna(m->desbloqueio.coluna);
            int aprovadas = 0;
            for (const int i : daColuna) {
                if (progresso.Aprovado(i)) ++aprovadas;
            }
            return aprovadas >= m->desbloqueio.quantas;
        }
    }

    return false;
}

namespace {

    /// Le o bloco de desbloqueio. Devolve false quando ele e inaproveitavel.
    bool LerDesbloqueio(const nlohmann::json& j, const std::string& onde,
                        std::vector<std::string>& problemas, Desbloqueio& saida) {

        if (!j.is_object()) {
            problemas.emplace_back(onde + "\"desbloqueio\" deveria ser um objeto");
            return false;
        }

        const std::string tipo = Texto(j, "tipo");

        if (tipo == "sempre") {
            saida.tipo = TipoDeDesbloqueio::Sempre;
            return true;
        }

        if (tipo == "aprovadoEm") {
            saida.tipo = TipoDeDesbloqueio::AprovadoEm;
            if (!j.contains("materias") || !j["materias"].is_array() || j["materias"].empty()) {
                problemas.emplace_back(onde + "\"aprovadoEm\" precisa de \"materias\" com pelo"
                                              " menos um codigo");
                return false;
            }
            for (const auto& c : j["materias"]) {
                if (!c.is_string()) {
                    problemas.emplace_back(onde + "os codigos de \"materias\" precisam ser texto");
                    return false;
                }
                saida.materias.push_back(c.get<std::string>());
            }
            return true;
        }

        if (tipo == "aprovadasNaColuna") {
            saida.tipo = TipoDeDesbloqueio::AprovadasNaColuna;
            saida.quantas = Inteiro(j, "quantas", 0);
            saida.coluna  = Inteiro(j, "coluna", -1);
            if (saida.quantas <= 0) {
                problemas.emplace_back(onde + "\"quantas\" precisa ser 1 ou mais");
                return false;
            }
            if (saida.coluna < 0) {
                problemas.emplace_back(onde + "falta \"coluna\"");
                return false;
            }
            return true;
        }

        problemas.emplace_back(onde + "tipo de desbloqueio \"" + tipo + "\" nao existe"
                                      " (use sempre, aprovadoEm ou aprovadasNaColuna)");
        return false;
    }
}

Lista LerMaterias(const std::string& textoJson) {

    Lista lista;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(textoJson);
    }
    catch (const std::exception& e) {
        lista.problemas.emplace_back(std::string("o arquivo nao e JSON valido: ") + e.what());
        return lista;
    }

    if (!raiz.is_object() || !raiz.contains("materias") || !raiz["materias"].is_array()) {
        lista.problemas.emplace_back("o arquivo deveria ter uma lista \"materias\"");
        return lista;
    }

    std::set<std::string> codigosVistos;

    for (const auto& j : raiz["materias"]) {

        if (!j.is_object()) {
            lista.problemas.emplace_back("ha uma materia que nao e um objeto; descartada");
            continue;
        }

        Materia m;
        m.codigo = Texto(j, "codigo");

        if (!CodigoServe(m.codigo)) {
            lista.problemas.emplace_back("materia com codigo invalido \"" + m.codigo +
                                         "\" (so letras e numeros, sem espaco); descartada");
            continue;
        }

        const std::string onde = "materia " + m.codigo + ": ";

        if (!codigosVistos.insert(m.codigo).second) {
            // Duas materias com o mesmo codigo dividiriam a mesma nota no save.
            lista.problemas.emplace_back(onde + "codigo repetido; a segunda foi descartada");
            continue;
        }

        m.nome = Texto(j, "nome", m.codigo);
        m.chefe = Texto(j, "chefe");

        m.coluna = Inteiro(j, "coluna", -1);
        if (m.coluna < 0) {
            lista.problemas.emplace_back(onde + "falta \"coluna\" (0 e a primeira); descartada");
            continue;
        }

        if (!j.contains("desbloqueio")) {
            lista.problemas.emplace_back(onde + "falta \"desbloqueio\"; descartada");
            continue;
        }
        if (!LerDesbloqueio(j["desbloqueio"], onde, lista.problemas, m.desbloqueio)) {
            continue;
        }

        lista.materias.push_back(std::move(m));
    }

    if (lista.materias.empty()) {
        lista.problemas.emplace_back("nenhuma materia utilizavel: o jogo nao teria o que mostrar");
        return lista;
    }

    // ---- Conferencias que so dao para fazer com a lista inteira em maos ----

    // 1. Todo codigo citado numa regra existe. Um codigo errado aqui nao daria
    //    erro em jogo: a materia so ficaria fechada para sempre.
    for (const auto& m : lista.materias) {
        for (const auto& exigida : m.desbloqueio.materias) {
            if (lista.IndiceDe(exigida) < 0) {
                lista.problemas.emplace_back("materia " + m.codigo + ": o desbloqueio exige \"" +
                                             exigida + "\", que nao existe - ela nunca vai abrir");
            }
        }
    }

    // 2. Nenhuma regra e impossivel. Pedir duas aprovacoes numa coluna de uma
    //    materia so deixa aquela materia fechada para sempre, em silencio.
    for (const auto& m : lista.materias) {
        if (m.desbloqueio.tipo != TipoDeDesbloqueio::AprovadasNaColuna) continue;
        const int disponiveis = static_cast<int>(lista.DaColuna(m.desbloqueio.coluna).size());
        if (m.desbloqueio.quantas > disponiveis) {
            lista.problemas.emplace_back(
                "materia " + m.codigo + ": o desbloqueio pede " +
                std::to_string(m.desbloqueio.quantas) + " aprovacao(oes) na coluna " +
                std::to_string(m.desbloqueio.coluna) + ", que so tem " +
                std::to_string(disponiveis) + " materia(s) - ela nunca vai abrir");
        }
    }

    // 3. Alguma materia abre de inicio. Sem isto o jogo comeca com tudo trancado.
    bool algumaAbre = false;
    for (const auto& m : lista.materias) {
        if (m.desbloqueio.tipo == TipoDeDesbloqueio::Sempre) { algumaAbre = true; break; }
    }
    if (!algumaAbre) {
        lista.problemas.emplace_back("nenhuma materia tem desbloqueio \"sempre\": o jogo comecaria"
                                     " com todas trancadas e sem como abrir a primeira");
    }

    return lista;
}

}
