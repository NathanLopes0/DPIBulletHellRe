// Contrato de ProjectileMotion: a Motion decide a direcao, o modulo e
// preservado. Simula a trajetoria na mesma ordem do motor, sem SDL.

#include "doctest.h"
#include "../Source/Attacks/PathAim.h"

#include <functional>

namespace {

    struct Corpo {
        Vector2 pos;
        Vector2 vel;
    };

    constexpr float DT = 1.0f / 60.0f;

    /// Regra de passo de uma Motion de caminho: recebe a velocidade atual e a
    /// direcao ate o waypoint, devolve a nova velocidade.
    using RegraDePasso = std::function<Vector2(const Vector2& vel, const Vector2& dir)>;

    /**
     * Simula, frame a frame, na ORDEM DO MOTOR:
     *   1. RigidBody integra (pos += vel * dt)   - componentes rodam primeiro
     *   2. Motion atualiza a velocidade          - Projectile::OnUpdate
     *   3. Modifiers ajustam o que a Motion fez  - Projectile::OnUpdate
     *
     * Motion: ativa em tAtivar, mirando no jogador (como o homing).
     * Modifier: SlowDown que multiplica a velocidade por 'fator' em tSlowDown.
     */
    Vector2 SimularCaminho(const RegraDePasso& regra, const float tSlowDown, const float fator) {
        Corpo c{Vector2(400.f, 100.f), Vector2(0.f, 200.f)};
        const Vector2 jogador(200.f, 500.f);
        const float tAtivar = 0.5f, velocidade = 200.f;

        bool ativo = false, slowDownFeito = false;
        Vector2 waypoint;

        for (float t = 0.f; t < 2.0f; t += DT) {
            c.pos += c.vel * DT;                                   // 1. integra

            if (!ativo && t >= tAtivar) {                          // 2. Motion ativa
                ContextoDeResolucao ctx;
                ctx.posicaoProjetil = c.pos;
                ctx.velocidadeProjetil = c.vel;
                ctx.posicaoJogador = jogador;
                ctx.temJogador = true;
                const auto r = ResolverReferencial(ctx, Mira(Mira::MirarNoJogador));
                waypoint = r.origem + Vector2(r.cos, r.sin) * 1200.f;
                c.vel = Vector2(r.cos, r.sin) * velocidade;
                ativo = true;
            } else if (ativo) {                                    // 2. Motion segue
                c.vel = regra(c.vel, waypoint - c.pos);
            }

            if (!slowDownFeito && t >= tSlowDown) {                // 3. Modifier
                c.vel *= fator;
                slowDownFeito = true;
            }
        }
        return c.pos;
    }

    /// Referencia: o HomingBehavior original. Aponta UMA vez e se encerra; nada
    /// mais escreve a velocidade depois disso.
    Vector2 SimularHomingOriginal(const float tSlowDown, const float fator) {
        const RegraDePasso naoFazNada = [](const Vector2& vel, const Vector2&) { return vel; };
        return SimularCaminho(naoFazNada, tSlowDown, fator);
    }

    // A regra ANTIGA do PathBehavior: modulo fixo em pathSpeed.
    const RegraDePasso regraAntiga = [](const Vector2&, const Vector2& dir) {
        Vector2 d = dir;
        d.Normalize();
        return d * 200.f;
    };

    // A regra NOVA: preserva o modulo.
    const RegraDePasso regraNova = [](const Vector2& vel, const Vector2& dir) {
        return DirecionarPreservandoModulo(vel, dir, 200.f);
    };

    float Distancia(const Vector2& a, const Vector2& b) { return (a - b).Length(); }
}

// ---------------------------------------------------------------------------
// Testes da regra pura
// ---------------------------------------------------------------------------

TEST_CASE("DirecionarPreservandoModulo: aponta na direcao pedida") {
    const Vector2 v = DirecionarPreservandoModulo(Vector2(0.f, 100.f), Vector2(1.f, 0.f), 50.f);
    CHECK(v.x == doctest::Approx(100.f));
    CHECK(v.y == doctest::Approx(0.f));
}

TEST_CASE("DirecionarPreservandoModulo: preserva o modulo, nao importa a escala da direcao") {
    const Vector2 v = DirecionarPreservandoModulo(Vector2(30.f, 40.f), Vector2(0.f, 999.f), 1.f);
    CHECK(v.Length() == doctest::Approx(50.f));
}

TEST_CASE("DirecionarPreservandoModulo: velocidade nula usa o modulo minimo") {
    const Vector2 v = DirecionarPreservandoModulo(Vector2::Zero, Vector2(0.f, 1.f), 75.f);
    CHECK(v.Length() == doctest::Approx(75.f));
}

TEST_CASE("DirecionarPreservandoModulo: direcao nula mantem a velocidade atual") {
    const Vector2 v = DirecionarPreservandoModulo(Vector2(10.f, 20.f), Vector2::Zero, 75.f);
    CHECK(v.x == doctest::Approx(10.f));
    CHECK(v.y == doctest::Approx(20.f));
}

// ---------------------------------------------------------------------------
// O defeito, e a prova de que o teste e sensivel a ele
// ---------------------------------------------------------------------------

TEST_CASE("Sem Modifier: as tres regras chegam ao mesmo lugar") {
    // Controle. Sem SlowDown, modulo fixo e modulo preservado sao identicos,
    // entao as tres simulacoes devem coincidir. Se este falhar, o problema esta
    // no simulador, nao nas regras.
    const Vector2 homing = SimularHomingOriginal(99.f, 1.0f);
    CHECK(Distancia(SimularCaminho(regraAntiga, 99.f, 1.0f), homing) < 1.0f);
    CHECK(Distancia(SimularCaminho(regraNova,   99.f, 1.0f), homing) < 1.0f);
}

TEST_CASE("Regra ANTIGA diverge do homing quando ha SlowDown depois da ativacao") {
    // Documenta o defeito que motivou o commit. Se um dia este teste passar a
    // falhar, significa que o simulador ficou insensivel ao problema.
    const Vector2 homing = SimularHomingOriginal(1.0f, 0.5f);
    const Vector2 antiga = SimularCaminho(regraAntiga, 1.0f, 0.5f);
    CHECK(Distancia(antiga, homing) > 50.0f);
}

TEST_CASE("Regra NOVA coincide com o homing mesmo com SlowDown depois da ativacao") {
    const Vector2 homing = SimularHomingOriginal(1.0f, 0.5f);
    const Vector2 nova   = SimularCaminho(regraNova, 1.0f, 0.5f);
    CHECK(Distancia(nova, homing) < 1.0f);
}
