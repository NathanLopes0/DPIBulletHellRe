//
// Fases de ataque de um chefe, lidas de texto JSON.
//

#include "FasesDeAtaque.h"

#include "../JsonDeDados.h"

namespace {

    const std::vector<std::string> kEstrategias = {
        "AngledAttack", "CircleSpreadAttack", "WaveAttack", "BaloonAttack", "LaserAttack"
    };
    const std::vector<std::string> kMovimentos = {
        "RandomWander", "HoverAbovePlayer", "GoToCenter"
    };
    /// "None" NAO entra: a propria BaloonAttack recusa side == None.
    const std::vector<std::string> kLadosDeBalao = {"Down", "Left", "Right", "Up"};
    /// Os nomes que a maquina de estados procura por texto. Nao sao livres.
    const std::vector<std::string> kNomesDeFase = {
        "StateOne", "StateTwo", "StateThree", "StateFinal"
    };

    bool Contem(const std::vector<std::string>& lista, const std::string& v) {
        for (const auto& x : lista) if (x == v) return true;
        return false;
    }

    float Campo(const nlohmann::json& j, const char* nome, const float padrao) {
        return (j.contains(nome) && j[nome].is_number()) ? j[nome].get<float>() : padrao;
    }

    std::string Texto(const nlohmann::json& j, const char* nome, const std::string& padrao = "") {
        return (j.contains(nome) && j[nome].is_string()) ? j[nome].get<std::string>() : padrao;
    }

    /// Le um numero OPCIONAL. Ausente devolve nullopt, e nao um padrao inventado:
    /// e o que garante que a ponte nao sobrescreva o padrao do AttackParams.
    std::optional<float> Opcional(const nlohmann::json& j, const char* nome) {
        if (j.contains(nome) && j[nome].is_number()) return j[nome].get<float>();
        return std::nullopt;
    }

    DescricaoDeMovimento LerMovimento(const nlohmann::json& j, const std::string& onde,
                                      std::vector<std::string>& problemas) {
        DescricaoDeMovimento m;
        if (!j.is_object()) {
            problemas.emplace_back(onde + "\"movimento\" deveria ser um objeto");
            return m;
        }
        m.tipo = Texto(j, "tipo", "GoToCenter");
        if (!MovimentoExiste(m.tipo)) {
            problemas.emplace_back(onde + "movimento \"" + m.tipo +
                                   "\" nao existe (use RandomWander, HoverAbovePlayer ou GoToCenter)");
            m.tipo = "GoToCenter";
        }
        m.a = Campo(j, "a", 0.0f);
        m.b = Campo(j, "b", 0.0f);
        return m;
    }

    std::optional<bool> OpcionalBool(const nlohmann::json& j, const char* nome) {
        if (j.contains(nome) && j[nome].is_boolean()) return j[nome].get<bool>();
        return std::nullopt;
    }

    /// Le o bloco de balao. Devolve false quando ele e inaproveitavel.
    bool LerBalao(const nlohmann::json& j, const std::string& onde,
                  std::vector<std::string>& problemas, DescricaoDeBalao& saida) {

        if (!j.is_object()) {
            problemas.emplace_back(onde + "\"balao\" deveria ser um objeto");
            return false;
        }

        saida.lado = Texto(j, "lado");
        if (!LadoDeBalaoExiste(saida.lado)) {
            problemas.emplace_back(onde + "\"balao\" precisa de \"lado\": Down, Left, Right ou Up"
                                          " (a propria BaloonAttack recusa None e nao dispara nada)");
            return false;
        }

        saida.spawnAleatorio    = OpcionalBool(j, "spawnAleatorio");
        saida.centradoNoJogador = OpcionalBool(j, "centradoNoJogador");
        saida.deslocamento      = Opcional(j, "deslocamento");

        if (j.contains("pontosDeSpawn")) {
            if (!j["pontosDeSpawn"].is_array()) {
                problemas.emplace_back(onde + "\"pontosDeSpawn\" deveria ser uma lista de pares");
                return false;
            }
            for (const auto& p : j["pontosDeSpawn"]) {
                if (!p.is_array() || p.size() != 2 || !p[0].is_number() || !p[1].is_number()) {
                    problemas.emplace_back(onde + "cada ponto de \"pontosDeSpawn\" deveria ser [x, y]");
                    return false;
                }
                saida.pontosDeSpawn.emplace_back(p[0].get<float>(), p[1].get<float>());
            }
        }

        // O modo de posicoes exatas tem duas exigencias que a BaloonAttack
        // confere em tempo de execucao, escrevendo no log e nao disparando nada.
        // Conferir aqui troca um ataque invisivel por uma frase no arquivo.
        const bool aleatorio = saida.spawnAleatorio.value_or(false);   // igual ao padrao do struct
        if (!aleatorio) {
            if (saida.pontosDeSpawn.empty()) {
                problemas.emplace_back(onde + "com \"spawnAleatorio\" falso e preciso dar "
                                              "\"pontosDeSpawn\", senao o ataque nao dispara nada");
                return false;
            }
            if (saida.centradoNoJogador.value_or(true) != false) {
                // centerOnPlayer nasce TRUE no BaloonAttackParams, e a estrategia
                // recusa a combinacao. Omitir o campo aqui seria aceitar um
                // ataque que nunca dispara.
                problemas.emplace_back(onde + "com \"spawnAleatorio\" falso e preciso dizer "
                                              "\"centradoNoJogador\": false - a BaloonAttack recusa "
                                              "as duas coisas juntas");
                return false;
            }
        }

        return true;
    }

    /**
     * Procura, entre as regras EM LINHA de um ataque, o ritmo da investida.
     *
     * Devolve nullopt quando nenhuma regra tem MiraPeriodica. So olha as regras
     * em linha de proposito: uma regra apontada por nome vive em regras.json, e
     * derivar o cooldown de um ritmo escrito em OUTRO arquivo criaria uma
     * dependencia invisivel entre os dois - mexer em regras.json mudaria a
     * cadencia de um ataque sem nenhum sinal em fases.json.
     */
    std::optional<RitmoDeCiclos> RitmoDaInvestida(const std::vector<Regra>& regras) {
        for (const Regra& r : regras) {
            if (r.temMotion && r.motion.tipo == "MiraPeriodica") return r.motion.ritmo;
        }
        return std::nullopt;
    }

    /// Le o cooldown nas duas formas: numero, ou derivado do ritmo da investida.
    bool LerCooldown(const nlohmann::json& j, const std::string& onde,
                     DescricaoDeAtaque& saida, std::vector<std::string>& problemas) {

        if (j.contains("cooldown") && j["cooldown"].is_object()) {

            const auto& c = j["cooldown"];
            const std::string base = Texto(c, "doRitmo");
            if (base != "total" && base != "ciclo") {
                problemas.emplace_back(onde + "\"cooldown\" como objeto precisa de "
                                              "\"doRitmo\": \"total\" ou \"ciclo\"");
                return false;
            }

            const auto ritmo = RitmoDaInvestida(saida.regras);
            if (!ritmo) {
                problemas.emplace_back(onde + "\"cooldown\" pede o ritmo, mas este ataque nao tem "
                                              "uma \"investidaRepetida\" nas regras EM LINHA. "
                                              "Escreva as regras aqui mesmo, ou use um numero.");
                return false;
            }

            const float valorBase = (base == "total") ? ritmo->DuracaoTotal() : ritmo->Ciclo();
            const float divisor = Campo(c, "dividirPor", 1.0f);
            if (divisor <= 0.0f) {
                problemas.emplace_back(onde + "\"dividirPor\" precisa ser maior que zero");
                return false;
            }
            saida.cooldown = valorBase / divisor + Campo(c, "mais", 0.0f);
        }
        else {
            saida.cooldown = Campo(j, "cooldown", 1.0f);
        }

        if (saida.cooldown <= 0.0f) {
            // Cooldown zero dispararia uma rajada por quadro e encheria o pool
            // em menos de um segundo.
            problemas.emplace_back(onde + "\"cooldown\" precisa ser maior que zero");
            return false;
        }
        return true;
    }

    /// Le um ataque. Devolve false quando a descricao e inaproveitavel.
    bool LerAtaque(const nlohmann::json& j, const std::string& onde,
                   std::vector<std::string>& problemas, DescricaoDeAtaque& saida) {

        if (!j.is_object()) {
            problemas.emplace_back(onde + "deveria ser um objeto");
            return false;
        }

        saida.estrategia = Texto(j, "estrategia");
        if (saida.estrategia.empty()) {
            problemas.emplace_back(onde + "falta \"estrategia\"");
            return false;
        }
        if (!EstrategiaExiste(saida.estrategia)) {
            problemas.emplace_back(onde + "estrategia \"" + saida.estrategia +
                                   "\" nao existe (use AngledAttack, CircleSpreadAttack, "
                                   "WaveAttack, BaloonAttack ou LaserAttack)");
            return false;
        }

        saida.projetil = Texto(j, "projetil");
        if (saida.projetil.empty()) {
            problemas.emplace_back(onde + "falta \"projetil\", o nome da fabrica de projeteis "
                                          "deste chefe");
            return false;
        }

        if (j.contains("projeteis")) {
            if (!j["projeteis"].is_number_integer()) {
                problemas.emplace_back(onde + "\"projeteis\" precisa ser um inteiro");
                return false;
            }
            const int n = j["projeteis"].get<int>();
            if (n < 1) {
                problemas.emplace_back(onde + "\"projeteis\" precisa ser 1 ou mais");
                return false;
            }
            saida.projeteis = n;
        }

        saida.velocidade    = Opcional(j, "velocidade");
        saida.angulo        = Opcional(j, "angulo");
        saida.anguloCentral = Opcional(j, "anguloCentral");
        saida.intervalo     = Opcional(j, "intervalo");

        // "regras" aceita as duas formas: o nome de um conjunto de regras.json,
        // ou a lista escrita aqui mesmo. Lido ANTES do cooldown porque um
        // cooldown derivado precisa do ritmo que mora nas regras em linha.
        if (j.contains("regras")) {
            if (j["regras"].is_string()) {
                saida.regrasNome = j["regras"].get<std::string>();
            }
            else if (j["regras"].is_array()) {
                // Reaproveita o parser de regras em vez de duplica-lo: o array e
                // reserializado e passa pelo mesmo caminho de regras.json.
                saida.regras = LerListaDeRegras(j["regras"].dump(), onde, problemas);
                saida.temRegrasEmLinha = true;
            }
            else {
                problemas.emplace_back(onde + "\"regras\" deveria ser um nome ou uma lista");
                return false;
            }
        }

        if (!LerCooldown(j, onde, saida, problemas)) return false;

        // --- bloco de balao ---
        const bool ehBalao = (saida.estrategia == "BaloonAttack");
        if (j.contains("balao")) {
            if (!ehBalao) {
                // Nao e fatal: o bloco simplesmente nao seria lido por ninguem.
                // Mas avisar evita alguem passar uma tarde ajustando "lado" num
                // AngledAttack e nao ver nada mudar.
                problemas.emplace_back(onde + "so a BaloonAttack usa \"balao\"; o bloco foi ignorado");
            }
            else {
                DescricaoDeBalao b;
                if (!LerBalao(j["balao"], onde, problemas, b)) return false;
                saida.balao = b;
            }
        }
        else if (ehBalao) {
            problemas.emplace_back(onde + "um ataque BaloonAttack precisa do bloco \"balao\": sem "
                                          "ele a estrategia recusa os parametros e nao dispara nada");
            return false;
        }

        return true;
    }

}

bool EstrategiaExiste(const std::string& nome)   { return Contem(kEstrategias, nome); }
bool MovimentoExiste(const std::string& nome)    { return Contem(kMovimentos, nome); }
bool NomeDeFaseExiste(const std::string& nome)   { return Contem(kNomesDeFase, nome); }
bool LadoDeBalaoExiste(const std::string& nome)  { return Contem(kLadosDeBalao, nome); }

FasesLidas LerFases(const std::string& textoJson) {

    FasesLidas saida;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(textoJson);
    }
    catch (const std::exception& e) {
        saida.problemas.emplace_back(std::string("o arquivo nao e um JSON valido: ") + e.what());
        return saida;
    }
    if (!raiz.is_object()) {
        saida.problemas.emplace_back("o arquivo deveria ser um objeto com um conjunto de fases por chefe");
        return saida;
    }

    for (auto itConj = raiz.begin(); itConj != raiz.end(); ++itConj) {

        const std::string& nomeConj = itConj.key();
        const auto& corpoConj = itConj.value();

        if (!corpoConj.is_object()) {
            saida.problemas.emplace_back("conjunto \"" + nomeConj +
                                         "\": deveria ser um objeto com uma fase por chave");
            continue;
        }

        std::vector<DescricaoDeFase> fases;

        for (auto itFase = corpoConj.begin(); itFase != corpoConj.end(); ++itFase) {

            const std::string& nomeFase = itFase.key();
            const auto& corpoFase = itFase.value();
            const std::string onde = "conjunto \"" + nomeConj + "\", fase \"" + nomeFase + "\": ";

            if (!corpoFase.is_object()) {
                saida.problemas.emplace_back(onde + "deveria ser um objeto");
                continue;
            }
            if (!NomeDeFaseExiste(nomeFase)) {
                // Nome livre nao serve: a FSM procura os estados por texto, e um
                // nome inventado nunca seria alcancado.
                saida.problemas.emplace_back(onde + "nome de fase invalido. Use StateOne, "
                                             "StateTwo, StateThree ou StateFinal");
                continue;
            }

            DescricaoDeFase f;
            f.nome = nomeFase;
            f.duracao = Campo(corpoFase, "duracao", 17.0f);
            f.proximo = Texto(corpoFase, "proximo");

            if (f.duracao <= 0.0f) {
                saida.problemas.emplace_back(onde + "\"duracao\" precisa ser maior que zero");
                continue;
            }

            if (corpoFase.contains("movimento")) {
                f.movimento = LerMovimento(corpoFase["movimento"], onde, saida.problemas);
            }

            if (!corpoFase.contains("ataques") || !corpoFase["ataques"].is_array()) {
                saida.problemas.emplace_back(onde + "falta \"ataques\", que deveria ser uma lista");
                continue;
            }

            const auto& lista = corpoFase["ataques"];
            for (size_t k = 0; k < lista.size(); ++k) {
                DescricaoDeAtaque a;
                const std::string ondeAtaque = onde + "ataque " + std::to_string(k + 1) + ": ";
                if (LerAtaque(lista[k], ondeAtaque, saida.problemas, a)) {
                    f.ataques.push_back(std::move(a));
                }
            }

            if (f.ataques.empty()) {
                // Uma fase sem ataque nenhum e um chefe parado por 17 segundos.
                saida.problemas.emplace_back(onde + "nenhum ataque valido, a fase foi descartada");
                continue;
            }

            fases.push_back(std::move(f));
        }

        saida.conjuntos.emplace(nomeConj, std::move(fases));
    }

    return saida;
}

std::vector<std::string> ValidarTransicoes(const std::vector<DescricaoDeFase>& fases,
                                           const std::string& nomeDoConjunto) {

    std::vector<std::string> problemas;
    const std::string onde = "conjunto \"" + nomeDoConjunto + "\": ";

    if (fases.empty()) {
        problemas.emplace_back(onde + "nenhuma fase valida");
        return problemas;
    }

    bool temStateOne = false, temStateFinal = false;
    for (const auto& f : fases) {
        if (f.nome == "StateOne")   temStateOne = true;
        if (f.nome == "StateFinal") temStateFinal = true;
    }

    if (!temStateOne) {
        problemas.emplace_back(onde + "falta StateOne, que e por onde a luta comeca");
    }
    if (!temStateFinal) {
        // A armadilha que ja custou depuracao: nota entre 40 e 59 ao fim da
        // terceira fase manda a FSM para StateFinal. Sem o estado, ela apenas
        // loga um erro e o chefe congela.
        problemas.emplace_back(onde + "falta StateFinal. Ele e obrigatorio mesmo num chefe de "
                                     "tres fases: se a nota ficar entre 40 e 59 ao fim da "
                                     "terceira, a FSM vai para StateFinal e o chefe congela "
                                     "sem ele");
    }

    for (const auto& f : fases) {
        if (f.proximo.empty()) continue;   // fim de linha, legitimo
        bool achou = false;
        for (const auto& outra : fases) {
            if (outra.nome == f.proximo) { achou = true; break; }
        }
        if (!achou) {
            problemas.emplace_back(onde + "a fase \"" + f.nome + "\" aponta para \"" + f.proximo +
                                   "\", que nao existe neste conjunto");
        }
    }

    return problemas;
}
