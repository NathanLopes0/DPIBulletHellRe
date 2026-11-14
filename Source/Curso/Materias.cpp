//
// As materias do curso, lidas de texto JSON.
//

#include "Materias.h"

#include <algorithm>
#include <cctype>
#include <set>

#include "../Aluno/Horas.h"
#include "../Aluno/Mencoes.h"
#include "../Motor/JsonDeDados.h"

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

int Lista::QuantasColunasObrigatorias() const {
    int maior = -1;
    for (const auto& m : materias) if (!m.optativa) maior = std::max(maior, m.coluna);
    return maior + 1;
}

int Lista::QuantasObrigatorias() const {
    int n = 0;
    for (const auto& m : materias) if (!m.optativa) ++n;
    return n;
}

int Lista::QuantasColunas() const {
    int maior = -1;
    for (const auto& m : materias) maior = std::max(maior, m.coluna);
    return maior + 1;
}

namespace {

    /// "de ouro", "no Dificil", "\"intocavel\"": como o nome da mencao entra na frase.
    std::string MencaoNaFrase(const Mencoes::Mencao m) {
        switch (m) {
            case Mencoes::Mencao::Bronze:
            case Mencoes::Mencao::Prata:
            case Mencoes::Mencao::Ouro:
            case Mencoes::Mencao::NotaCheia:
                return std::string("de ") + Mencoes::Nome(m);
            case Mencoes::Mencao::Dificil:
            case Mencoes::Mencao::Dupla:
                return Mencoes::Nome(m);
            default:
                return std::string("\"") + Mencoes::Nome(m) + "\"";
        }
    }

    /// "pelo menos 1 mencao de ouro", "40 h ganhas", "6 aprovacoes no curso".
    std::string FraseDoRequisito(const Requisito& r) {
        const std::string n = std::to_string(r.quantas);
        switch (r.tipo) {
            case TipoDeRequisito::Mencao: {
                const std::optional<Mencoes::Mencao> m = Mencoes::PelaChave(r.mencao);
                if (!m) return {};
                // \xc3\xa7 e o c cedilha; \xc3\xa3 o a com til; \xc3\xb5 o o com til.
                return "pelo menos " + n + (r.quantas == 1 ? " men\xc3\xa7\xc3\xa3o " : " men\xc3\xa7\xc3\xb5" "es ") +
                       MencaoNaFrase(*m);
            }
            case TipoDeRequisito::Horas:
                return n + " h de atividades complementares";
            case TipoDeRequisito::Aprovadas:
                return n + (r.quantas == 1 ? " aprova\xc3\xa7\xc3\xa3o" : " aprova\xc3\xa7\xc3\xb5" "es") + " no curso";
        }
        return {};
    }
}

std::string Lista::ExigenciaDe(const int indice) const {

    const Materia* m = Por(indice);
    if (!m) return {};

    // A frase da regra, e depois os requisitos: "Precisa passar em INF 420, pelo menos 1 mencao de ouro".
    std::string frase = FraseDaRegra(indice);
    for (const Requisito& r : m->desbloqueio.requisitos) {
        const std::string parte = FraseDoRequisito(r);
        if (parte.empty()) continue;
        frase += frase.empty() ? "Precisa de " + parte : ", " + parte;
    }
    return frase;
}

std::string Lista::FraseDaRegra(const int indice) const {

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

                // O nome em tela quando ele existe.
                const Materia* exigida = Por(IndiceDe(regra.materias[i]));
                frase += exigida ? exigida->nome : regra.materias[i];
            }
            return frase;
        }

        case TipoDeDesbloqueio::AprovadasNaColuna: {
            std::string frase = (regra.quantas == 1)
                                    ? "Precisa de 1 aprovacao "
                                    : "Precisa de " + std::to_string(regra.quantas) + " aprovacoes ";

            // "A coluna anterior" so quando ela e mesmo a anterior.
            frase += (regra.coluna == m->coluna - 1)
                         ? "na coluna anterior"
                         : "na coluna " + std::to_string(regra.coluna + 1);
            return frase;
        }
    }

    return {};
}

bool Lista::ColunaDependeDaAnterior(const int coluna) const {

    if (coluna <= 0) return false;

    const std::vector<int> daColuna = DaColuna(coluna);
    if (daColuna.empty()) return false;

    for (const int indice : daColuna) {
        const Materia* m = Por(indice);
        if (!m) return false;

        switch (m->desbloqueio.tipo) {

            case TipoDeDesbloqueio::Sempre:
                return false;

            case TipoDeDesbloqueio::AprovadasNaColuna:
                if (m->desbloqueio.coluna != coluna - 1) return false;
                break;

            case TipoDeDesbloqueio::AprovadoEm: {
                if (m->desbloqueio.materias.empty()) return false;
                for (const auto& codigo : m->desbloqueio.materias) {
                    const Materia* exigida = Por(IndiceDe(codigo));
                    if (!exigida || exigida->coluna != coluna - 1) return false;
                }
                break;
            }
        }
    }

    return true;
}

bool Lista::DesbloqueadaParaTodos(const int indice, const std::vector<const Progresso*>& progressos) const {
    if (progressos.empty()) return false;
    for (const Progresso* p : progressos) {
        if (!p || !Desbloqueada(indice, *p)) return false;
    }
    return true;
}

std::string ChaveDaCompra(const std::string& codigo) {
    return "compra@" + codigo;
}

int Lista::PrecoDe(const int indice) const {
    const Materia* m = Por(indice);
    return m ? m->desbloqueio.preco : 0;
}

bool Lista::Comprada(const int indice, const Progresso& progresso) const {
    const Materia* m = Por(indice);
    return m && progresso.Atividades().count(ChaveDaCompra(m->codigo)) > 0;
}

bool Lista::Desbloqueada(const int indice, const Progresso& progresso) const {
    if (!RegraCumprida(indice, progresso)) return false;
    return PrecoDe(indice) <= 0 || Comprada(indice, progresso);
}

int Lista::Contagem(const Requisito& r, const Progresso& progresso) const {
    switch (r.tipo) {
        case TipoDeRequisito::Mencao: {
            const std::optional<Mencoes::Mencao> qual = Mencoes::PelaChave(r.mencao);
            if (!qual) return 0;
            int com = 0;
            for (int i = 0; i < Quantas(); ++i) {
                if (Mencoes::Da(progresso, *this, i).Tem(*qual)) ++com;
            }
            return com;
        }
        case TipoDeRequisito::Horas:
            // A carga, e nao o saldo: gastar horas numa optativa nao fecha outra.
            return Horas::De(progresso.Atividades());
        case TipoDeRequisito::Aprovadas: {
            int aprovadas = 0;
            for (int i = 0; i < Quantas(); ++i) {
                const Materia* m = Por(i);
                if (m && !m->optativa && progresso.Aprovado(i)) ++aprovadas;
            }
            return aprovadas;
        }
    }
    return 0;
}

bool Lista::Cumpre(const Requisito& r, const Progresso& progresso) const {
    if (r.tipo == TipoDeRequisito::Mencao && !Mencoes::PelaChave(r.mencao)) return false;
    return Contagem(r, progresso) >= r.quantas;
}

std::vector<ItemDaExigencia> Lista::Itens(const int indice, const Progresso& progresso) const {
    std::vector<ItemDaExigencia> itens;
    const Materia* m = Por(indice);
    if (!m) return itens;
    const Desbloqueio& d = m->desbloqueio;
    switch (d.tipo) {
        case TipoDeDesbloqueio::Sempre:
            break;
        case TipoDeDesbloqueio::AprovadoEm:
            for (const std::string& codigo : d.materias) {
                const int i = IndiceDe(codigo);
                const Materia* exigida = Por(i);
                itens.push_back({"Passar em " + (exigida ? exigida->nome : codigo), i >= 0 && progresso.Aprovado(i)});
            }
            break;
        case TipoDeDesbloqueio::AprovadasNaColuna: {
            const int tem = progresso.QuantasAprovadas(DaColuna(d.coluna));
            // \xc3\xa7\xc3\xb5 e o "coes", \xc3\xaa o e circunflexo.
            itens.push_back({std::to_string(d.quantas) + (d.quantas == 1 ? " aprova\xc3\xa7\xc3\xa3o" : " aprova\xc3\xa7\xc3\xb5" "es") +
                                 " na coluna " + std::to_string(d.coluna + 1) + "  (voc\xc3\xaa tem " + std::to_string(tem) + ")",
                             tem >= d.quantas});
            break;
        }
    }
    for (const Requisito& r : d.requisitos) {
        std::string texto = FraseDoRequisito(r);
        if (texto.empty()) continue;
        // "pelo menos 1 mencao de ouro" vira "Pelo menos 1 mencao de ouro  (voce tem 2)".
        texto[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(texto[0])));
        itens.push_back({texto + "  (voc\xc3\xaa tem " + std::to_string(Contagem(r, progresso)) + ")", Cumpre(r, progresso)});
    }
    return itens;
}

bool Lista::RegraCumprida(const int indice, const Progresso& progresso) const {

    const Materia* m = Por(indice);
    if (!m) return false;

    for (const Requisito& r : m->desbloqueio.requisitos) {
        if (!Cumpre(r, progresso)) return false;
    }

    switch (m->desbloqueio.tipo) {

        case TipoDeDesbloqueio::Sempre:
            return true;

        case TipoDeDesbloqueio::AprovadoEm: {
            // Todas as exigidas.
            for (const auto& exigida : m->desbloqueio.materias) {
                const int i = IndiceDe(exigida);
                if (i < 0 || !progresso.Aprovado(i)) return false;
            }
            return true;
        }

        case TipoDeDesbloqueio::AprovadasNaColuna: {
            return progresso.QuantasAprovadas(DaColuna(m->desbloqueio.coluna)) >= m->desbloqueio.quantas;
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

        // O preco vale para qualquer tipo: e o que se paga depois de cumprida a regra.
        saida.preco = Inteiro(j, "preco", 0);
        if (saida.preco < 0) {
            problemas.emplace_back(onde + "\"preco\" e em horas, 0 ou mais; ficou sem preco");
            saida.preco = 0;
        }

        // Os requisitos, que valem junto com a regra. O que esta errado fica de fora, com aviso -
        // mas a materia nao: sem o requisito, ela so fica mais facil de abrir.
        if (j.contains("requisitos")) {
            if (!j["requisitos"].is_array()) {
                problemas.emplace_back(onde + "\"requisitos\" deveria ser uma lista");
            } else {
                for (const auto& r : j["requisitos"]) {
                    Requisito req;
                    const std::string tipoDoRequisito = r.is_object() ? Texto(r, "tipo") : std::string();
                    req.quantas = r.is_object() ? Inteiro(r, "quantas", 1) : 0;
                    if (tipoDoRequisito == "mencao") {
                        req.tipo = TipoDeRequisito::Mencao;
                        req.mencao = Texto(r, "mencao");
                        if (!Mencoes::PelaChave(req.mencao)) {
                            problemas.emplace_back(onde + "a mencao \"" + req.mencao + "\" nao existe; requisito ignorado");
                            continue;
                        }
                    } else if (tipoDoRequisito == "horas") {
                        req.tipo = TipoDeRequisito::Horas;
                    } else if (tipoDoRequisito == "aprovadas") {
                        req.tipo = TipoDeRequisito::Aprovadas;
                    } else {
                        problemas.emplace_back(onde + "requisito \"" + tipoDoRequisito + "\" nao existe "
                                                      "(use mencao, horas ou aprovadas); ignorado");
                        continue;
                    }
                    if (req.quantas < 1) {
                        problemas.emplace_back(onde + "\"quantas\" de um requisito precisa ser 1 ou mais; ignorado");
                        continue;
                    }
                    saida.requisitos.push_back(req);
                }
            }
        }

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

        // Os dois campos de apresentacao.
        m.nomeCompleto = Texto(j, "nomeCompleto");
        m.professor = Texto(j, "professor");

        m.chefe = Texto(j, "chefe");

        // "orientacao": a batalha pede um orientador antes de comecar.
        if (j.contains("orientacao")) {
            if (j["orientacao"].is_boolean()) {
                m.orientacao = j["orientacao"].get<bool>();
            } else {
                lista.problemas.emplace_back(onde + "\"orientacao\" e true ou false; ficou sem orientador");
            }
        }

        // "optativa": vai para a ala das optativas, depois das obrigatorias.
        if (j.contains("optativa")) {
            if (j["optativa"].is_boolean()) {
                m.optativa = j["optativa"].get<bool>();
            } else {
                lista.problemas.emplace_back(onde + "\"optativa\" e true ou false; ficou obrigatoria");
            }
        }

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
        // So optativa se compra: uma obrigatoria que custasse horas trancaria o curso de quem nao as tem.
        if (m.desbloqueio.preco > 0 && !m.optativa) {
            lista.problemas.emplace_back(onde + "so optativa tem \"preco\"; esta abre so com a regra");
            m.desbloqueio.preco = 0;
        }

        lista.materias.push_back(std::move(m));
    }

    if (lista.materias.empty()) {
        lista.problemas.emplace_back("nenhuma materia utilizavel: o jogo nao teria o que mostrar");
        return lista;
    }

    // ---- Conferencias que so dao para fazer com a lista inteira em maos ----

    // 1.
    for (const auto& m : lista.materias) {
        for (const auto& exigida : m.desbloqueio.materias) {
            if (lista.IndiceDe(exigida) < 0) {
                lista.problemas.emplace_back("materia " + m.codigo + ": o desbloqueio exige \"" +
                                             exigida + "\", que nao existe - ela nunca vai abrir");
            }
        }
    }

    // 2.
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

    // 3. As optativas vem depois das obrigatorias: uma no meio delas quebraria a grade.
    const int primeiraDasOptativas = lista.QuantasColunasObrigatorias();
    for (const auto& m : lista.materias) {
        if (m.optativa && m.coluna < primeiraDasOptativas) {
            lista.problemas.emplace_back("materia " + m.codigo + ": optativa na coluna " + std::to_string(m.coluna) +
                                         ", mas as optativas vem depois das obrigatorias (coluna " +
                                         std::to_string(primeiraDasOptativas) + " em diante)");
        }
    }

    // 4.
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
