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

        std::vector<Regra> regras;

        for (size_t k = 0; k < lista.size(); ++k) {

            const auto& j = lista[k];
            const std::string onde = "conjunto \"" + nomeConjunto + "\", regra " + std::to_string(k + 1) + ": ";

            if (!j.is_object()) {
                saida.problemas.emplace_back(onde + "deveria ser um objeto");
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
                    saida.problemas.emplace_back(onde + "\"indices\" esta vazio ou nao tem inteiros");
                    continue;
                }
            }
            else if (j.contains("quando") && j["quando"].is_string()) {
                const std::string q = j["quando"].get<std::string>();
                if (q == "pares")        r.condicao = Regra::Pares;
                else if (q == "impares") r.condicao = Regra::Impares;
                else if (q == "sempre")  r.condicao = Regra::Sempre;
                else {
                    saida.problemas.emplace_back(onde + "\"quando\" so aceita sempre, pares ou impares");
                    continue;
                }
            }

            // --- motion ---
            bool falhou = false;
            if (j.contains("motion")) {
                std::string erro;
                const DescricaoDeBehavior d = LerDescricao(j["motion"], erro);
                if (!erro.empty()) {
                    saida.problemas.emplace_back(onde + "motion " + erro);
                    falhou = true;
                }
                else if (!EhMotion(d.tipo)) {
                    // O equivalente em tempo de leitura do static_assert da fase 1.
                    saida.problemas.emplace_back(onde + "\"" + d.tipo + "\" e um Modifier, nao pode ir em \"motion\"");
                    falhou = true;
                }
                else {
                    r.temMotion = true;
                    r.motion = d;
                }
            }

            // --- modifiers ---
            if (!falhou && j.contains("modifiers")) {
                if (!j["modifiers"].is_array()) {
                    saida.problemas.emplace_back(onde + "\"modifiers\" deveria ser uma lista");
                    falhou = true;
                }
                else {
                    for (const auto& m : j["modifiers"]) {
                        std::string erro;
                        const DescricaoDeBehavior d = LerDescricao(m, erro);
                        if (!erro.empty()) {
                            saida.problemas.emplace_back(onde + "modifier " + erro);
                            falhou = true;
                            break;
                        }
                        if (!EhModifier(d.tipo)) {
                            saida.problemas.emplace_back(onde + "\"" + d.tipo + "\" e uma Motion, nao pode ir em \"modifiers\"");
                            falhou = true;
                            break;
                        }
                        r.modifiers.push_back(d);
                    }
                }
            }
            if (falhou) continue;

            if (j.contains("animacao") && j["animacao"].is_string()) {
                r.animacao = j["animacao"].get<std::string>();
            }

            if (!r.temMotion && r.modifiers.empty() && r.animacao.empty()) {
                saida.problemas.emplace_back(onde + "nao faz nada: precisa de motion, modifiers ou animacao");
                continue;
            }

            regras.push_back(std::move(r));
        }

        saida.conjuntos.emplace(nomeConjunto, std::move(regras));
    }

    return saida;
}
