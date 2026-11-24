//
// A final do Lucas: o Phoenix e a fenix.
//

#include "Fenix.h"

#include <algorithm>
#include <cmath>

namespace Fenix {

    Degrau DegrauDo(const Dificuldade::Nivel nivel) {
        Degrau g;
        if (nivel == Dificuldade::Nivel::Dificil) {
            g.periodoDoOito = 4.8f;
            g.entrePenas = 0.75f;
            g.penas = 6;
            g.velocidadeDasPenas = 240.0f;
            g.entreBroadcasts = 3.0f;
            g.tirosDoSocket = 12;
            g.velocidadeDoSocket = 180.0f;
            g.tirosDoRenascimento = 24;
            g.velocidadeDoRenascimento = 220.0f;
            g.maisRapidaPorVersao = 1.2f;
        }
        return g;
    }

    float SeguirX(const float atual, const float aluno, const float dt) {
        const float passo = kRapidezDoLiveView * dt;
        return atual + std::clamp(aluno - atual, -passo, passo);
    }

    Vector2 NoVoo(const float t, const float meioX, const Campo& c, const float periodo, const int versao,
                  const float maisRapidaPorVersao) {
        const float rapidez = std::pow(std::max(1.0f, maisRapidaPorVersao), static_cast<float>(std::max(0, versao)));
        const float a = Math::TwoPi * t * rapidez / std::max(0.5f, periodo);
        // O oito deitado (a lemniscata de Gerono): largo de lado, baixo de altura.
        const float rx = c.w * 0.32f, ry = c.h * 0.14f;
        const float x = std::clamp(meioX + rx * Math::Sin(a), c.x + 40.0f, c.x + c.w - 40.0f);
        const float y = c.y + c.h * 0.32f + ry * Math::Sin(a) * Math::Cos(a) * 2.0f;
        return Vector2(x, y);
    }

    std::array<Vector2, 4> Sockets(const Campo& c) {
        return {Vector2(c.x + c.w * 0.1f, c.y + c.h * 0.45f), Vector2(c.x + c.w * 0.9f, c.y + c.h * 0.45f),
                Vector2(c.x + c.w * 0.3f, c.y + c.h * 0.62f), Vector2(c.x + c.w * 0.7f, c.y + c.h * 0.62f)};
    }
}
