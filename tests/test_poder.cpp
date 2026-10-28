//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Poder.h"

using namespace Poder;

TEST_CASE("Poder: cada orientador tem um poder dele, e quem nao e professor nao tem") {
    const Golpe::Materia todas[] = {Golpe::Materia::Andre, Golpe::Materia::Salles, Golpe::Materia::Ricardo,
                                    Golpe::Materia::Thiago, Golpe::Materia::Luizinho, Golpe::Materia::Vladimir,
                                    Golpe::Materia::Julio, Golpe::Materia::MariaLucia, Golpe::Materia::Lucas,
                                    Golpe::Materia::Vitor};
    std::set<int> tipos;
    std::set<std::string> nomes;
    std::set<int> quadros;
    int atacam = 0;
    for (const Golpe::Materia m : todas) {
        const Descricao d = Da(m);
        CAPTURE(d.nome);
        CHECK(d.tipo != Tipo::Nenhum);
        CHECK_FALSE(d.nome.empty());
        CHECK_FALSE(d.efeito.empty());
        CHECK(d.duracao >= 0.0f);
        tipos.insert(static_cast<int>(d.tipo));
        nomes.insert(d.nome);
        quadros.insert(QuadroDaMateria(m));
        if (Ataca(d.tipo)) ++atacam;
    }
    CHECK(tipos.size() == 10u);    // dez orientadores, dez poderes diferentes
    CHECK(nomes.size() == 10u);
    CHECK(quadros.size() == 10u);
    // HA DOS DOIS JEITOS: os que atacam e os que defendem.
    CHECK(atacam >= 3);
    CHECK(atacam <= 7);
    CHECK(Da(Golpe::Materia::Generico).tipo == Tipo::Nenhum);
    CHECK(QuadroDaMateria(Golpe::Materia::Generico) == -1);
    CHECK(QuadroDaMateria(Golpe::Materia::Andre) == 3);
    CHECK(QuadroDaMateria(Golpe::Materia::Vitor) == 12);
}

TEST_CASE("Poder: a carga enche acertando, e as contas dos efeitos") {
    CHECK(Fracao(0) == doctest::Approx(0.0f));
    CHECK(Fracao(kCarga / 2) == doctest::Approx(0.5f).epsilon(0.02));
    CHECK(Fracao(kCarga) == doctest::Approx(1.0f));
    CHECK(Fracao(kCarga * 3) == doctest::Approx(1.0f));

    // O fork(): o filho fica no espelho, dentro do campo.
    CHECK(Espelho(100.0f, 0.0f, 1200.0f) == doctest::Approx(1100.0f));
    CHECK(Espelho(600.0f, 0.0f, 1200.0f) == doctest::Approx(600.0f));
    CHECK(Espelho(-50.0f, 0.0f, 1200.0f) == doctest::Approx(1200.0f));

    // Os agentes: no anel, e girando.
    const Vector2 c(500.0f, 500.0f);
    for (int k = 0; k < 3; ++k) {
        CHECK((Agente(c, k, 3, 0.7f, 70.0f) - c).Length() == doctest::Approx(70.0f));
    }
    CHECK((Agente(c, 0, 3, 0.0f, 70.0f) - Agente(c, 0, 3, 0.3f, 70.0f)).Length() > 10.0f);
    CHECK((Agente(c, 0, 3, 0.0f, 70.0f) - Agente(c, 1, 3, 0.0f, 70.0f)).Length() > 50.0f);

    CHECK(NoAlcance(c, c + Vector2(100.0f, 0.0f), 100.0f));
    CHECK_FALSE(NoAlcance(c, c + Vector2(100.1f, 0.0f), 100.0f));
}
