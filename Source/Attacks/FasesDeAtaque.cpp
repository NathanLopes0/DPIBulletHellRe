//
// Fases de ataque de um chefe, lidas de texto JSON.
//

#include "FasesDeAtaque.h"

#include "../Tabela.h"

#include "../JsonDeDados.h"

namespace {

    const std::vector<std::string> kEstrategias = {
        "AngledAttack", "CircleSpreadAttack", "WaveAttack", "BaloonAttack", "LaserAttack", "ConsultaAttack"
    };
    const std::vector<std::string> kMovimentos = {
        "RandomWander", "HoverAbovePlayer", "GoToCenter",
        // As tres do "o movimento e o ataque" - ver MovementStrategies.h.
        "Orbitar", "Varredura", "Mergulho"
    };

    /// Os nomes aceitos, para a mensagem de erro. DERIVADA da lista acima, e
    /// nao escrita a mao ao lado dela: a frase antiga citava tres movimentos
    /// por extenso, e acrescentar um quarto sem lembrar dela faria o erro
    /// ensinar uma lista incompleta justamente a quem acabou de errar o nome.
    std::string MovimentosAceitos() {
        std::string s;
        for (size_t i = 0; i < kMovimentos.size(); ++i) {
            if (i > 0) s += (i + 1 == kMovimentos.size()) ? " ou " : ", ";
            s += kMovimentos[i];
        }
        return s;
    }
    /// "None" NAO entra: a propria BaloonAttack recusa side == None.
    const std::vector<std::string> kLadosDeBalao = {"Down", "Left", "Right", "Up"};
    const std::vector<std::string> kEixosDeConsulta = {"Linha", "Coluna"};
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
                                   "\" nao existe (use " + MovimentosAceitos() + ")");
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

    /// Inteiro de verdade, nao "numero que por acaso e inteiro": 2.5 linhas nao
    /// significa nada, e aceitar silenciosamente truncaria para 2.
    std::optional<int> OpcionalInteiro(const nlohmann::json& j, const char* nome) {
        if (j.contains(nome) && j[nome].is_number_integer()) return j[nome].get<int>();
        return std::nullopt;
    }

    /// Le o bloco de consulta. Devolve false quando ele e inaproveitavel.
    bool LerConsulta(const nlohmann::json& j, const std::string& onde,
                     std::vector<std::string>& problemas, DescricaoDeConsulta& saida) {

        if (!j.is_object()) {
            problemas.emplace_back(onde + "\"consulta\" deveria ser um objeto");
            return false;
        }

        saida.eixo = Texto(j, "eixo");
        if (!EixoDeConsultaExiste(saida.eixo)) {
            problemas.emplace_back(onde + "\"consulta\" precisa de \"eixo\": Linha ou Coluna");
            return false;
        }

        saida.indice    = OpcionalInteiro(j, "indice");
        saida.linhas    = OpcionalInteiro(j, "linhas");
        saida.colunas   = OpcionalInteiro(j, "colunas");
        saida.aviso     = Opcional(j, "aviso");
        saida.invertido = OpcionalBool(j, "invertido");

        // Uma tabela de zero faixas nao tem onde varrer, e a estrategia recusaria
        // em tempo de execucao escrevendo no log. Recusar aqui troca um ataque
        // que nao dispara por uma frase que diz o arquivo e a linha.
        if (saida.linhas && *saida.linhas <= 0) {
            problemas.emplace_back(onde + "\"linhas\" precisa ser pelo menos 1");
            return false;
        }
        if (saida.colunas && *saida.colunas <= 0) {
            problemas.emplace_back(onde + "\"colunas\" precisa ser pelo menos 1");
            return false;
        }

        // Aviso negativo nao e so um numero estranho: e a varredura disparando
        // antes de aparecer. O jogador seria atingido por um ataque que nunca
        // teve como ler - ver ConsultaAttackParams::aviso.
        if (saida.aviso && *saida.aviso < 0.0f) {
            problemas.emplace_back(onde + "\"aviso\" nao pode ser negativo: e o tempo em que o "
                                          "jogador ve a faixa escolhida antes de ela disparar");
            return false;
        }

        if (saida.indice && *saida.indice < 0) {
            problemas.emplace_back(onde + "\"indice\" nao pode ser negativo; omita o campo para "
                                          "deixar o chefe escolher a faixa");
            return false;
        }

        // UM INDICE FORA DA TABELA E RECUSADO, E NAO LIMITADO EM SILENCIO.
        //
        // A ConsultaAttack limita para a ultima faixa, porque em tempo de
        // execucao nao ha nada melhor a fazer. Mas isso ja produziu uma fase
        // final inteira em que quatro ataques diziam varrer as faixas 0, 2, 1 e
        // 3 de uma tabela de UMA faixa: os quatro caiam na mesma, a tela toda, e
        // nada avisava. O arquivo descrevia uma trelica que o jogo nunca desenhou.
        const Tabela::Forma padrao{};
        const int quantas = (saida.eixo == "Coluna")
                                ? saida.colunas.value_or(padrao.colunas)
                                : saida.linhas.value_or(padrao.linhas);

        if (saida.indice && *saida.indice >= quantas) {
            problemas.emplace_back(onde + "\"indice\" " + std::to_string(*saida.indice) +
                                   " nao existe numa tabela de " + std::to_string(quantas) +
                                   " faixa(s) nesse eixo: a varredura cairia na ultima e este "
                                   "ataque varreria uma faixa diferente da que esta escrita aqui");
            return false;
        }

        return true;
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
                                   "WaveAttack, BaloonAttack, LaserAttack ou ConsultaAttack)");
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

        // --- bloco de consulta ---
        const bool ehConsulta = (saida.estrategia == "ConsultaAttack");
        if (j.contains("consulta")) {
            if (!ehConsulta) {
                problemas.emplace_back(onde + "so a ConsultaAttack usa \"consulta\"; o bloco foi ignorado");
            }
            else {
                DescricaoDeConsulta c;
                if (!LerConsulta(j["consulta"], onde, problemas, c)) return false;
                saida.consulta = c;
            }
        }
        else if (ehConsulta) {
            problemas.emplace_back(onde + "um ataque ConsultaAttack precisa do bloco \"consulta\": "
                                          "sem ele a estrategia nao sabe que faixa varrer e nao "
                                          "dispara nada");
            return false;
        }

        return true;
    }

}

bool EstrategiaExiste(const std::string& nome)   { return Contem(kEstrategias, nome); }
bool MovimentoExiste(const std::string& nome)    { return Contem(kMovimentos, nome); }
bool NomeDeFaseExiste(const std::string& nome)   { return Contem(kNomesDeFase, nome); }
bool LadoDeBalaoExiste(const std::string& nome)  { return Contem(kLadosDeBalao, nome); }
bool EixoDeConsultaExiste(const std::string& nome) { return Contem(kEixosDeConsulta, nome); }

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
        saida.problemas.emplace_back("o arquivo deveria ser um objeto com uma fase por chave");
        return saida;
    }

    {
        for (auto itFase = raiz.begin(); itFase != raiz.end(); ++itFase) {

            const std::string& nomeFase = itFase.key();
            const auto& corpoFase = itFase.value();
            const std::string onde = "fase \"" + nomeFase + "\": ";

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

            // "anuncio": o texto do placar nesta fase. Um valor que nao e texto
            // e RECUSADO, e nao lido como vazio: viraria um placar que some sem
            // aviso nenhum, e o jogador nunca saberia que havia algo a ler.
            if (corpoFase.contains("anuncio") && !corpoFase["anuncio"].is_string()) {
                saida.problemas.emplace_back(onde + "\"anuncio\" deveria ser um texto");
                continue;
            }
            f.anuncio = Texto(corpoFase, "anuncio");

            // "intercambio": os alunos trocam de lugar a cada "aCada" segundos.
            // Os tres jeitos de errar sao recusados aqui, porque em jogo nenhum
            // deles da erro: sem periodo a troca nunca acontece, um aviso do
            // tamanho do periodo fica aceso para sempre, e um numero que falta
            // vira zero sem ninguem ver.
            if (corpoFase.contains("intercambio")) {
                const auto& ic = corpoFase["intercambio"];
                if (!ic.is_object() || !ic.contains("aCada") || !ic["aCada"].is_number()
                    || !ic.contains("aviso") || !ic["aviso"].is_number()) {
                    saida.problemas.emplace_back(onde + "\"intercambio\" precisa ser um objeto "
                                                 "com \"aCada\" e \"aviso\", em segundos");
                    continue;
                }
                DescricaoDeIntercambio d;
                d.aCada = ic["aCada"].get<float>();
                d.aviso = ic["aviso"].get<float>();
                if (d.aCada <= 0.0f || d.aviso < 0.0f || d.aviso >= d.aCada) {
                    saida.problemas.emplace_back(onde + "\"intercambio\" precisa de \"aCada\" "
                                                 "maior que zero e \"aviso\" entre zero e "
                                                 "\"aCada\" - um aviso do tamanho do periodo "
                                                 "ficaria aceso para sempre");
                    continue;
                }
                f.intercambio = d;
            }

            if (corpoFase.contains("movimento")) {
                f.movimento = LerMovimento(corpoFase["movimento"], onde, saida.problemas);
            }

            // "ataquesDe": esta fase usa os ataques de OUTRA. Serve a fase que
            // se divide, em que o corpo original repete o ataque de uma fase
            // anterior - escrever a lista de novo seria a mesma coisa dita duas
            // vezes, e duas copias divergem.
            f.ataquesDe = Texto(corpoFase, "ataquesDe");

            if (corpoFase.contains("clone")) {
                const auto& c = corpoFase["clone"];
                if (!c.is_object()) {
                    saida.problemas.emplace_back(onde + "\"clone\" deveria ser um objeto");
                    continue;
                }
                f.clone.ataquesDe = Texto(c, "ataquesDe");
                if (f.clone.ataquesDe.empty()) {
                    saida.problemas.emplace_back(onde + "\"clone\" precisa de \"ataquesDe\" "
                                                 "dizendo de qual fase o segundo corpo tira os "
                                                 "ataques");
                    continue;
                }
                f.clone.existe = true;
            }

            // UMA FASE NAO PODE TER AS DUAS COISAS: "ataques" e "ataquesDe"
            // juntos seriam duas respostas para "o que ele dispara aqui", e o
            // leitor teria de escolher uma em silencio.
            const bool temLista = corpoFase.contains("ataques") && corpoFase["ataques"].is_array();

            if (temLista && !f.ataquesDe.empty()) {
                saida.problemas.emplace_back(onde + "tem \"ataques\" E \"ataquesDe\". Escolha "
                                             "um: ou a fase declara os ataques dela, ou usa os "
                                             "de outra");
                continue;
            }

            if (!f.ataquesDe.empty()) {
                // A lista vem depois, quando todas as fases estiverem lidas:
                // aqui a fase de origem pode ainda nao ter sido vista.
                saida.fases.push_back(std::move(f));
                continue;
            }

            if (!temLista) {
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

            saida.fases.push_back(std::move(f));
        }
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

    // "ataquesDe" e o "clone" apontam para OUTRA fase, e um nome errado neles
    // falharia em silencio: a fase carregaria sem ataque nenhum e o chefe
    // ficaria parado. Mesma razao da checagem de "proximo" logo abaixo.
    auto existe = [&fases](const std::string& nome) {
        for (const auto& f : fases) if (f.nome == nome) return true;
        return false;
    };

    for (const auto& f : fases) {
        for (const auto& [campo, alvo] : {std::pair<const char*, std::string>{"ataquesDe", f.ataquesDe},
                                          {"clone.ataquesDe", f.clone.existe ? f.clone.ataquesDe : std::string{}}}) {
            if (alvo.empty()) continue;

            if (alvo == f.nome) {
                // Uma fase que tira os ataques de si mesma e uma fase sem
                // ataques: a resolucao nao tem de onde copiar.
                problemas.emplace_back(onde + "a fase \"" + f.nome + "\" usa \"" + campo +
                                       "\" apontando para ela mesma");
                continue;
            }
            if (!existe(alvo)) {
                problemas.emplace_back(onde + "a fase \"" + f.nome + "\" tem \"" + campo +
                                       "\": \"" + alvo + "\", que nao existe neste conjunto");
                continue;
            }

            // SEM CORRENTE: a fase apontada tem de declarar os ataques dela.
            // Permitir A -> B -> C exigiria detectar ciclo, e o ganho seria
            // nenhum - "use os ataques de quem usa os ataques de outro" nao
            // diz nada que apontar direto para a origem nao diga melhor.
            for (const auto& origem : fases) {
                if (origem.nome != alvo) continue;
                if (!origem.ataquesDe.empty()) {
                    problemas.emplace_back(onde + "a fase \"" + f.nome + "\" aponta para \"" +
                                           alvo + "\", que por sua vez usa os ataques de outra. "
                                           "Aponte direto para a fase que declara os ataques");
                }
            }
        }
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
