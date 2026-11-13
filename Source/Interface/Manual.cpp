//
// O manual do jogo: as paginas da tela "Como Jogar".
//

#include "Manual.h"

#include "../Motor/JsonDeDados.h"

namespace Manual {

    namespace {
        /// As cadeias de 'campo', ignorando o que nao for texto (com aviso).
        std::vector<std::string> Textos(const nlohmann::json& pagina, const char* campo, const std::string& onde,
                                        std::vector<std::string>& problemas) {
            std::vector<std::string> saida;
            if (!pagina.contains(campo)) return saida;
            const auto& lista = pagina.at(campo);
            if (!lista.is_array()) {
                problemas.push_back(onde + ": \"" + campo + "\" deveria ser uma lista de frases.");
                return saida;
            }
            for (const auto& item : lista) {
                if (item.is_string()) saida.push_back(item.get<std::string>());
                else problemas.push_back(onde + ": um item de \"" + campo + "\" nao e texto, e ficou de fora.");
            }
            return saida;
        }
    }

    Lido LerManual(const std::string& textoJson) {
        Lido lido;
        nlohmann::json raiz;
        try {
            raiz = LerJsonDeDados(textoJson);
        } catch (const std::exception& e) {
            lido.problemas.push_back(std::string("como-jogar.json nao e JSON valido: ") + e.what());
            return lido;
        }
        if (!raiz.is_object() || !raiz.contains("paginas") || !raiz.at("paginas").is_array()) {
            lido.problemas.push_back("como-jogar.json precisa de uma lista \"paginas\".");
            return lido;
        }

        int n = 0;
        for (const auto& item : raiz.at("paginas")) {
            ++n;
            const std::string onde = "pagina " + std::to_string(n);
            if (!item.is_object()) {
                lido.problemas.push_back(onde + " nao e um objeto, e ficou de fora.");
                continue;
            }
            Pagina p;
            if (item.contains("titulo") && item.at("titulo").is_string()) p.titulo = item.at("titulo").get<std::string>();
            p.linhas = Textos(item, "linhas", onde, lido.problemas);
            p.noComputador = Textos(item, "no_computador", onde, lido.problemas);
            if (item.contains("mostrar")) {
                if (item.at("mostrar") == "mencoes") p.mencoes = true;
                else lido.problemas.push_back(onde + ": \"mostrar\" so conhece \"mencoes\", e foi ignorado.");
            }

            if (p.titulo.empty() || (p.linhas.empty() && !p.mencoes)) {
                lido.problemas.push_back(onde + " nao tem titulo ou nao tem linhas, e ficou de fora.");
                continue;
            }
            if (static_cast<int>(p.linhas.size() + p.noComputador.size()) > kLinhasNoMaximo) {
                lido.problemas.push_back(onde + " tem mais de " + std::to_string(kLinhasNoMaximo)
                                         + " linhas; as que sobram nao aparecem.");
            }
            lido.paginas.push_back(std::move(p));
        }
        return lido;
    }

    std::vector<std::string> Linhas(const Pagina& p, const bool arcade) {
        std::vector<std::string> saida = p.linhas;
        if (!arcade) saida.insert(saida.end(), p.noComputador.begin(), p.noComputador.end());
        if (static_cast<int>(saida.size()) > kLinhasNoMaximo) saida.resize(kLinhasNoMaximo);
        return saida;
    }

    int Virar(const int atual, const int passo, const int quantas) {
        if (quantas <= 0) return 0;
        const int alvo = atual + passo;
        if (alvo < 0) return 0;
        if (alvo >= quantas) return quantas - 1;
        return alvo;
    }

    std::string Indicador(const int atual, const int quantas) {
        return std::to_string(atual + 1) + " de " + std::to_string(quantas);
    }
}
