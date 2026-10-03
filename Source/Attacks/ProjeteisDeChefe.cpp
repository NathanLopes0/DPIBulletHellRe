//
// Os tipos de projetil de cada chefe, lidos de texto JSON.
//

#include "ProjeteisDeChefe.h"

#include <set>

#include "../JsonDeDados.h"

namespace {

    const std::vector<std::string> kDimensoes = {"largura", "altura"};

    bool Contem(const std::vector<std::string>& lista, const std::string& v) {
        for (const auto& x : lista) if (x == v) return true;
        return false;
    }

    std::string Texto(const nlohmann::json& j, const char* nome, const std::string& padrao = "") {
        return (j.contains(nome) && j[nome].is_string()) ? j[nome].get<std::string>() : padrao;
    }

    float Numero(const nlohmann::json& j, const char* nome, const float padrao) {
        return (j.contains(nome) && j[nome].is_number()) ? j[nome].get<float>() : padrao;
    }

    bool Booleano(const nlohmann::json& j, const char* nome, const bool padrao) {
        return (j.contains(nome) && j[nome].is_boolean()) ? j[nome].get<bool>() : padrao;
    }

    /// Le a lista de animacoes. Devolve false quando nao sobrou nenhuma util -
    /// um projetil sem animacao nao tem como ser desenhado, entao a entrada
    /// inteira cai.
    bool LerAnimacoes(const nlohmann::json& j, const std::string& onde,
                      std::vector<std::string>& problemas,
                      std::vector<AnimacaoDeProjetil>& saida) {

        if (!j.contains("animacoes")) {
            problemas.emplace_back(onde + "falta \"animacoes\" (um objeto de nome -> lista de quadros)");
            return false;
        }
        if (!j["animacoes"].is_object()) {
            problemas.emplace_back(onde + "\"animacoes\" deveria ser um objeto de nome -> lista de quadros");
            return false;
        }

        std::set<std::string> vistos;

        for (auto it = j["animacoes"].begin(); it != j["animacoes"].end(); ++it) {

            const std::string nome = it.key();
            const std::string ondeAnim = onde + "animacao \"" + nome + "\": ";

            if (nome.empty()) {
                problemas.emplace_back(onde + "ha uma animacao com nome vazio");
                continue;
            }
            // nlohmann guarda um objeto num mapa ordenado, entao chave repetida no
            // texto nem chega aqui - a ultima vence silenciosamente. Este teste
            // pega o caso que SOBRA: duas animacoes cujo nome difere so por algo
            // que o jogo trata como igual nao existe hoje, mas a checagem custa
            // nada e documenta a intencao.
            if (!vistos.insert(nome).second) {
                problemas.emplace_back(ondeAnim + "nome repetido");
                continue;
            }

            if (!it.value().is_array()) {
                problemas.emplace_back(ondeAnim + "deveria ser uma lista de indices de quadro");
                continue;
            }

            AnimacaoDeProjetil anim;
            anim.nome = nome;

            bool quadroRuim = false;
            for (const auto& q : it.value()) {
                if (!q.is_number_integer()) {
                    problemas.emplace_back(ondeAnim + "os quadros precisam ser numeros inteiros");
                    quadroRuim = true;
                    break;
                }
                const int indice = q.get<int>();
                if (indice < 0) {
                    problemas.emplace_back(ondeAnim + "quadro negativo (" + std::to_string(indice) + ")");
                    quadroRuim = true;
                    break;
                }
                anim.quadros.push_back(indice);
            }
            if (quadroRuim) continue;

            if (anim.quadros.empty()) {
                problemas.emplace_back(ondeAnim + "lista de quadros vazia");
                continue;
            }

            saida.push_back(std::move(anim));
        }

        if (saida.empty()) {
            problemas.emplace_back(onde + "nenhuma animacao utilizavel, entao este projetil"
                                          " foi descartado (sem animacao nao ha o que desenhar)");
            return false;
        }

        return true;
    }

    /// Le uma entrada de projetil. Devolve false quando ela e inaproveitavel.
    bool LerProjetil(const nlohmann::json& j, const std::string& onde,
                     std::vector<std::string>& problemas, DescricaoDeProjetil& saida) {

        if (!j.is_object()) {
            problemas.emplace_back(onde + "deveria ser um objeto");
            return false;
        }

        saida.sprite = Texto(j, "sprite");
        saida.dados  = Texto(j, "dados");

        if (saida.sprite.empty()) {
            problemas.emplace_back(onde + "falta \"sprite\" (o caminho do .png relativo a Assets)");
            return false;
        }
        if (saida.dados.empty()) {
            problemas.emplace_back(onde + "falta \"dados\" (o caminho do .json do atlas, relativo a Assets)");
            return false;
        }

        if (!LerAnimacoes(j, onde, problemas, saida.animacoes)) {
            return false;
        }

        saida.escala = Numero(j, "escala", 1.0f);
        if (saida.escala <= 0.0f) {
            problemas.emplace_back(onde + "\"escala\" precisa ser maior que zero; usando 1");
            saida.escala = 1.0f;
        }

        if (j.contains("ordemDeDesenho")) {
            if (j["ordemDeDesenho"].is_number_integer()) {
                saida.ordemDeDesenho = j["ordemDeDesenho"].get<int>();
            } else {
                problemas.emplace_back(onde + "\"ordemDeDesenho\" precisa ser um numero inteiro; usando 100");
            }
        }

        saida.colisorDimensao = Texto(j, "colisorDimensao", "largura");
        if (!DimensaoDeColisorExiste(saida.colisorDimensao)) {
            problemas.emplace_back(onde + "\"colisorDimensao\" \"" + saida.colisorDimensao +
                                   "\" nao existe (use largura ou altura); usando largura");
            saida.colisorDimensao = "largura";
        }

        saida.colisorDivisor = Numero(j, "colisorDivisor", 2.0f);
        if (saida.colisorDivisor <= 0.0f) {
            problemas.emplace_back(onde + "\"colisorDivisor\" precisa ser maior que zero; usando 2");
            saida.colisorDivisor = 2.0f;
        }

        saida.posicionarNoDono = Booleano(j, "posicionarNoDono", true);
        saida.rotacionarComAVelocidade = Booleano(j, "rotacionarComAVelocidade", false);

        saida.margemEmSprites = Numero(j, "margemEmSprites", 1.0f);
        if (saida.margemEmSprites < 0.0f) {
            problemas.emplace_back(onde + "\"margemEmSprites\" nao pode ser negativa; usando 1");
            saida.margemEmSprites = 1.0f;
        }

        // Zero e VALIDO aqui, e quer dizer "sem parcela proporcional a tela" - e o
        // que os baloes do Andre usam. Negativo nao: encolheria a area viva para
        // dentro da tela e o projetil morreria a vista do jogador.
        saida.margemDivisorDeTela = Numero(j, "margemDivisorDeTela", 12.0f);
        if (saida.margemDivisorDeTela < 0.0f) {
            problemas.emplace_back(onde + "\"margemDivisorDeTela\" nao pode ser negativa"
                                          " (use 0 para nenhuma folga de tela); usando 12");
            saida.margemDivisorDeTela = 12.0f;
        }

        // A animacao inicial e resolvida AQUI, e nao na ponte: assim quem le a
        // descricao - teste incluido - ve o valor que o jogo vai usar, em vez de
        // uma string vazia que alguem mais adiante interpreta.
        saida.animacaoInicial = Texto(j, "animacaoInicial");
        if (saida.animacaoInicial.empty()) {
            saida.animacaoInicial = saida.animacoes.front().nome;
        } else {
            bool existe = false;
            for (const auto& a : saida.animacoes) {
                if (a.nome == saida.animacaoInicial) { existe = true; break; }
            }
            if (!existe) {
                problemas.emplace_back(onde + "\"animacaoInicial\" \"" + saida.animacaoInicial +
                                       "\" nao esta entre as animacoes deste projetil; usando \"" +
                                       saida.animacoes.front().nome + "\"");
                saida.animacaoInicial = saida.animacoes.front().nome;
            }
        }

        return true;
    }
}

bool DimensaoDeColisorExiste(const std::string& nome) {
    return Contem(kDimensoes, nome);
}

ProjeteisLidos LerProjeteis(const std::string& textoJson) {

    ProjeteisLidos lidos;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(textoJson);
    } catch (const std::exception& e) {
        lidos.problemas.emplace_back(std::string("o arquivo nao e JSON valido: ") + e.what());
        return lidos;
    }

    if (!raiz.is_object()) {
        lidos.problemas.emplace_back("a raiz deveria ser um objeto de conjuntos (um por chefe)");
        return lidos;
    }

    for (auto conjunto = raiz.begin(); conjunto != raiz.end(); ++conjunto) {

        const std::string nomeDoConjunto = conjunto.key();

        if (!conjunto.value().is_object()) {
            lidos.problemas.emplace_back("conjunto \"" + nomeDoConjunto +
                                         "\" deveria ser um objeto de nome -> projetil");
            continue;
        }

        std::map<std::string, DescricaoDeProjetil> projeteis;

        for (auto it = conjunto.value().begin(); it != conjunto.value().end(); ++it) {

            const std::string nome = it.key();
            const std::string onde = "conjunto \"" + nomeDoConjunto + "\", projetil \"" + nome + "\": ";

            if (nome.empty()) {
                lidos.problemas.emplace_back("conjunto \"" + nomeDoConjunto +
                                             "\": ha um projetil com nome vazio");
                continue;
            }

            DescricaoDeProjetil d;
            if (LerProjetil(it.value(), onde, lidos.problemas, d)) {
                projeteis[nome] = std::move(d);
            }
        }

        if (projeteis.empty()) {
            lidos.problemas.emplace_back("conjunto \"" + nomeDoConjunto +
                                         "\" ficou sem nenhum projetil utilizavel");
            continue;
        }

        lidos.conjuntos[nomeDoConjunto] = std::move(projeteis);
    }

    return lidos;
}
