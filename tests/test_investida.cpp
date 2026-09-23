// Testes da investida repetida: o Modifier que pulsa o modulo e a Motion que
// re-aponta em instantes discretos. So a camada pura, sem SDL.

#include "doctest.h"
#include "../Source/Attacks/PathAim.h"

namespace {
    constexpr float DT = 1.0f / 60.0f;
    float anguloEntre(const Vector2& a, const Vector2& b) {
        const float g = Math::ToDegrees(Math::Atan2(a.y, a.x) - Math::Atan2(b.y, b.x));
        float d = Math::Abs(g);
        if (d > 180.0f) d = 360.0f - d;
        return d;
    }
}

// ---------------------------------------------------------------------------
// AjustarModulo — o espelho de DirecionarPreservandoModulo
// ---------------------------------------------------------------------------

TEST_CASE("AjustarModulo: define o modulo pedido") {
    const Vector2 v = AjustarModulo(Vector2(30.f, 40.f), 200.f);
    CHECK(v.Length() == doctest::Approx(200.f));
}

TEST_CASE("AjustarModulo: preserva a direcao") {
    const Vector2 original(30.f, 40.f);
    const Vector2 v = AjustarModulo(original, 7.f);
    CHECK(anguloEntre(v, original) == doctest::Approx(0.f).epsilon(0.001));
}

TEST_CASE("AjustarModulo: velocidade nula fica como esta") {
    // Nao ha direcao a preservar; inventar uma seria decidir pelo chamador.
    const Vector2 v = AjustarModulo(Vector2::Zero, 500.f);
    CHECK(v.Length() == doctest::Approx(0.f));
}

TEST_CASE("AjustarModulo: modulo negativo vira zero") {
    const Vector2 v = AjustarModulo(Vector2(10.f, 0.f), -50.f);
    CHECK(v.Length() == doctest::Approx(0.f));
}

TEST_CASE("AjustarModulo e DirecionarPreservandoModulo sao complementares") {
    // Um troca o modulo e mantem a direcao; o outro troca a direcao e mantem o
    // modulo. Aplicados em sequencia, o resultado tem o modulo de um e a
    // direcao do outro.
    const Vector2 v = DirecionarPreservandoModulo(AjustarModulo(Vector2(3.f, 4.f), 100.f),
                                                  Vector2(0.f, 1.f), 1.f);
    CHECK(v.Length() == doctest::Approx(100.f));
    CHECK(anguloEntre(v, Vector2(0.f, 1.f)) == doctest::Approx(0.f).epsilon(0.001));
}

// ---------------------------------------------------------------------------
// CalcularFaseDoPulso — toda a aritmetica de tempo do Modifier
// ---------------------------------------------------------------------------

namespace {
    constexpr RitmoDeCiclos kRitmo{0.5f, 0.2f, 1.0f, 3};
    FaseDoPulso fase(float t) { return CalcularFaseDoPulso(t, kRitmo, 800.f, 15.f); }
}

TEST_CASE("Pulso: antes do atraso nao esta ativo nem terminou") {
    const auto f = fase(0.3f);
    CHECK_FALSE(f.ativo);
    CHECK(f.ciclo == 0);
}

TEST_CASE("Pulso: a investida vem primeiro, e dura o que foi pedido") {
    CHECK(fase(0.55f).investindo);
    CHECK(fase(0.55f).modulo == doctest::Approx(800.f));
    CHECK(fase(0.69f).investindo);
    CHECK_FALSE(fase(0.75f).investindo);
    CHECK(fase(0.75f).modulo == doctest::Approx(15.f));
}

TEST_CASE("Pulso: conta os ciclos") {
    CHECK(fase(0.6f).ciclo == 0);
    CHECK(fase(1.9f).ciclo == 1);   // 0.5 + 1.2
    CHECK(fase(2.95f).ciclo == 2);  // ja no terceiro ciclo, ainda investindo
}

TEST_CASE("Pulso: a sequencia termina numa INVESTIDA, nunca numa pausa") {
    // Sao N investidas com N-1 pausas entre elas. Terminar na pausa deixaria o
    // projetil lento demais para sair da tela, e ele viraria um obstaculo
    // permanente ocupando uma vaga do pool.
    const auto ultimaInvestida = fase(2.95f);          // 0.5 + 2*1.2 + 0.05
    CHECK(ultimaInvestida.ativo);
    CHECK(ultimaInvestida.investindo);
    CHECK(ultimaInvestida.modulo == doctest::Approx(800.f));

    // A ultima investida vai de t=2.9 a t=3.1; logo depois viria a pausa.
    const auto logoDepois = fase(3.15f);               // a pausa que nao acontece
    CHECK_FALSE(logoDepois.ativo);
    CHECK(logoDepois.ciclo == 3);                      // encerrou

    // Como a ultima fase ativa foi uma investida, a velocidade que fica valendo
    // e a alta - e o projetil sai da tela sozinho.
    CHECK(ultimaInvestida.modulo > 100.f);
}

TEST_CASE("Pulso: depois da ultima repeticao para de agir") {
    const auto f = fase(10.0f);
    CHECK_FALSE(f.ativo);
    CHECK(f.ciclo == 3);            // e por isso que o Modifier se encerra
}

TEST_CASE("Pulso: zero repeticoes encerra de imediato") {
    const auto f = CalcularFaseDoPulso(1.f, RitmoDeCiclos{0.f, 0.2f, 1.f, 0}, 800.f, 15.f);
    CHECK_FALSE(f.ativo);
    CHECK(f.ciclo <= 0);
}

// ---------------------------------------------------------------------------
// A investida inteira, simulada na ordem do motor
// ---------------------------------------------------------------------------

TEST_CASE("Investida repetida: re-aponta e pulsa, sem uma peca saber da outra") {
    const Vector2 jogador(600.f, 600.f);
    Vector2 pos(600.f, 120.f), vel(0.f, 200.f);

    constexpr RitmoDeCiclos ritmo{0.f, 0.2f, 1.0f, 6};
    const float vInvestida = 700.f, vPausa = 15.f;
    const int repeticoes = ritmo.repeticoes;

    float t = 0.f;
    int miradas = 0;
    float piorErro = 0.f, menorModuloNaInvestida = 1e9f, maiorModuloNaPausa = 0.f;
    float menorDistancia = 1e9f;

    while (t < ritmo.Ciclo() * static_cast<float>(repeticoes)) {
        pos += vel * DT;                                   // 1. RigidBody

        if (miradas < repeticoes && t >= ritmo.InicioDaInvestida(miradas)) {  // 2. Motion
            const Vector2 paraJogador = jogador - pos;
            vel = DirecionarPreservandoModulo(vel, paraJogador, vel.Length());
            piorErro = Math::Max(piorErro, anguloEntre(vel, paraJogador));
            ++miradas;
        }

        const auto f = CalcularFaseDoPulso(t, ritmo, vInvestida, vPausa);   // 3. Modifier
        if (f.ativo) {
            vel = AjustarModulo(vel, f.modulo);
            if (f.investindo) menorModuloNaInvestida = Math::Min(menorModuloNaInvestida, vel.Length());
            else              maiorModuloNaPausa = Math::Max(maiorModuloNaPausa, vel.Length());
        }
        menorDistancia = Math::Min(menorDistancia, (pos - jogador).Length());
        t += DT;
    }

    CHECK(miradas == repeticoes);
    CHECK(piorErro < 0.5f);                                    // sempre aponta para o jogador
    CHECK(menorModuloNaInvestida == doctest::Approx(vInvestida));
    CHECK(maiorModuloNaPausa == doctest::Approx(vPausa));
    CHECK(menorDistancia < 200.f);                             // rondou o jogador durante a cacada

    // E o mais importante para o pool: termina em velocidade de investida, nao
    // de pausa. Terminar devagar deixaria o projetil preso na tela.
    CHECK(vel.Length() == doctest::Approx(vInvestida));
    CHECK((pos - jogador).Length() > 300.f);                   // ja esta indo embora
}

TEST_CASE("Investida repetida: a pausa nao pode ser parada de verdade") {
    // A direcao mora dentro do vetor velocidade. Com modulo zero ela se perde, e
    // a mirada seguinte nao tem o que girar.
    Vector2 parado = AjustarModulo(Vector2(0.f, 200.f), 0.f);
    const Vector2 reapontado = DirecionarPreservandoModulo(parado, Vector2(100.f, 0.f), parado.Length());
    CHECK(reapontado.Length() == doctest::Approx(0.f));

    // Com um resto de velocidade, a mirada funciona.
    Vector2 quaseParado = AjustarModulo(Vector2(0.f, 200.f), 15.f);
    const Vector2 ok = DirecionarPreservandoModulo(quaseParado, Vector2(100.f, 0.f), quaseParado.Length());
    CHECK(ok.Length() == doctest::Approx(15.f));
    CHECK(ok.x == doctest::Approx(15.f));
}

// ---------------------------------------------------------------------------
// O ritmo compartilhado: nenhum tempo precisa ser calculado a mao
// ---------------------------------------------------------------------------

TEST_CASE("Ritmo: o ciclo e a soma das duas fases") {
    constexpr RitmoDeCiclos r{0.f, 0.25f, 1.5f, 4};
    CHECK(r.Ciclo() == doctest::Approx(1.75f));
}

TEST_CASE("Ritmo: as investidas comecam em multiplos do ciclo, a partir do atraso") {
    constexpr RitmoDeCiclos r{0.5f, 0.2f, 1.0f, 4};
    CHECK(r.InicioDaInvestida(0) == doctest::Approx(0.5f));
    CHECK(r.InicioDaInvestida(1) == doctest::Approx(1.7f));
    CHECK(r.InicioDaInvestida(3) == doctest::Approx(4.1f));
}

TEST_CASE("Ritmo: a duracao total nao conta a ultima pausa") {
    // N investidas com N-1 pausas entre elas.
    constexpr RitmoDeCiclos r{0.f, 0.2f, 1.0f, 6};
    CHECK(r.DuracaoTotal() == doctest::Approx(6 * 0.2f + 5 * 1.0f));
    CHECK(RitmoDeCiclos{0.f, 0.2f, 1.0f, 0}.DuracaoTotal() == doctest::Approx(0.f));
}

TEST_CASE("Ritmo: mudar a pausa move as miradas E o pulso juntos") {
    // Este e o teste que existe por causa do acoplamento antigo: o intervalo
    // entre miradas era um numero digitado a mao, que precisava ser igual a
    // investida + pausa. Agora os dois saem do mesmo objeto.
    constexpr RitmoDeCiclos curto{0.f, 0.2f, 1.0f, 3};
    constexpr RitmoDeCiclos longo{0.f, 0.2f, 2.0f, 3};   // so a pausa mudou

    // A Motion re-aponta no inicio de cada investida...
    CHECK(curto.InicioDaInvestida(1) == doctest::Approx(1.2f));
    CHECK(longo.InicioDaInvestida(1) == doctest::Approx(2.2f));

    // ...e o Modifier concorda, sem ninguem recalcular nada.
    CHECK(CalcularFaseDoPulso(curto.InicioDaInvestida(1) + 0.01f, curto, 800.f, 15.f).investindo);
    CHECK(CalcularFaseDoPulso(longo.InicioDaInvestida(1) + 0.01f, longo, 800.f, 15.f).investindo);

    // E no instante em que a investida comecaria com a pausa curta, o ritmo
    // longo ainda esta pausado - que era exatamente o bug de configuracao.
    CHECK_FALSE(CalcularFaseDoPulso(1.21f, longo, 800.f, 15.f).investindo);
}
