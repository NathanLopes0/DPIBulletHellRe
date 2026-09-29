//
// Formas de caminho prontas para o PathBehavior.
//

#include "PathShapes.h"

#include <map>
#include "../JsonDeDados.h"
#include <string>

namespace {

    // Chave do cache de formas. Duas chamadas com os mesmos parametros
    // descrevem a mesma geometria, entao devolvem o mesmo objeto.
    struct ChaveForma {
        int forma;   // 0 = Arc, 1 = Loop, 2 = Zigzag
        float a;
        float b;
        int n;

        bool operator<(const ChaveForma& o) const {
            if (forma != o.forma) return forma < o.forma;
            if (a != o.a)         return a < o.a;
            if (b != o.b)         return b < o.b;
            return n < o.n;
        }
    };

    // As formas vivem pela duracao do processo. Sao poucas dezenas de pontos
    // cada e ficam compartilhadas por todos os projeteis que as usam, entao o
    // custo de memoria e irrelevante perto do que se economiza em alocacoes.
    std::map<ChaveForma, PathShapes::Path> gCacheDeFormas;

}

namespace PathShapes {

Path Reta(const float distance) {

    const ChaveForma chave{3, distance, 0.f, 1};
    if (const auto it = gCacheDeFormas.find(chave); it != gCacheDeFormas.end()) {
        return it->second;
    }

    auto forma = std::make_shared<const std::vector<Vector2>>(
                     std::vector<Vector2>{ Vector2(distance, 0.f) });
    gCacheDeFormas.emplace(chave, forma);
    return forma;
}

Path Arc(const float forward, const float lateral, int segments) {

    if (segments < 2) segments = 2;

    const ChaveForma chave{0, forward, lateral, segments};
    if (const auto it = gCacheDeFormas.find(chave); it != gCacheDeFormas.end()) {
        return it->second;
    }

    std::vector<Vector2> pontos;
    pontos.reserve(segments);

    // A barriga e um meio-seno: vale 0 nas duas pontas e 'lateral' no meio.
    // Comeca em i = 1 porque o ponto 0 seria a propria posicao de disparo, e um
    // waypoint em cima do projetil seria consumido no primeiro frame sem
    // produzir movimento nenhum.
    for (int i = 1; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        pontos.emplace_back(forward * t, lateral * Math::Sin(Math::Pi * t));
    }

    auto forma = std::make_shared<const std::vector<Vector2>>(std::move(pontos));
    gCacheDeFormas.emplace(chave, forma);
    return forma;
}

Path Loop(const float radius, const float exitDistance, int segments) {

    if (segments < 4) segments = 4;

    const ChaveForma chave{1, radius, exitDistance, segments};
    if (const auto it = gCacheDeFormas.find(chave); it != gCacheDeFormas.end()) {
        return it->second;
    }

    std::vector<Vector2> pontos;
    pontos.reserve(segments + 1);

    // Circulo de raio 'radius' centrado em (0, radius): assim ele passa pela
    // origem, que e onde o projetil esta quando o caminho comeca. O percurso
    // sai para frente, sobe pelo lado e volta ao ponto de partida.
    for (int i = 1; i <= segments; ++i) {
        const float angulo = 2.0f * Math::Pi * static_cast<float>(i) / static_cast<float>(segments);
        pontos.emplace_back(radius * Math::Sin(angulo), radius * (1.0f - Math::Cos(angulo)));
    }

    // Ponto de fuga: depois de fechar a volta, segue em frente.
    if (exitDistance > 0.0f) {
        pontos.emplace_back(exitDistance, 0.0f);
    }

    auto forma = std::make_shared<const std::vector<Vector2>>(std::move(pontos));
    gCacheDeFormas.emplace(chave, forma);
    return forma;
}

Path Zigzag(const float step, const float amplitude, int legs) {

    if (legs < 1) legs = 1;

    const ChaveForma chave{2, step, amplitude, legs};
    if (const auto it = gCacheDeFormas.find(chave); it != gCacheDeFormas.end()) {
        return it->second;
    }

    std::vector<Vector2> pontos;
    pontos.reserve(legs);

    for (int i = 1; i <= legs; ++i) {
        const float lado = (i % 2 == 0) ? -amplitude : amplitude;
        pontos.emplace_back(step * static_cast<float>(i), lado);
    }

    auto forma = std::make_shared<const std::vector<Vector2>>(std::move(pontos));
    gCacheDeFormas.emplace(chave, forma);
    return forma;
}

size_t CachedShapeCount() {
    return gCacheDeFormas.size();
}

}

// ===========================================================================
// Formas vindas de arquivo
// ===========================================================================

namespace {

    /// Monta uma forma a partir de um gerador em C++. Devolve vazio se o tipo
    /// nao existir, para que o chamador possa relatar o problema.
    std::vector<Vector2> DoGerador(const std::string& tipo, float a, float b, int n) {
        if (tipo == "Reta")   return *PathShapes::Reta(a);
        if (tipo == "Arc")    return *PathShapes::Arc(a, b, n);
        if (tipo == "Loop")   return *PathShapes::Loop(a, b, n);
        if (tipo == "Zigzag") return *PathShapes::Zigzag(a, b, n);
        return {};
    }

}

namespace PathShapes {

FormasLidas LerFormas(const std::string& textoJson) {

    FormasLidas saida;

    nlohmann::json raiz;
    try {
        raiz = LerJsonDeDados(textoJson);
    }
    catch (const std::exception& e) {
        saida.problemas.emplace_back(std::string("o arquivo nao e um JSON valido: ") + e.what());
        return saida;
    }

    if (!raiz.is_object()) {
        saida.problemas.emplace_back("o arquivo deveria ser um objeto com um nome de forma por chave");
        return saida;
    }

    for (auto it = raiz.begin(); it != raiz.end(); ++it) {

        const std::string& nome = it.key();
        const auto& corpo = it.value();

        if (!corpo.is_object()) {
            saida.problemas.emplace_back("forma \"" + nome + "\": deveria ser um objeto com \"pontos\" ou \"gerador\"");
            continue;
        }

        std::vector<Vector2> pontos;

        if (corpo.contains("pontos")) {
            const auto& lista = corpo["pontos"];
            if (!lista.is_array() || lista.empty()) {
                saida.problemas.emplace_back("forma \"" + nome + "\": \"pontos\" deveria ser uma lista nao vazia");
                continue;
            }
            bool ok = true;
            for (const auto& par : lista) {
                if (!par.is_array() || par.size() != 2 || !par[0].is_number() || !par[1].is_number()) {
                    saida.problemas.emplace_back("forma \"" + nome + "\": cada ponto deveria ser um par [x, y] de numeros");
                    ok = false;
                    break;
                }
                pontos.emplace_back(par[0].get<float>(), par[1].get<float>());
            }
            if (!ok) continue;
        }
        else if (corpo.contains("gerador")) {
            const auto& g = corpo["gerador"];
            if (!g.is_object() || !g.contains("tipo") || !g["tipo"].is_string()) {
                saida.problemas.emplace_back("forma \"" + nome + "\": \"gerador\" precisa de um campo \"tipo\"");
                continue;
            }
            const std::string tipo = g["tipo"].get<std::string>();
            const float a = g.value("a", 0.0f);
            const float b = g.value("b", 0.0f);
            const int   n = g.value("n", 10);
            pontos = DoGerador(tipo, a, b, n);
            if (pontos.empty()) {
                saida.problemas.emplace_back("forma \"" + nome + "\": gerador \"" + tipo +
                                             "\" nao existe (use Reta, Arc, Loop ou Zigzag)");
                continue;
            }
        }
        else {
            saida.problemas.emplace_back("forma \"" + nome + "\": precisa de \"pontos\" ou \"gerador\"");
            continue;
        }

        saida.formas.emplace(nome, std::move(pontos));
    }

    return saida;
}

}
