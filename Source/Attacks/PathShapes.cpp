//
// Formas de caminho prontas para o PathBehavior.
//

#include "PathShapes.h"

#include <map>

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
