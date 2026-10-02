//
// Regras de configuracao de projetil, lidas de texto JSON.
//

#include "RegrasDeAtaque.h"

#include "../JsonDeDados.h"

namespace {

    const std::vector<std::string> kMotions   = {"Path", "Tracking", "Wobble", "MiraPeriodica"};
    const std::vector<std::string> kModifiers = {"Accelerate", "SlowDown", "Activate",
                                                 "Deactivate", "PulsoDeVelocidade"};

    bool Contem(const std::vector<std::string>& lista, const std::string& v) {
        for (const auto& x : lista) if (x == v) return true;
        return false;
    }

    float Campo(const nlohmann::json& j, const char* nome, float padrao) {
        return (j.contains(nome) && j[nome].is_number()) ? j[nome].get<float>() : padrao;
    }

    RitmoDeCiclos LerRitmo(const nlohmann::json& j) {
        RitmoDeCiclos r;
        if (!j.is_object()) return r;
        r.atraso           = Campo(j, "atraso", 0.0f);
        r.duracaoInvestida = Campo(j, "investida", 0.2f);
        r.duracaoPausa     = Campo(j, "pausa", 1.0f);
        r.repeticoes       = (j.contains("repeticoes") && j["repeticoes"].is_number_integer())
                             ? j["repeticoes"].get<int>() : 1;
        return r;
    }

    /// Le uma descricao de behavior. 'erro' fica vazio quando deu certo.
    DescricaoDeBehavior LerDescricao(const nlohmann::json& j, std::string& erro) {
        DescricaoDeBehavior d;
        if (!j.is_object() || !j.contains("tipo") || !j["tipo"].is_string()) {
            erro = "precisa ser um objeto com um campo \"tipo\"";
            return d;
        }
        d.tipo = j["tipo"].get<std::string>();
        if (!Contem(kMotions, d.tipo) && !Contem(kModifiers, d.tipo)) {
            erro = "tipo \"" + d.tipo + "\" nao existe";
            return d;
        }

        d.forma           = (j.contains("forma") && j["forma"].is_string()) ? j["forma"].get<std::string>() : "";
        d.mira            = (j.contains("mira")  && j["mira"].is_string())  ? j["mira"].get<std::string>()  : "";
        d.parametroDaMira = Campo(j, "antecipacao", 0.0f);
        d.velocidade      = Campo(j, "velocidade", 0.0f);
        d.atraso          = Campo(j, "atraso", 0.0f);
        d.pararPor        = Campo(j, "pararPor", 0.0f);

        // Nao se chama "pausa" porque esse nome ja e do PulsoDeVelocidade, onde
        // quer dizer MODULO DE VELOCIDADE durante a pausa - coisa diferente. Dois
        // campos de mesmo nome e sentidos diferentes no mesmo arquivo e pedir para
        // alguem escrever um achando que escreve o outro.
        if (j.contains("pararNoPonto") && j["pararNoPonto"].is_number_integer()) {
            d.pararNoPonto = j["pararNoPonto"].get<int>();
        }
        d.forca           = Campo(j, "forca", 0.0f);
        d.duracao         = Campo(j, "duracao", 0.0f);
        d.amplitude       = Campo(j, "amplitude", 0.0f);
        d.frequencia      = Campo(j, "frequencia", 0.0f);
        d.fator           = Campo(j, "fator", 1.0f);
        d.moduloInvestida = Campo(j, "investida", 0.0f);
        d.moduloPausa     = Campo(j, "pausa", 0.0f);

        if (j.contains("velocidadeInicial") && j["velocidadeInicial"].is_array()
            && j["velocidadeInicial"].size() == 2) {
            d.velocidadeInicial = Vector2(j["velocidadeInicial"][0].get<float>(),
                                          j["velocidadeInicial"][1].get<float>());
        }
        if (j.contains("ritmo")) d.ritmo = LerRitmo(j["ritmo"]);
        return d;
    }

    /// O laco por-regra, um lugar so. LerRegras roda isto uma vez por conjunto;
    /// LerListaDeRegras roda para um array que ja veio de outro arquivo.
    std::vector<Regra> LerLista(const nlohmann::json& lista, const std::string& prefixo,
                               std::vector<std::string>& problemas) {

        std::vector<Regra> regras;

        for (size_t k = 0; k < lista.size(); ++k) {

            const auto& j = lista[k];
            const std::string onde = prefixo + "regra " + std::to_string(k + 1) + ": ";

            if (!j.is_object()) {
                problemas.emplace_back(onde + "deveria ser um objeto");
                continue;
            }

            Regra r;

            // --- condicao ---
            if (j.contains("chance") && j["chance"].is_number()) {
                r.condicao = Regra::Chance;
                r.chance = j["chance"].get<float>();
            }
            else if (j.contains("indices") && j["indices"].is_array()) {
                r.condicao = Regra::Indices;
                for (const auto& i : j["indices"]) {
                    if (i.is_number_integer()) r.indices.push_back(i.get<int>());
                }
                if (r.indices.empty()) {
                    problemas.emplace_back(onde + "\"indices\" esta vazio ou nao tem inteiros");
                    continue;
                }
            }
            else if (j.contains("quando") && j["quando"].is_string()) {
                const std::string q = j["quando"].get<std::string>();
                if (q == "pares")        r.condicao = Regra::Pares;
                else if (q == "impares") r.condicao = Regra::Impares;
                else if (q == "sempre")  r.condicao = Regra::Sempre;
                else {
                    problemas.emplace_back(onde + "\"quando\" so aceita sempre, pares ou impares");
                    continue;
                }
            }

            // --- motion ---
            bool falhou = false;
            if (j.contains("motion")) {
                std::string erro;
                const DescricaoDeBehavior d = LerDescricao(j["motion"], erro);
                if (!erro.empty()) {
                    problemas.emplace_back(onde + "motion " + erro);
                    falhou = true;
                }
                else if (!EhMotion(d.tipo)) {
                    // O equivalente em tempo de leitura do static_assert da fase 1.
                    problemas.emplace_back(onde + "\"" + d.tipo + "\" e um Modifier, nao pode ir em \"motion\"");
                    falhou = true;
                }
                else {
                    // A parada so existe em caminho, e os dois campos so fazem
                    // sentido juntos. Metade da configuracao nao da erro em jogo -
                    // simplesmente nao para -, que e o tipo de silencio que faz
                    // alguem procurar o problema no motor em vez de no arquivo.
                    if (d.pararPor > 0.0f && d.pararNoPonto < 0) {
                        problemas.emplace_back(onde + "\"pararPor\" sem \"pararNoPonto\": falta dizer"
                                                      " em qual ponto do caminho parar");
                        falhou = true;
                    }
                    else if (d.pararNoPonto >= 0 && d.pararPor <= 0.0f) {
                        problemas.emplace_back(onde + "\"pararNoPonto\" sem \"pararPor\" maior que zero:"
                                                      " falta dizer por quanto tempo parar");
                        falhou = true;
                    }
                    else if (d.pararPor > 0.0f && d.tipo != "Path") {
                        problemas.emplace_back(onde + "parada so existe em \"Path\", e esta motion e \"" +
                                               d.tipo + "\"");
                        falhou = true;
                    }
                    else {
                        r.temMotion = true;
                        r.motion = d;
                    }
                }
            }

            // --- modifiers ---
            if (!falhou && j.contains("modifiers")) {
                if (!j["modifiers"].is_array()) {
                    problemas.emplace_back(onde + "\"modifiers\" deveria ser uma lista");
                    falhou = true;
                }
                else {
                    for (const auto& m : j["modifiers"]) {
                        std::string erro;
                        const DescricaoDeBehavior d = LerDescricao(m, erro);
                        if (!erro.empty()) {
                            problemas.emplace_back(onde + "modifier " + erro);
                            falhou = true;
                            break;
                        }
                        if (!EhModifier(d.tipo)) {
                            problemas.emplace_back(onde + "\"" + d.tipo + "\" e uma Motion, nao pode ir em \"modifiers\"");
                            falhou = true;
                            break;
                        }
                        r.modifiers.push_back(d);
                    }
                }
            }
            if (falhou) continue;

            // --- investidaRepetida: acucar que vira as duas pecas ---
            //
            // A investida precisa de uma Motion que re-aponta e um Modifier que
            // pulsa o modulo, derivados DO MESMO ritmo. Escrever as duas a mao
            // significa escrever o ritmo duas vezes, e mudar um e esquecer o
            // outro faz a mirada acontecer fora do mergulho - um defeito que nao
            // da erro nenhum, so fica esquisito.
            //
            // A expansao acontece AQUI, na leitura, e nao na ponte: assim o resto
            // do sistema nunca ve um terceiro tipo de coisa, e o equivalente em
            // dados de AplicarInvestidaRepetida fica testavel na camada pura.
            if (j.contains("investidaRepetida")) {
                const auto& inv = j["investidaRepetida"];
                if (!inv.is_object()) {
                    problemas.emplace_back(onde + "\"investidaRepetida\" deveria ser um objeto");
                    continue;
                }
                if (r.temMotion) {
                    problemas.emplace_back(onde + "\"investidaRepetida\" JA define a motion do "
                                                  "projetil, entao nao pode vir junto de \"motion\"");
                    continue;
                }
                if (!inv.contains("ritmo")) {
                    problemas.emplace_back(onde + "\"investidaRepetida\" precisa de \"ritmo\"");
                    continue;
                }

                const RitmoDeCiclos ritmo = LerRitmo(inv["ritmo"]);
                if (ritmo.repeticoes < 1) {
                    problemas.emplace_back(onde + "\"repeticoes\" do ritmo precisa ser 1 ou mais");
                    continue;
                }

                const float naPausa = Campo(inv, "velocidadeNaPausa", 15.0f);
                if (naPausa <= 0.0f) {
                    // A direcao mora dentro do vetor velocidade: com modulo zero
                    // a mirada seguinte nao tem o que girar, e o projetil fica
                    // parado para sempre.
                    problemas.emplace_back(onde + "\"velocidadeNaPausa\" nao pode ser zero: a "
                                                  "direcao mora no vetor velocidade, e com modulo "
                                                  "zero a mirada seguinte nao tem o que girar");
                    continue;
                }

                DescricaoDeBehavior mira;
                mira.tipo = "MiraPeriodica";
                mira.ritmo = ritmo;
                r.temMotion = true;
                r.motion = mira;

                DescricaoDeBehavior pulso;
                pulso.tipo = "PulsoDeVelocidade";
                pulso.ritmo = ritmo;   // o MESMO ritmo, por construcao
                pulso.moduloInvestida = Campo(inv, "velocidadeNaInvestida", 0.0f);
                pulso.moduloPausa = naPausa;
                r.modifiers.push_back(pulso);
            }

            if (j.contains("animacao") && j["animacao"].is_string()) {
                r.animacao = j["animacao"].get<std::string>();
            }

            if (j.contains("escala")) {
                if (!j["escala"].is_number() || j["escala"].get<float>() <= 0.0f) {
                    problemas.emplace_back(onde + "\"escala\" precisa ser um numero maior que zero");
                    continue;
                }
                r.escala = j["escala"].get<float>();
            }

            if (!r.temMotion && r.modifiers.empty() && r.animacao.empty() && !r.escala) {
                problemas.emplace_back(onde + "nao faz nada: precisa de motion, modifiers, "
                                              "animacao, escala ou investidaRepetida");
                continue;
            }

            regras.push_back(std::move(r));
        }

        return regras;
    }

}

bool EhMotion(const std::string& tipo)   { return Contem(kMotions, tipo); }
bool EhModifier(const std::string& tipo) { return Contem(kModifiers, tipo); }

bool RegraSeAplica(const Regra& regra, const int indice, const float sorteio) {
    switch (regra.condicao) {
        case Regra::Sempre:  return true;
        case Regra::Chance:  return sorteio < regra.chance;
        case Regra::Pares:   return indice % 2 == 0;
        case Regra::Impares: return indice % 2 != 0;
        case Regra::Indices:
            for (const int i : regra.indices) if (i == indice) return true;
            return false;
    }
    return false;
}

RegrasLidas LerRegras(const std::string& textoJson) {

    RegrasLidas saida;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(textoJson);
    }
    catch (const std::exception& e) {
        saida.problemas.emplace_back(std::string("o arquivo nao e um JSON valido: ") + e.what());
        return saida;
    }
    if (!raiz.is_object()) {
        saida.problemas.emplace_back("o arquivo deveria ser um objeto com um conjunto de regras por chave");
        return saida;
    }

    for (auto it = raiz.begin(); it != raiz.end(); ++it) {

        const std::string& nomeConjunto = it.key();
        const auto& lista = it.value();

        if (!lista.is_array()) {
            saida.problemas.emplace_back("conjunto \"" + nomeConjunto + "\": deveria ser uma lista de regras");
            continue;
        }

        saida.conjuntos.emplace(nomeConjunto,
            LerLista(lista, "conjunto \"" + nomeConjunto + "\", ", saida.problemas));
    }

    return saida;
}

std::vector<Regra> LerListaDeRegras(const std::string& textoDaLista, const std::string& onde,
                                   std::vector<std::string>& problemas) {

    nlohmann::json lista;
    try {
        lista = LerJsonDeDados(textoDaLista);
    }
    catch (const std::exception& e) {
        problemas.emplace_back(onde + "as regras nao sao um JSON valido: " + e.what());
        return {};
    }
    if (!lista.is_array()) {
        problemas.emplace_back(onde + "\"regras\" deveria ser uma lista");
        return {};
    }
    return LerLista(lista, onde, problemas);
}