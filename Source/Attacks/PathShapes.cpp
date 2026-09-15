//
// Formas de caminho prontas para o PathBehavior.
//

#include "PathShapes.h"

namespace PathShapes {

std::vector<Vector2> Arc(const float forward, const float lateral, int segments) {

    if (segments < 2) segments = 2;

    std::vector<Vector2> pontos;
    pontos.reserve(segments);

    // A barriga é um meio-seno: vale 0 nas duas pontas e 'lateral' no meio.
    // Começa em i = 1 porque o ponto 0 seria a própria posição de disparo, e um
    // waypoint em cima do projétil seria consumido no primeiro frame sem
    // produzir movimento nenhum.
    for (int i = 1; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        pontos.emplace_back(forward * t,
                            lateral * Math::Sin(Math::Pi * t));
    }

    return pontos;
}

std::vector<Vector2> Loop(const float radius, const float exitDistance, int segments) {

    if (segments < 4) segments = 4;

    std::vector<Vector2> pontos;
    pontos.reserve(segments + 1);

    // Círculo de raio 'radius' centrado em (0, radius): assim ele passa pela
    // origem, que é onde o projétil está quando o caminho começa. O percurso
    // sai para frente, sobe pelo lado e volta ao ponto de partida.
    for (int i = 1; i <= segments; ++i) {
        const float angulo = 2.0f * Math::Pi * static_cast<float>(i) / static_cast<float>(segments);
        pontos.emplace_back(radius * Math::Sin(angulo),
                            radius * (1.0f - Math::Cos(angulo)));
    }

    // Ponto de fuga: depois de fechar a volta, segue em frente. Sem ele o
    // caminho terminaria exatamente na origem e o projétil sairia na tangente,
    // que é o mesmo rumo — funciona, mas o ponto explícito deixa a intenção
    // clara e permite controlar a distância.
    if (exitDistance > 0.0f) {
        pontos.emplace_back(exitDistance, 0.0f);
    }

    return pontos;
}

std::vector<Vector2> Zigzag(const float step, const float amplitude, int legs) {

    if (legs < 1) legs = 1;

    std::vector<Vector2> pontos;
    pontos.reserve(legs);

    for (int i = 1; i <= legs; ++i) {
        const float lado = (i % 2 == 0) ? -amplitude : amplitude;
        pontos.emplace_back(step * static_cast<float>(i), lado);
    }

    return pontos;
}

}
