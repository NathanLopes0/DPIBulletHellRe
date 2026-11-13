//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

#include "../Source/Regras/Poder.h"

using namespace Poder;

TEST_CASE("Poder: cada orientador tem um poder dele, e quem nao e professor nao tem") {
    const Golpe::Materia todas[] = {Golpe::Materia::Andre, Golpe::Materia::Salles, Golpe::Materia::Ricardo,
                                    Golpe::Materia::Thiago, Golpe::Materia::Luizinho, Golpe::Materia::Vladimir,
                                    Golpe::Materia::Julio, Golpe::Materia::MariaLucia, Golpe::Materia::Lucas,
                                    Golpe::Materia::Vitor, Golpe::Materia::Arroyo};
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
    CHECK(tipos.size() == 11u);    // onze orientadores, onze poderes diferentes
    CHECK(nomes.size() == 11u);
    CHECK(quadros.size() == 11u);
    // Ha dos dois jeitos: os que atacam e os que defendem.
    CHECK(atacam >= 3);
    CHECK(atacam <= 7);
    CHECK(Da(Golpe::Materia::Generico).tipo == Tipo::Nenhum);
    CHECK(QuadroDaMateria(Golpe::Materia::Generico) == -1);
    CHECK(QuadroDaMateria(Golpe::Materia::Andre) == 3);
    CHECK(QuadroDaMateria(Golpe::Materia::Vitor) == 12);
    CHECK(QuadroDaMateria(Golpe::Materia::Arroyo) == kQuadroDoArroyo);   // no fim da folha
}

TEST_CASE("Poder: a busca binaria do Arroyo leva a metade de cima, passo a passo") {
    CHECK(LevaNoPasso(16) == 8);
    CHECK(LevaNoPasso(5) == 3);
    CHECK(LevaNoPasso(1) == 1);
    CHECK(LevaNoPasso(0) == 0);
    int n = 32, passos = 0;
    while (n > 0) { n -= LevaNoPasso(n); ++passos; }
    CHECK(passos == 6);   // log2(32) + 1: o ultimo leva o que sobrou
    const Descricao d = Da(Golpe::Materia::Arroyo);
    CHECK(d.tipo == Tipo::BuscaBinaria);
    CHECK(d.tiros == kPassosDaBusca);
    CHECK_FALSE(Ataca(d.tipo));
    // A previa: comeca com 16 tiros do Hugo, e no fim sobra um.
    auto doHugo = [](const Quadro& q) {
        return std::count_if(q.pontos.begin(), q.pontos.end(), [](const Ponto& p) { return p.marca == Marca::TiroDoHugo; });
    };
    CHECK(doHugo(Previa(Tipo::BuscaBinaria, 0.2f)) == 16);
    CHECK(doHugo(Previa(Tipo::BuscaBinaria, 3.5f)) == 1);
    CHECK(Previa(Tipo::BuscaBinaria, 3.5f).legenda.find("log") != std::string::npos);
}

TEST_CASE("Poder: a carga enche acertando, e as contas dos efeitos") {
    CHECK(Fracao(0) == doctest::Approx(0.0f));
    CHECK(Fracao(kCarga / 2) == doctest::Approx(0.5f).epsilon(0.02));
    CHECK(Fracao(kCarga) == doctest::Approx(1.0f));
    CHECK(Fracao(kCarga * 3) == doctest::Approx(1.0f));

    const Vector2 v(0.0f, -900.0f);
    const Vector2 virado = Guiar(v, Vector2(0.0f, 0.0f), Vector2(500.0f, 0.0f), kGiroDosAgentes, 0.016f);
    CHECK(virado.Length() == doctest::Approx(900.0f));
    CHECK(virado.x > 0.0f);
    const float angulo = std::acos((virado.x * v.x + virado.y * v.y) / (900.0f * 900.0f));
    CHECK(angulo == doctest::Approx(kGiroDosAgentes * 0.016f).epsilon(0.01));
    // Com tempo, ele aponta para o alvo.
    Vector2 w = v;
    for (int i = 0; i < 60; ++i) w = Guiar(w, Vector2(0.0f, 0.0f), Vector2(500.0f, 0.0f), kGiroDosAgentes, 0.016f);
    CHECK(w.x == doctest::Approx(900.0f).epsilon(0.01));
    // Em cima do alvo, nao gira.
    CHECK(Guiar(v, Vector2(5.0f, 5.0f), Vector2(5.0f, 5.0f), kGiroDosAgentes, 0.016f).y == doctest::Approx(-900.0f));

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

TEST_CASE("Poder: os lacos aninhados do Andre - a matriz sai linha por linha, centrada no aluno") {
    const Descricao d = Da(Golpe::Materia::Andre);
    REQUIRE(d.tipo == Tipo::Matriz);
    CHECK(d.tiros == kLinhasDaMatriz * kColunasDaMatriz);
    float antes = -1.0f;
    float soma = 0.0f;
    for (int k = 0; k < d.tiros; ++k) {
        const Celula c = CelulaDaMatriz(k);
        CHECK(c.i == k / kColunasDaMatriz);              // o laco de fora
        CHECK(c.j == k % kColunasDaMatriz);              // o de dentro
        CHECK(c.quando > antes);                         // um depois do outro
        CHECK(c.quando < d.duracao);                     // a matriz inteira sai enquanto o poder dura
        antes = c.quando;
        soma += c.dx;
    }
    CHECK(soma == doctest::Approx(0.0f).epsilon(0.001));  // centrada
    // A linha inteira sai antes de a seguinte comecar.
    CHECK(static_cast<float>(kColunasDaMatriz - 1) * kEntreCelulas < kEntreLinhas);
    CHECK(CelulaDaMatriz(kColunasDaMatriz).dx == doctest::Approx(CelulaDaMatriz(0).dx));
    CHECK(CelulaDaMatriz(1).dx - CelulaDaMatriz(0).dx == doctest::Approx(kEntreColunas));
}

TEST_CASE("Poder: o leque do finally, a porta do semaforo e a pilha") {
    // O tiro do meio do leque vai no alvo, e as pontas abrem por igual.
    CHECK(DesvioNoLeque(2, 5, kAberturaDoFinally) == doctest::Approx(0.0f));
    CHECK(DesvioNoLeque(0, 5, kAberturaDoFinally) == doctest::Approx(-kAberturaDoFinally / 2.0f));
    CHECK(DesvioNoLeque(4, 5, kAberturaDoFinally) == doctest::Approx(kAberturaDoFinally / 2.0f));
    CHECK(DesvioNoLeque(0, 1, kAberturaDoFinally) == doctest::Approx(0.0f));
    CHECK(Da(Golpe::Materia::Luizinho).tiros % 2 == 1);

    // Na porta para quem chega descendo - nem quem sobe, nem quem ja passou.
    CHECK(NaPorta(495.0f, 200.0f, 500.0f));
    CHECK(NaPorta(500.0f, 200.0f, 500.0f));
    CHECK_FALSE(NaPorta(495.0f, -200.0f, 500.0f));
    CHECK_FALSE(NaPorta(495.0f, 0.0f, 500.0f));
    CHECK_FALSE(NaPorta(400.0f, 200.0f, 500.0f));
    CHECK_FALSE(NaPorta(540.0f, 200.0f, 500.0f));
    // O tiro mais rapido do jogo nao pula a porta num quadro.
    CHECK(650.0f * 0.016f < kFolgaDaPorta * 1.5f);
    // Passa mais de um enquanto a trava dura, e a pilha devolve tudo depressa.
    CHECK(Da(Golpe::Materia::Vitor).duracao / kEntrePassagens > 5.0f);
    CHECK(static_cast<float>(kMaiorPilha) * kEntrePops < 1.5f);
}

TEST_CASE("Poder: a previa de cada poder cabe no campo, mexe, tem legenda e volta ao comeco") {
    const Tipo tipos[] = {Tipo::Matriz, Tipo::Pilha, Tipo::Congelar, Tipo::Transacao, Tipo::TryCatch,
                          Tipo::Converter, Tipo::Agentes, Tipo::BalaDePrata, Tipo::PromiseAll, Tipo::Mutex};
    for (const Tipo t : tipos) {
        CAPTURE(static_cast<int>(t));
        std::set<std::string> legendas;
        std::set<int> desenhos;
        bool temSeu = false, temDoHugo = false;
        for (float s = 0.0f; s < kCicloDaPrevia; s += 0.05f) {
            const Quadro q = Previa(t, s);
            CHECK_FALSE(q.legenda.empty());
            CHECK(q.legenda.size() <= 40u);              // cabe numa linha do cartao
            legendas.insert(q.legenda);
            int aluno = 0, hugo = 0;
            float soma = 0.0f;
            for (const Ponto& p : q.pontos) {
                CHECK(p.x >= -0.02f);
                CHECK(p.x <= 1.02f);
                CHECK(p.y >= -0.02f);
                CHECK(p.y <= 1.02f);
                aluno += p.marca == Marca::Aluno ? 1 : 0;
                hugo += p.marca == Marca::Hugo ? 1 : 0;
                temSeu = temSeu || p.marca == Marca::TiroSeu;
                temDoHugo = temDoHugo || p.marca == Marca::TiroDoHugo;
                soma += p.x * 31.0f + p.y * 17.0f;
            }
            for (const Traco& r : q.tracos) {
                CHECK(std::min(r.x0, r.x1) >= -0.02f);
                CHECK(std::max(r.x0, r.x1) <= 1.02f);
                CHECK(std::min(r.y0, r.y1) >= -0.02f);
                CHECK(std::max(r.y0, r.y1) <= 1.02f);
            }
            CHECK(aluno == 1);
            CHECK(hugo == 1);
            desenhos.insert(static_cast<int>(soma * 100.0f));
        }
        CHECK(legendas.size() >= (t == Tipo::Agentes ? 1u : 2u));   // a legenda acompanha os passos
        CHECK(desenhos.size() > 20u);                                // e um desenho animado
        CHECK((temSeu || temDoHugo));
        // Volta ao comeco.
        CHECK(Previa(t, 0.3f).legenda == Previa(t, 0.3f + kCicloDaPrevia).legenda);
        CHECK(Previa(t, 0.3f).pontos.size() == Previa(t, 0.3f + kCicloDaPrevia).pontos.size());
    }
    // Sem poder: so o Hugo e o aluno.
    CHECK(Previa(Tipo::Nenhum, 1.0f).pontos.size() == 2u);
}
