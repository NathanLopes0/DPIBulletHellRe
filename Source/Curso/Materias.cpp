//
// As materias do curso, lidas de texto JSON.
//

#include "Materias.h"

#include <algorithm>
#include <cctype>
#include <cmath>
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
        // A fase tutorial nao ocupa lugar na grade.
        if (materias[i].coluna == coluna && !materias[i].tutorial) saida.push_back(static_cast<int>(i));
    }
    return saida;
}

bool Lista::DoCurso(const int indice) const {
    const Materia* m = Por(indice);
    return m && !m->tutorial;
}

int Lista::QuantasDoCurso() const {
    int n = 0;
    for (const auto& m : materias) if (!m.tutorial) ++n;
    return n;
}

int Lista::Primeira() const {
    const std::vector<int> col = DaColuna(0);
    return col.empty() ? -1 : col.front();
}

int Lista::Tutorial() const {
    for (int i = 0; i < Quantas(); ++i) {
        if (materias[static_cast<size_t>(i)].tutorial) return i;
    }
    return -1;
}

int Lista::QuantasColunasObrigatorias() const {
    int maior = -1;
    for (const auto& m : materias) if (!m.optativa && !m.tutorial) maior = std::max(maior, m.coluna);
    return maior + 1;
}

int Lista::QuantasObrigatorias() const {
    int n = 0;
    for (const auto& m : materias) if (!m.optativa && !m.tutorial) ++n;
    return n;
}

std::vector<int> Lista::DoGeral() const {
    std::vector<int> contadas;
    for (int i = 0; i < Quantas(); ++i) if (DoCurso(i)) contadas.push_back(i);
    return contadas;
}

int Lista::QuantasColunas() const {
    int maior = -1;
    for (const auto& m : materias) if (!m.tutorial) maior = std::max(maior, m.coluna);
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
                // Todas as obrigatorias: "mencao no Dificil em todas as 13 obrigatorias".
                if (r.soObrigatorias && r.fracao >= 0.999f) {
                    return "menção " + MencaoNaFrase(*m) + " em todas as " + n + " obrigatórias";
                }
                return "pelo menos " + n + (r.quantas == 1 ? " menção " : " menções ") +
                       MencaoNaFrase(*m) + (r.soObrigatorias ? " nas obrigatórias" : "");
            }
            case TipoDeRequisito::Horas:
                return n + " h de atividades complementares";
            case TipoDeRequisito::Aprovadas:
                return n + (r.quantas == 1 ? " aprovação" : " aprovações") + " no curso";
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

std::string ChaveDaLiberacao(const std::string& codigo) {
    return "liberada@" + codigo;
}

std::string ChaveDaRevelacao(const std::string& codigo) {
    return "revelada@" + codigo;
}

std::string ChaveDoAtalho(const std::string& codigo) {
    return "atalho@" + codigo;
}

int Lista::PrecoDe(const int indice) const {
    const Materia* m = Por(indice);
    return m ? m->desbloqueio.preco : 0;
}

bool Lista::Comprada(const int indice, const Progresso& progresso) const {
    const Materia* m = Por(indice);
    return m && progresso.Atividades().count(ChaveDaCompra(m->codigo)) > 0;
}

bool Lista::Escondida(const int indice, const Progresso& progresso) const {
    const Materia* m = Por(indice);
    if (m && m->tutorial) return true;
    return m && m->secreta && !Desbloqueada(indice, progresso);
}

bool Lista::Desbloqueada(const int indice, const Progresso& progresso) const {
    // A secreta aberta a mao (F10) dispensa a regra.
    if (const Materia* m = Por(indice); m && m->secreta && progresso.Atividades().count(ChaveDaLiberacao(m->codigo)) > 0) {
        return true;
    }
    // Quem digitou o codigo secreto: a secreta abre com a materia do atalho aprovada.
    if (const Materia* m = Por(indice); m && m->secreta && !m->atalho.empty()
        && progresso.Atividades().count(ChaveDoAtalho(m->codigo)) > 0) {
        const int atalho = IndiceDe(m->atalho);
        if (atalho >= 0 && progresso.Aprovado(atalho)) return true;
    }
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
                if (r.soObrigatorias && Por(i)->optativa) continue;
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
            itens.push_back({std::to_string(d.quantas) + (d.quantas == 1 ? " aprovação" : " aprovações") +
                                 " na coluna " + std::to_string(d.coluna + 1) + "  (você tem " + std::to_string(tem) + ")",
                             tem >= d.quantas});
            break;
        }
    }
    for (const Requisito& r : d.requisitos) {
        std::string texto = FraseDoRequisito(r);
        if (texto.empty()) continue;
        // "pelo menos 5 mencoes de ouro" vira "5 mencoes de ouro  (tem 2)": o painel e estreito, e a
        // marca ao lado ja diz que e o minimo.
        const std::string prefixo = "pelo menos ";
        if (texto.rfind(prefixo, 0) == 0) texto.erase(0, prefixo.size());
        texto[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(texto[0])));
        itens.push_back({texto + "  (tem " + std::to_string(Contagem(r, progresso)) + ")", Cumpre(r, progresso)});
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
                        // "fracao": uma parte das materias com batalha, em vez de "quantas"; "de":
                        // "obrigatorias" deixa as optativas de fora da conta.
                        if (r.contains("fracao")) {
                            req.fracao = r["fracao"].is_number() ? r["fracao"].get<float>() : 0.0f;
                            if (req.fracao <= 0.0f || req.fracao > 1.0f) {
                                problemas.emplace_back(onde + "\"fracao\" de um requisito vai de 0 (sem ele) a 1; ignorado");
                                continue;
                            }
                        }
                        if (r.contains("de")) {
                            if (Texto(r, "de") == "obrigatorias") {
                                req.soObrigatorias = true;
                            } else {
                                problemas.emplace_back(onde + "\"de\" de um requisito so aceita \"obrigatorias\"; ignorado");
                                continue;
                            }
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

        // "secreta": nao aparece ate a regra dela estar cumprida.
        if (j.contains("secreta")) {
            if (j["secreta"].is_boolean()) {
                m.secreta = j["secreta"].get<bool>();
            } else {
                lista.problemas.emplace_back(onde + "\"secreta\" e true ou false; ficou a vista");
            }
        }

        // "tutorial": a fase da primeira entrada, fora do curso.
        if (j.contains("tutorial")) {
            if (j["tutorial"].is_boolean()) {
                m.tutorial = j["tutorial"].get<bool>();
            } else {
                lista.problemas.emplace_back(onde + "\"tutorial\" e true ou false; ficou no curso");
            }
        }

        // "atalho": a materia que abre a secreta para quem digitou o codigo secreto.
        if (j.contains("atalho")) {
            if (j["atalho"].is_string() && m.secreta) {
                m.atalho = j["atalho"].get<std::string>();
            } else {
                lista.problemas.emplace_back(onde + "\"atalho\" e o codigo de uma materia, e so na secreta; ficou sem atalho");
            }
        }

        // "musica": a musica de fundo da batalha, no lugar da de sempre.
        if (j.contains("musica")) {
            if (j["musica"].is_string()) {
                m.musica = j["musica"].get<std::string>();
            } else {
                lista.problemas.emplace_back(onde + "\"musica\" e o nome de um arquivo de Assets/Sounds; ficou a de sempre");
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

    // 0. A fracao de um requisito vira "quantas": a parte das materias com batalha (as secretas nao
    // entram na conta, e com "de": "obrigatorias" as optativas tambem nao).
    for (auto& m : lista.materias) {
        for (Requisito& r : m.desbloqueio.requisitos) {
            if (r.fracao <= 0.0f) continue;
            int universo = 0;
            for (const auto& outra : lista.materias) {
                if (outra.chefe.empty() || outra.secreta || outra.tutorial) continue;
                if (r.soObrigatorias && outra.optativa) continue;
                ++universo;
            }
            r.quantas = std::max(1, static_cast<int>(std::ceil(r.fracao * static_cast<float>(universo) - 1e-4f)));
        }
    }

    // 1.
    for (const auto& m : lista.materias) {
        for (const auto& exigida : m.desbloqueio.materias) {
            if (lista.IndiceDe(exigida) < 0) {
                lista.problemas.emplace_back("materia " + m.codigo + ": o desbloqueio exige \"" +
                                             exigida + "\", que nao existe - ela nunca vai abrir");
            }
        }
        if (!m.atalho.empty() && lista.IndiceDe(m.atalho) < 0) {
            lista.problemas.emplace_back("materia " + m.codigo + ": o atalho e \"" + m.atalho +
                                         "\", que nao existe - o codigo secreto nao a abre");
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
