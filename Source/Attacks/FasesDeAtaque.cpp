//
// Fases de ataque de um chefe, lidas de texto JSON.
//

#include "FasesDeAtaque.h"

#include <algorithm>

#include "../Tabela.h"

#include "../JsonDeDados.h"
#include "Flex.h"

namespace {

    const std::vector<std::string> kEstrategias = {
        "AngledAttack", "CircleSpreadAttack", "WaveAttack", "BaloonAttack", "LaserAttack", "ConsultaAttack",
        "CaixaAttack", "FlexAttack", "CapsulaAttack", "SerpenteAttack", "SprintAttack",
        "PontosAttack", "TokenAttack", "PalavraAttack"
    };
    const std::vector<std::string> kMovimentos = {
        "RandomWander", "HoverAbovePlayer", "GoToCenter",
        // As tres do "o movimento e o ataque" - ver MovementStrategies.h.
        "Orbitar", "Varredura", "Mergulho",
        // A escada da cascata - ver Escada.h.
        "Escada",
        // O registrador de deslocamento e o clock do Ricardo - ver Logica.h
        // e Relogio.h.
        "Deslocamento", "OndaQuadrada",
        // O professor que diz para onde vai (Boss::AlvoDoMovimento): o
        // Vladimir e o Salles, que levam o corpo ate o lugar da estrutura.
        "Conduzido"
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
    const std::vector<std::string> kGatilhos = {"notificacao", "consulta", "agir", "nuvens", "chover", "portas",
                                                "contador", "estagios", "buscar", "lw", "sw", "add", "addi",
                                                "acender", "estados", "diagrama", "ler_a", "ler_b",
                                                "aceita", "rejeita", "derivar", "variavel", "metade", "triplo",
                                                "parou", "escrever", "capivara", "push", "estourar", "baldes",
                                                "chave", "percorrer", "nulo", "no", "visitar", "folha_nula",
                                                "achado", "aresta", "elemento", "trocar", "assentar",
                                                "cabecalho", "registro", "marcar", "selecionar",
                                                "projetar", "celula", "gravar", "desfazer",
                                                "placa", "pedido", "classe", "mensagem", "compilar",
                                                "bug", "debito", "rotulo", "balao", "confete",
                                                "indice", "subir", "core"};
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

        if (j.contains("etapa") && !j["etapa"].is_string()) {
            problemas.emplace_back(onde + "\"etapa\" deveria ser o nome de uma etapa da fase");
            return false;
        }
        saida.etapa = Texto(j, "etapa");

        // O GATILHO: um ataque que dispara quando o chefe puxa, e nao pelo
        // relogio. Um gatilho que nenhum chefe puxa e um ataque que nunca
        // aparece, calado - recusado na leitura.
        if (j.contains("gatilho")) {
            if (!j["gatilho"].is_string() || !GatilhoExiste(j["gatilho"].get<std::string>())) {
                problemas.emplace_back(onde + "\"gatilho\" precisa ser o nome de um gatilho que existe");
                return false;
            }
            saida.gatilho = j["gatilho"].get<std::string>();
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

        // --- bloco de caixa (opcional) ---
        if (j.contains("caixa")) {
            if (saida.estrategia != "CaixaAttack") {
                problemas.emplace_back(onde + "so a CaixaAttack usa \"caixa\"; o bloco foi ignorado");
            }
            else if (!j["caixa"].is_object()) {
                problemas.emplace_back(onde + "\"caixa\" deveria ser um objeto");
                return false;
            }
            else {
                DescricaoDeCaixa c;
                c.proporcao         = Opcional(j["caixa"], "proporcao");
                c.meiaAlturaInicial = Opcional(j["caixa"], "meiaAlturaInicial");

                // Zero ou negativo nao e "uma caixa estranha": e caixa nenhuma.
                // PontosDoContorno devolve vazio e o ataque nao dispara - calado.
                if ((c.proporcao && *c.proporcao <= 0.0f)
                    || (c.meiaAlturaInicial && *c.meiaAlturaInicial <= 0.0f)) {
                    problemas.emplace_back(onde + "\"proporcao\" e \"meiaAlturaInicial\" da "
                                                  "caixa precisam ser maiores que zero");
                    return false;
                }
                saida.caixa = c;
            }
        }

        // --- bloco de capsula (opcional) ---
        if (j.contains("capsula")) {
            if (saida.estrategia != "CapsulaAttack") {
                problemas.emplace_back(onde + "so a CapsulaAttack usa \"capsula\"; o bloco foi ignorado");
            }
            else if (!j["capsula"].is_object()) {
                problemas.emplace_back(onde + "\"capsula\" deveria ser um objeto");
                return false;
            }
            else {
                DescricaoDeCapsula c;
                c.abre       = Opcional(j["capsula"], "abre");
                c.velocidade = Opcional(j["capsula"], "velocidade");
                c.escreve    = Texto(j["capsula"], "escreve");

                // Uma capsula que nunca abre leva as letras empilhadas e
                // blindadas ate fora da tela; uma que abre parada deixa o anel
                // em cima do ponto onde abriu, e parado ele nunca sai da tela.
                if ((c.abre && *c.abre <= 0.0f) || (c.velocidade && *c.velocidade <= 0.0f)) {
                    problemas.emplace_back(onde + "\"abre\" e \"velocidade\" da capsula "
                                                  "precisam ser maiores que zero");
                    return false;
                }
                saida.capsula = c;
            }
        }

        // --- bloco de serpente (opcional) ---
        if (j.contains("serpente")) {
            if (saida.estrategia != "SerpenteAttack") {
                problemas.emplace_back(onde + "so a SerpenteAttack usa \"serpente\"; o bloco foi ignorado");
            }
            else if (!j["serpente"].is_object()) {
                problemas.emplace_back(onde + "\"serpente\" deveria ser um objeto");
                return false;
            }
            else {
                DescricaoDeSerpente sp;
                sp.espaco  = Opcional(j["serpente"], "espaco");
                sp.giro    = Opcional(j["serpente"], "giro");
                sp.duracao = Opcional(j["serpente"], "duracao");
                sp.cabeca  = Texto(j["serpente"], "cabeca");
                sp.corpo   = Texto(j["serpente"], "corpo");

                // Espaco zero empilha o corpo inteiro em cima da cabeca, e giro
                // ou duracao negativos nao sao uma serpente estranha: sao um
                // numero digitado errado. Giro e duracao ZERO valem - e a
                // serpente que so vai reta.
                if ((sp.espaco && *sp.espaco <= 0.0f) || (sp.giro && *sp.giro < 0.0f)
                    || (sp.duracao && *sp.duracao < 0.0f)) {
                    problemas.emplace_back(onde + "\"serpente\": \"espaco\" precisa ser maior "
                                                  "que zero, e \"giro\" e \"duracao\" nao podem "
                                                  "ser negativos");
                    return false;
                }
                saida.serpente = sp;
            }
        }

        // --- bloco de sprint (opcional) ---
        if (j.contains("sprint")) {
            if (saida.estrategia != "SprintAttack") {
                problemas.emplace_back(onde + "so a SprintAttack usa \"sprint\"; o bloco foi ignorado");
            }
            else if (!j["sprint"].is_object()) {
                problemas.emplace_back(onde + "\"sprint\" deveria ser um objeto");
                return false;
            }
            else {
                DescricaoDeSprint sp;
                sp.revisao  = Opcional(j["sprint"], "revisao");
                sp.historia = Texto(j["sprint"], "historia");
                sp.cursor   = Texto(j["sprint"], "cursor");
                sp.clique   = Texto(j["sprint"], "clique");

                // Uma revisao em zero, ou antes, derrubaria as historias em
                // cima do aluno antes de o cursor sair do professor: nao e uma
                // sprint curta, e um numero digitado errado.
                if (sp.revisao && *sp.revisao <= 0.0f) {
                    problemas.emplace_back(onde + "\"sprint\": \"revisao\" precisa ser maior que zero");
                    return false;
                }
                saida.sprint = sp;
            }
        }

        // --- bloco de tokens (opcional) ---
        if (j.contains("tokens")) {
            if (saida.estrategia != "TokenAttack") {
                problemas.emplace_back(onde + "so a TokenAttack usa \"tokens\"; o bloco foi ignorado");
            }
            else if (!j["tokens"].is_object()) {
                problemas.emplace_back(onde + "\"tokens\" deveria ser um objeto");
                return false;
            }
            else {
                const auto& tj = j["tokens"];
                DescricaoDeTokens tk;
                tk.temperatura = Opcional(tj, "temperatura");
                tk.abertura = Opcional(tj, "abertura");
                if (tj.contains("candidatos")) {
                    if (!tj["candidatos"].is_number_integer() || tj["candidatos"].get<int>() < 1) {
                        problemas.emplace_back(onde + "\"tokens\": \"candidatos\" precisa ser um inteiro "
                                                      "de 1 para cima");
                        return false;
                    }
                    tk.candidatos = tj["candidatos"].get<int>();
                }
                if (tj.contains("frase")) {
                    if (!tj["frase"].is_array()) {
                        problemas.emplace_back(onde + "\"tokens\": \"frase\" precisa ser uma lista de nomes");
                        return false;
                    }
                    for (const auto& palavra : tj["frase"]) {
                        if (!palavra.is_string()) {
                            problemas.emplace_back(onde + "\"tokens\": todo item de \"frase\" e o nome de "
                                                          "uma animacao");
                            return false;
                        }
                        tk.frase.push_back(palavra.get<std::string>());
                    }
                }

                // Temperatura negativa nao e "mais fria que zero": e um sinal
                // digitado errado. E um leque negativo poe os candidatos de
                // tras para a frente, com a mesma cara de leque.
                if ((tk.temperatura && *tk.temperatura < 0.0f) || (tk.abertura && *tk.abertura < 0.0f)) {
                    problemas.emplace_back(onde + "\"tokens\": \"temperatura\" e \"abertura\" nao podem "
                                                  "ser negativas");
                    return false;
                }
                saida.tokens = tk;
            }
        }

        // --- bloco de palavra (opcional) ---
        if (j.contains("palavra")) {
            if (saida.estrategia != "PalavraAttack") {
                problemas.emplace_back(onde + "so a PalavraAttack usa \"palavra\"; o bloco foi ignorado");
            }
            else if (!j["palavra"].is_object()) {
                problemas.emplace_back(onde + "\"palavra\" deveria ser um objeto");
                return false;
            }
            else {
                const auto& pj = j["palavra"];
                DescricaoDePalavra pl;
                pl.altura = Opcional(pj, "altura");
                pl.zero = Texto(pj, "zero");
                pl.um = Texto(pj, "um");

                // QUEM SE ATRAVESSA: o zero sempre; "todos" e a fileira de
                // portas, que nao machuca ninguem.
                const std::string atravessa = Texto(pj, "atravessa", "zeros");
                if (atravessa != "zeros" && atravessa != "todos") {
                    problemas.emplace_back(onde + "\"palavra\": \"atravessa\" e \"zeros\" ou \"todos\"");
                    return false;
                }
                pl.todosAtravessam = (atravessa == "todos");

                // O VALOR, escrito como se le: 0 e 1, com espacos para agrupar.
                // Tantos digitos quantos bits a fileira tem - um digito a mais
                // ou a menos deslocaria a palavra inteira de coluna.
                if (pj.contains("valor")) {
                    if (!pj["valor"].is_string()) {
                        problemas.emplace_back(onde + "\"palavra\": \"valor\" se escreve com 0 e 1, entre aspas");
                        return false;
                    }
                    std::uint32_t v = 0;
                    int digitos = 0;
                    for (const char c : pj["valor"].get<std::string>()) {
                        if (c == ' ') continue;
                        if (c != '0' && c != '1') {
                            problemas.emplace_back(onde + "\"palavra\": \"valor\" so tem 0, 1 e espacos");
                            return false;
                        }
                        v = (v << 1) | (c == '1' ? 1u : 0u);
                        ++digitos;
                    }
                    if (digitos > 31 || (saida.projeteis && digitos != *saida.projeteis)) {
                        problemas.emplace_back(onde + "\"palavra\": \"valor\" precisa de um digito por bit "
                                                      "(\"projeteis\")");
                        return false;
                    }
                    pl.valor = v;
                }
                saida.palavra = pl;
            }
        }

        // --- bloco de pontos (obrigatorio na PontosAttack) ---
        const bool ehPontos = (saida.estrategia == "PontosAttack");
        if (j.contains("pontos") && !ehPontos) {
            problemas.emplace_back(onde + "so a PontosAttack usa \"pontos\"; o bloco foi ignorado");
        }
        if (ehPontos) {
            if (!j.contains("pontos") || !j["pontos"].is_object()) {
                problemas.emplace_back(onde + "a PontosAttack precisa do bloco \"pontos\", com o \"posto\" "
                                              "de onde atira");
                return false;
            }
            DescricaoDePontos pt;
            pt.posto = Texto(j["pontos"], "posto");
            pt.mirar = OpcionalBool(j["pontos"], "mirar").value_or(false);
            pt.prever = OpcionalBool(j["pontos"], "prever").value_or(false);

            // UM POSTO QUE NINGUEM OCUPA NAO DISPARA NADA, e calado: o Boss nao
            // acha ponto nenhum e o ataque nunca aparece. Recusado na leitura.
            if (!PostoExiste(pt.posto)) {
                problemas.emplace_back(onde + "\"pontos\": o posto \"" + pt.posto + "\" nao existe");
                return false;
            }
            saida.pontos = pt;
        }

        // --- bloco de flex (obrigatorio na FlexAttack) ---
        const bool ehFlex = (saida.estrategia == "FlexAttack");
        if (j.contains("flex")) {
            if (!ehFlex) {
                problemas.emplace_back(onde + "so a FlexAttack usa \"flex\"; o bloco foi ignorado");
            }
            else {
                const auto& fj = j["flex"];
                DescricaoDeFlex f;
                if (fj.is_object()) {
                    f.de      = Texto(fj, "de");
                    f.para    = Texto(fj, "para");
                    f.aos     = Opcional(fj, "aos");
                    f.em      = Opcional(fj, "em");
                    f.item    = Opcional(fj, "item");
                    f.largura = Opcional(fj, "largura");
                }
                Flex::Justificar lixo{};
                if (!fj.is_object() || !Flex::PeloNome(f.de, lixo) || !Flex::PeloNome(f.para, lixo)) {
                    problemas.emplace_back(onde + "\"flex\" precisa de \"de\" e \"para\" com nomes "
                                                  "de justify-content: flex-start, flex-end, center, "
                                                  "space-between ou space-around");
                    return false;
                }
                if ((f.aos && *f.aos < 0.0f) || (f.em && *f.em < 0.0f)
                    || (f.item && *f.item <= 0.0f) || (f.largura && *f.largura <= 0.0f)) {
                    problemas.emplace_back(onde + "\"flex\": \"aos\" e \"em\" nao podem ser "
                                                  "negativos, e \"item\" e \"largura\" precisam ser "
                                                  "maiores que zero");
                    return false;
                }
                saida.flex = f;
            }
        }
        else if (ehFlex) {
            problemas.emplace_back(onde + "um ataque FlexAttack precisa do bloco \"flex\" com "
                                          "\"de\" e \"para\": sem eles a fileira nao sabe como "
                                          "nascer nem como terminar");
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
bool GatilhoExiste(const std::string& nome)        { return Contem(kGatilhos, nome); }
const std::vector<std::string>& NomesDosGatilhos() { return kGatilhos; }

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

            // "etapas": a fase em trechos com nome, em ordem. Recusadas inteiras se
            // qualquer uma estiver errada: uma etapa a menos desloca todas as
            // seguintes, e o ataque de Testes dispararia na hora da Manutencao.
            if (corpoFase.contains("etapas")) {
                const auto& lista = corpoFase["etapas"];
                bool boas = lista.is_array() && !lista.empty();
                float soma = 0.0f;
                for (size_t k = 0; boas && k < lista.size(); ++k) {
                    const auto& e = lista[k];
                    if (!e.is_object() || !e.contains("nome") || !e["nome"].is_string()
                        || !e.contains("duracao") || !e["duracao"].is_number()
                        || (e.contains("anuncio") && !e["anuncio"].is_string())) { boas = false; break; }
                    DescricaoDeEtapa etapa;
                    etapa.nome = e["nome"].get<std::string>();
                    etapa.duracao = e["duracao"].get<float>();
                    etapa.anuncio = Texto(e, "anuncio");
                    if (etapa.nome.empty() || etapa.duracao <= 0.0f) { boas = false; break; }
                    for (const auto& outra : f.etapas) if (outra.nome == etapa.nome) boas = false;
                    soma += etapa.duracao;
                    f.etapas.push_back(std::move(etapa));
                }
                if (!boas) {
                    saida.problemas.emplace_back(onde + "\"etapas\" precisa ser uma lista de objetos "
                                                 "com \"nome\" (sem repetir) e \"duracao\" maior "
                                                 "que zero");
                    continue;
                }
                // A ULTIMA ETAPA TEM DE CABER NA FASE: a que passasse do fim
                // nunca terminaria, e o ataque dela simplesmente pararia antes.
                if (soma > f.duracao + 0.001f) {
                    saida.problemas.emplace_back(onde + "as \"etapas\" somam mais que a \"duracao\" "
                                                 "da fase");
                    continue;
                }
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

            // UM ATAQUE QUE PEDE UMA ETAPA QUE A FASE NAO TEM nunca dispararia.
            // Descartado com aviso, como um ataque mal escrito.
            for (auto it = f.ataques.begin(); it != f.ataques.end();) {
                float de = 0.0f, ate = 0.0f;
                if (!it->etapa.empty() && !JanelaDaEtapa(f.etapas, it->etapa, de, ate)) {
                    saida.problemas.emplace_back(onde + "um ataque pede a etapa \"" + it->etapa +
                                                 "\", que a fase nao tem");
                    it = f.ataques.erase(it);
                } else {
                    ++it;
                }
            }
            if (f.ataques.empty()) {
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

bool JanelaDaEtapa(const std::vector<DescricaoDeEtapa>& etapas, const std::string& nome,
                   float& de, float& ate) {
    float t = 0.0f;
    for (const auto& e : etapas) {
        if (e.nome == nome) { de = t; ate = t + e.duracao; return true; }
        t += e.duracao;
    }
    return false;
}

int EtapaNoTempo(const std::vector<DescricaoDeEtapa>& etapas, const float t) {
    float fim = 0.0f;
    for (size_t i = 0; i < etapas.size(); ++i) {
        fim += etapas[i].duracao;
        if (t < fim) return static_cast<int>(i);
    }
    return -1;
}

float PrimeiroDisparoNaJanela(const float de) {
    // A janela que abre no zero nunca esteve fechada: o relogio do ataque
    // comeca em kPrimeiroDisparoDaFase e conta dali. A que abre depois passou
    // o comeco da fase fechada, e fechada o relogio fica em no maximo
    // kPrimeiroDisparoDaJanela.
    if (de <= 0.0f) return kPrimeiroDisparoDaFase;
    return de + std::min(kPrimeiroDisparoDaFase, kPrimeiroDisparoDaJanela);
}
