//
// O catalogo de pecas da personagem, e a aparencia escolhida por um aluno.
//

#include "Personagens.h"

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

    int ValorDoDigito(const char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }
}

namespace Personagens {

bool IdServe(const std::string& id) {
    if (id.empty()) return false;
    for (const char c : id) {
        const bool letra  = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        const bool numero = (c >= '0' && c <= '9');
        if (!letra && !numero && c != '-' && c != '_') return false;
    }
    return true;
}

bool LerCorHex(const std::string& hex, unsigned char& r, unsigned char& g, unsigned char& b) {

    // Aceita com e sem o "#" na frente: quem edita o arquivo a mao vai escrever
    // das duas formas, e recusar uma delas so geraria um bug chato de achar.
    const std::string h = (!hex.empty() && hex[0] == '#') ? hex.substr(1) : hex;
    if (h.size() != 6) return false;

    int v[6];
    for (int i = 0; i < 6; ++i) {
        v[i] = ValorDoDigito(h[static_cast<size_t>(i)]);
        if (v[i] < 0) return false;
    }

    r = static_cast<unsigned char>(v[0] * 16 + v[1]);
    g = static_cast<unsigned char>(v[2] * 16 + v[3]);
    b = static_cast<unsigned char>(v[4] * 16 + v[5]);
    return true;
}

const Peca* Categoria::PecaPor(const std::string& id) const {
    for (const auto& p : pecas) if (p.id == id) return &p;
    return nullptr;
}

const Cor* Categoria::CorPor(const std::string& id) const {
    for (const auto& c : cores) if (c.id == id) return &c;
    return nullptr;
}

const Escolha* Aparencia::Por(const std::string& categoria) const {
    const auto it = escolhas.find(categoria);
    return it == escolhas.end() ? nullptr : &it->second;
}

void Aparencia::Definir(const std::string& categoria, const Escolha& escolha) {
    escolhas[categoria] = escolha;
}

const Categoria* Catalogo::Por(const std::string& id) const {
    for (const auto& c : categorias) if (c.id == id) return &c;
    return nullptr;
}

Aparencia Catalogo::Padrao() const {

    Aparencia a;
    for (const auto& c : categorias) {
        // LerCatalogo descarta categoria sem peca ou sem cor, entao aqui as duas
        // listas tem pelo menos um item.
        a.Definir(c.id, Escolha{c.pecas.front().id, c.cores.front().id});
    }
    return a;
}

Aparencia Catalogo::Resolver(const Aparencia& pedida, std::vector<std::string>* trocas) const {

    Aparencia saida;

    for (const auto& c : categorias) {

        const Escolha* pedido = pedida.Por(c.id);

        // Sem escolha nenhuma para esta categoria: e o caso de um save gravado
        // antes de a categoria existir. Cai no padrao sem reclamar, porque nao
        // ha nada de errado nisso.
        if (pedido == nullptr) {
            saida.Definir(c.id, Escolha{c.pecas.front().id, c.cores.front().id});
            continue;
        }

        // CADA METADE CAI SOZINHA. Se o tipo de cabelo saiu do catalogo mas a
        // cor continua la, so o tipo volta ao padrao - a escolha de cor que o
        // aluno fez nao tem por que ser perdida junto.
        Escolha e = *pedido;

        if (c.PecaPor(e.peca) == nullptr) {
            if (trocas) {
                trocas->emplace_back("a peca \"" + e.peca + "\" nao existe mais em \"" + c.id +
                                     "\"; usando \"" + c.pecas.front().id + "\"");
            }
            e.peca = c.pecas.front().id;
        }

        if (c.CorPor(e.cor) == nullptr) {
            if (trocas) {
                trocas->emplace_back("a cor \"" + e.cor + "\" nao existe mais em \"" + c.id +
                                     "\"; usando \"" + c.cores.front().id + "\"");
            }
            e.cor = c.cores.front().id;
        }

        saida.Definir(c.id, e);
    }

    // Escolhas de categorias que nao existem mais somem junto com elas.
    for (const auto& par : pedida.escolhas) {
        if (Por(par.first) == nullptr && trocas) {
            trocas->emplace_back("a categoria \"" + par.first + "\" nao existe mais;"
                                 " a escolha dela foi descartada");
        }
    }

    return saida;
}

std::vector<Camada> Catalogo::Camadas(const Aparencia& aparencia) const {

    const Aparencia valida = Resolver(aparencia);

    std::vector<Camada> camadas;

    for (const auto& c : categorias) {
        const Escolha* e = valida.Por(c.id);
        if (e == nullptr) continue;

        const Peca* p = c.PecaPor(e->peca);
        const Cor* cor = c.CorPor(e->cor);
        if (p == nullptr || cor == nullptr) continue;   // Resolver ja garantiu

        Camada camada;
        camada.arte = p->arte;
        camada.ordem = c.ordem;
        camada.tingida = true;
        camada.r = cor->r;
        camada.g = cor->g;
        camada.b = cor->b;
        camadas.push_back(camada);
    }

    for (const auto& f : fixas) {
        Camada camada;
        camada.arte = f.arte;
        camada.ordem = f.ordem;
        camada.tingida = false;
        camadas.push_back(camada);
    }

    // Ordenacao ESTAVEL: duas camadas com a mesma ordem mantem a ordem em que
    // entraram, em vez de trocarem de lugar entre uma execucao e outra. O
    // arquivo nao devia ter ordens repetidas - LerCatalogo reclama quando tem -
    // mas se tiver, o desenho ao menos nao muda sozinho.
    std::stable_sort(camadas.begin(), camadas.end(),
                     [](const Camada& a, const Camada& b) { return a.ordem < b.ordem; });

    return camadas;
}

namespace {

    /// Le as cores de uma categoria. Devolve quantas foram descartadas.
    void LerCores(const nlohmann::json& j, Categoria& cat, const std::string& onde,
                  std::vector<std::string>& problemas) {

        if (!j.contains("cores") || !j["cores"].is_array()) {
            problemas.emplace_back(onde + "falta \"cores\"");
            return;
        }

        std::set<std::string> vistos;
        for (const auto& jc : j["cores"]) {

            if (!jc.is_object()) {
                problemas.emplace_back(onde + "ha uma cor que nao e um objeto; descartada");
                continue;
            }

            Cor cor;
            cor.id = Texto(jc, "id");
            cor.nome = Texto(jc, "nome", cor.id);

            if (!IdServe(cor.id)) {
                problemas.emplace_back(onde + "cor com \"id\" invalido (\"" + cor.id +
                                       "\"); descartada");
                continue;
            }
            if (!vistos.insert(cor.id).second) {
                problemas.emplace_back(onde + "a cor \"" + cor.id + "\" aparece duas vezes;"
                                       " a segunda foi descartada");
                continue;
            }
            if (!LerCorHex(Texto(jc, "rgb"), cor.r, cor.g, cor.b)) {
                problemas.emplace_back(onde + "a cor \"" + cor.id + "\" tem \"rgb\" invalido"
                                       " (esperado rrggbb); descartada");
                continue;
            }

            cat.cores.push_back(cor);
        }
    }

    void LerPecas(const nlohmann::json& j, Categoria& cat, const std::string& onde,
                  std::vector<std::string>& problemas) {

        if (!j.contains("pecas") || !j["pecas"].is_array()) {
            problemas.emplace_back(onde + "falta \"pecas\"");
            return;
        }

        std::set<std::string> vistos;
        for (const auto& jp : j["pecas"]) {

            if (!jp.is_object()) {
                problemas.emplace_back(onde + "ha uma peca que nao e um objeto; descartada");
                continue;
            }

            Peca p;
            p.id = Texto(jp, "id");
            p.nome = Texto(jp, "nome", p.id);
            p.arte = Texto(jp, "arte");

            if (!IdServe(p.id)) {
                problemas.emplace_back(onde + "peca com \"id\" invalido (\"" + p.id +
                                       "\"); descartada");
                continue;
            }
            if (!vistos.insert(p.id).second) {
                problemas.emplace_back(onde + "a peca \"" + p.id + "\" aparece duas vezes;"
                                       " a segunda foi descartada");
                continue;
            }
            if (p.arte.empty()) {
                problemas.emplace_back(onde + "a peca \"" + p.id + "\" nao diz \"arte\";"
                                       " descartada");
                continue;
            }

            cat.pecas.push_back(p);
        }
    }
}

Catalogo LerCatalogo(const std::string& textoJson) {

    Catalogo cat;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(textoJson);
    }
    catch (const std::exception& e) {
        cat.problemas.emplace_back(std::string("personagens.json nao e um JSON valido: ") + e.what());
        return cat;
    }

    if (!raiz.is_object()) {
        cat.problemas.emplace_back("personagens.json deveria ser um objeto");
        return cat;
    }

    // ----- categorias -----
    if (!raiz.contains("categorias") || !raiz["categorias"].is_array()) {
        cat.problemas.emplace_back("falta \"categorias\": sem elas nao ha o que escolher");
        return cat;
    }

    std::set<std::string> idsVistos;
    for (const auto& jc : raiz["categorias"]) {

        if (!jc.is_object()) {
            cat.problemas.emplace_back("ha uma categoria que nao e um objeto; descartada");
            continue;
        }

        Categoria c;
        c.id = Texto(jc, "id");
        c.nome = Texto(jc, "nome", c.id);
        c.ordem = Inteiro(jc, "ordem", 0);

        if (!IdServe(c.id)) {
            cat.problemas.emplace_back("categoria com \"id\" invalido (\"" + c.id +
                                       "\"); descartada");
            continue;
        }
        if (!idsVistos.insert(c.id).second) {
            cat.problemas.emplace_back("a categoria \"" + c.id + "\" aparece duas vezes;"
                                       " a segunda foi descartada");
            continue;
        }

        const std::string onde = "categoria " + c.id + ": ";
        LerPecas(jc, c, onde, cat.problemas);
        LerCores(jc, c, onde, cat.problemas);

        // Sem peca ou sem cor nao ha escolha possivel, e uma categoria assim na
        // tela seria uma linha que nao responde a nada.
        if (c.pecas.empty() || c.cores.empty()) {
            cat.problemas.emplace_back(onde + "ficou sem peca ou sem cor; a categoria inteira"
                                       " foi descartada");
            continue;
        }

        cat.categorias.push_back(c);
    }

    // ----- camadas fixas -----
    if (raiz.contains("fixas") && raiz["fixas"].is_array()) {
        for (const auto& jf : raiz["fixas"]) {
            if (!jf.is_object()) continue;
            CamadaFixa f;
            f.arte = Texto(jf, "arte");
            f.ordem = Inteiro(jf, "ordem", 0);
            if (f.arte.empty()) {
                cat.problemas.emplace_back("ha uma camada fixa sem \"arte\"; descartada");
                continue;
            }
            cat.fixas.push_back(f);
        }
    }

    // ----- ordens repetidas -----
    //
    // Nao descarta nada: so avisa. Duas camadas na mesma ordem continuam
    // desenhando, mas qual fica por cima deixa de ser uma decisao de quem
    // escreveu o arquivo.
    {
        std::map<int, std::string> porOrdem;
        for (const auto& c : cat.categorias) {
            const auto achado = porOrdem.find(c.ordem);
            if (achado != porOrdem.end()) {
                cat.problemas.emplace_back("\"" + c.id + "\" e \"" + achado->second +
                                           "\" tem a mesma ordem de desenho (" +
                                           std::to_string(c.ordem) + ")");
            }
            else porOrdem[c.ordem] = c.id;
        }
        for (const auto& f : cat.fixas) {
            const auto achado = porOrdem.find(f.ordem);
            if (achado != porOrdem.end()) {
                cat.problemas.emplace_back("\"" + f.arte + "\" e \"" + achado->second +
                                           "\" tem a mesma ordem de desenho (" +
                                           std::to_string(f.ordem) + ")");
            }
            else porOrdem[f.ordem] = f.arte;
        }
    }

    // ----- combinacoes prontas -----
    if (raiz.contains("predefinidas") && raiz["predefinidas"].is_array()) {

        std::set<std::string> vistos;
        for (const auto& jp : raiz["predefinidas"]) {

            if (!jp.is_object()) continue;

            Predefinida p;
            p.id = Texto(jp, "id");
            p.nome = Texto(jp, "nome", p.id);

            if (!IdServe(p.id)) {
                cat.problemas.emplace_back("combinacao com \"id\" invalido (\"" + p.id +
                                           "\"); descartada");
                continue;
            }
            if (!vistos.insert(p.id).second) {
                cat.problemas.emplace_back("a combinacao \"" + p.id + "\" aparece duas vezes;"
                                           " a segunda foi descartada");
                continue;
            }

            if (jp.contains("escolhas") && jp["escolhas"].is_object()) {
                for (auto it = jp["escolhas"].begin(); it != jp["escolhas"].end(); ++it) {
                    if (!it.value().is_object()) continue;
                    p.aparencia.Definir(it.key(), Escolha{Texto(it.value(), "peca"),
                                                          Texto(it.value(), "cor")});
                }
            }

            // Passa pelo mesmo conserto que um save sofreria. Assim uma
            // combinacao que cita peca inexistente vira uma combinacao valida e
            // o problema aparece aqui, e nao so quando alguem a escolher.
            std::vector<std::string> trocas;
            p.aparencia = cat.Resolver(p.aparencia, &trocas);
            for (const auto& t : trocas) {
                cat.problemas.emplace_back("combinacao " + p.id + ": " + t);
            }

            cat.predefinidas.push_back(p);
        }
    }

    return cat;
}

}
