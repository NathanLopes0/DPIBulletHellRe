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

        // "derrubavel" so aceita true ou false. Um texto ou um numero ali seria
        // lido como falso e o processo ficaria imortal, calado - e o aluno
        // passaria a fase atirando num projetil que nao cai.
        if (j.contains("derrubavel") && !j["derrubavel"].is_boolean()) {
            problemas.emplace_back(onde + "\"derrubavel\" precisa ser true ou false; usando false");
        }
        saida.derrubavel = Booleano(j, "derrubavel", false);

        // O ABRIGO, pela mesma razao: um texto ali seria lido como falso, e a
        // capivara passaria pelo campo sem proteger ninguem.
        if (j.contains("abrigo") && !j["abrigo"].is_boolean()) {
            problemas.emplace_back(onde + "\"abrigo\" precisa ser true ou false; usando false");
        }
        saida.abrigo = Booleano(j, "abrigo", false);

        // A RESISTENCIA E UM INTEIRO DE 1 PARA CIMA, e so num derrubavel: num
        // projetil que o tiro nao derruba ela nao teria o que contar, e quem a
        // escreveu provavelmente esqueceu o "derrubavel".
        if (j.contains("resistencia")) {
            if (!j["resistencia"].is_number_integer() || j["resistencia"].get<int>() < 1) {
                problemas.emplace_back(onde + "\"resistencia\" precisa ser um inteiro de 1 para cima; usando 1");
            } else if (!saida.derrubavel) {
                problemas.emplace_back(onde + "\"resistencia\" so vale num projetil \"derrubavel\"; ignorada");
            } else {
                saida.resistencia = j["resistencia"].get<int>();
            }
        }

        // ----- O efeito de acorrentar -----
        //
        // RECUSA CHAPADO em vez de corrigir calado: um "segundos" negativo ou
        // um fator fora de (0, 1] quase sempre e engano de digitacao, e aceitar
        // produziria um aluno que anda para tras ou que fica MAIS rapido ao ser
        // acertado - coisas que ninguem ia procurar no arquivo.
        if (j.contains("acorrenta")) {
            const auto& a = j["acorrenta"];
            if (!a.is_object()) {
                problemas.emplace_back(onde + "\"acorrenta\" deveria ser um objeto com "
                                              "\"segundos\", \"fatorDeVelocidade\" e "
                                              "\"multiplicadorDaPerda\"");
            }
            else {
                saida.acorrenta.segundos = Numero(a, "segundos", 0.0f);
                saida.acorrenta.fatorDeVelocidade = Numero(a, "fatorDeVelocidade", 1.0f);
                saida.acorrenta.multiplicadorDaPerda = Numero(a, "multiplicadorDaPerda", 1.0f);

                if (saida.acorrenta.segundos <= 0.0f) {
                    problemas.emplace_back(onde + "\"acorrenta.segundos\" precisa ser maior "
                                                  "que zero; o efeito foi ignorado");
                    saida.acorrenta = DescricaoDeProjetil::Acorrentamento{};
                }
                else if (saida.acorrenta.fatorDeVelocidade <= 0.0f
                         || saida.acorrenta.fatorDeVelocidade > 1.0f) {
                    problemas.emplace_back(onde + "\"acorrenta.fatorDeVelocidade\" precisa "
                                                  "estar entre 0 (exclusivo) e 1; o efeito foi "
                                                  "ignorado");
                    saida.acorrenta = DescricaoDeProjetil::Acorrentamento{};
                }
                else if (saida.acorrenta.multiplicadorDaPerda < 0.0f) {
                    problemas.emplace_back(onde + "\"acorrenta.multiplicadorDaPerda\" nao pode "
                                                  "ser negativo; o efeito foi ignorado");
                    saida.acorrenta = DescricaoDeProjetil::Acorrentamento{};
                }
            }
        }

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
        lidos.problemas.emplace_back("a raiz deveria ser um objeto de nome -> projetil");
        return lidos;
    }

    for (auto it = raiz.begin(); it != raiz.end(); ++it) {

        const std::string nome = it.key();
        const std::string onde = "projetil \"" + nome + "\": ";

        if (nome.empty()) {
            lidos.problemas.emplace_back("ha um projetil com nome vazio");
            continue;
        }

        DescricaoDeProjetil d;
        if (LerProjetil(it.value(), onde, lidos.problemas, d)) {
            lidos.projeteis[nome] = std::move(d);
        }
    }

    // ARQUIVO SEM NENHUM PROJETIL UTILIZAVEL nao e problema de leitura, e por
    // isso nao entra na lista daqui. Quem pede os projeteis de um chefe e que
    // sabe se o chefe pode ficar sem eles - e a ponte ja recusa o chefe nesse
    // caso, com uma mensagem que diz de quem era o arquivo.
    return lidos;
}
