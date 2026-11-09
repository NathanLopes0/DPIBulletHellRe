//
// A fase 1 do Luizinho: o polimorfismo.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "../Source/Attacks/Polimorfismo.h"

using namespace Polimorfismo;

namespace {
    /// A distancia do ponto ao segmento ab.
    float AoSegmento(const Vector2& p, const Vector2& a, const Vector2& b) {
        const Vector2 ab = b - a;
        const float t = std::clamp(Vector2::Dot(p - a, ab) / ab.LengthSq(), 0.0f, 1.0f);
        return (p - (a + ab * t)).Length();
    }

    /// O ponto esta no contorno da forma (num dos lados do poligono, ou no circulo).
    bool NoContorno(const Classe c, const Vector2& p) {
        if (c == Classe::Circulo) return std::fabs(p.Length() - 1.0f) < 0.001f;
        const auto v = Vertices(c);
        float perto = 1e9f;
        for (size_t k = 0; k < v.size(); ++k) perto = std::min(perto, AoSegmento(p, v[k], v[(k + 1) % v.size()]));
        return perto < 0.001f;
    }
}

TEST_CASE("Polimorfismo: cada classe tem o seu nome, e os vertices da forma") {
    CHECK(std::string(Nome(Classe::Circulo)) == "Circulo");
    CHECK(std::string(Nome(Classe::Estrela)) == "Estrela");
    CHECK(Vertices(Classe::Circulo).empty());
    CHECK(Vertices(Classe::Quadrado).size() == 4);
    CHECK(Vertices(Classe::Triangulo).size() == 3);
    CHECK(Vertices(Classe::Estrela).size() == 10);
    for (int c = 0; c < kClasses; ++c) {
        float maior = 0.0f;
        for (const Vector2& v : Vertices(static_cast<Classe>(c))) maior = std::max(maior, v.Length());
        if (static_cast<Classe>(c) != Classe::Circulo) CHECK(maior == doctest::Approx(1.0f));
    }
}

TEST_CASE("Polimorfismo: o contorno tem os pontos pedidos, todos no perimetro da forma, e nenhum fora do raio") {
    for (int c = 0; c < kClasses; ++c) {
        const auto classe = static_cast<Classe>(c);
        CAPTURE(Nome(classe));
        for (const int pontos : {3, 16, 20}) {
            const auto contorno = Contorno(classe, pontos);
            REQUIRE(static_cast<int>(contorno.size()) == pontos);
            for (const Vector2& p : contorno) {
                CHECK(NoContorno(classe, p));
                CHECK(p.Length() <= 1.0001f);
            }
        }
        CHECK(Contorno(classe, 0).empty());
    }
}

TEST_CASE("Polimorfismo: os pontos do contorno ficam por igual ao longo do perimetro") {
    // No quadrado de 16 pontos, um a cada quarto de lado: todos os vizinhos a mesma distancia.
    const auto q = Contorno(Classe::Quadrado, 16);
    const float lado = (Vertices(Classe::Quadrado)[1] - Vertices(Classe::Quadrado)[0]).Length();
    for (size_t i = 0; i < q.size(); ++i) {
        CHECK((q[(i + 1) % q.size()] - q[i]).Length() == doctest::Approx(lado / 4.0f).epsilon(0.01));
    }
}

TEST_CASE("Polimorfismo: a mesma sorte da o mesmo comando, e todos os comandos aparecem") {
    std::set<Comando> vistos;
    for (std::uint32_t s = 0; s < 300; ++s) {
        const Chamada a = Proxima(s, 3), b = Proxima(s, 3);
        CHECK(a.comando == b.comando);
        CHECK(a.objetos == b.objetos);
        vistos.insert(a.comando);
        // Um objeto na instancia e no tipo dinamico; os do laco, sem dois iguais seguidos.
        if (a.comando == Comando::Laco) {
            CHECK(a.objetos.size() == 3);
            for (size_t k = 1; k < a.objetos.size(); ++k) CHECK(a.objetos[k] != a.objetos[k - 1]);
        } else {
            CHECK(a.objetos.size() == 1);
        }
    }
    CHECK(vistos.size() == 3);
    CHECK(Proxima(5, 4).objetos.size() <= 4);
}

TEST_CASE("Polimorfismo: sem peso, o comando nao sai") {
    Plano so;
    so.pesoDaInstancia = 0.0f;
    so.pesoDoLaco = 0.0f;
    so.pesoDoDinamico = 1.0f;
    for (std::uint32_t s = 0; s < 50; ++s) CHECK(Proxima(s, 3, so).comando == Comando::TipoDinamico);
}

TEST_CASE("Polimorfismo: o placar e os rotulos - o tipo dinamico so aparece depois da chamada") {
    Chamada i{Comando::Instancia, {Classe::Estrela}};
    CHECK(Placar(i, false) == "new Estrela().desenhar();");
    CHECK(Rotulo(i, 0, true) == "new Estrela()");

    Chamada l{Comando::Laco, {Classe::Circulo, Classe::Quadrado, Classe::Triangulo}};
    CHECK(Placar(l, false) == "for (Forma f : formas) f.desenhar();");
    CHECK(Rotulo(l, 1, true) == "Quadrado");

    Chamada d{Comando::TipoDinamico, {Classe::Triangulo}};
    CHECK(Placar(d, false) == "Forma f = sortear();\nf.desenhar();");
    CHECK(Placar(d, true) == "Forma f = sortear();\nf.desenhar(); // Triangulo");
    CHECK(Rotulo(d, 0, false) == "Forma f = ?");
    CHECK(Rotulo(d, 0, true) == "Forma f = Triangulo");
}

TEST_CASE("Polimorfismo: os lugares ficam na faixa pedida e longe uns dos outros") {
    for (std::uint32_t s = 0; s < 100; ++s) {
        const auto lugares = Lugares(4, s, 1200.0f, 230.0f, 470.0f);
        REQUIRE(lugares.size() == 4);
        for (size_t a = 0; a < lugares.size(); ++a) {
            CHECK(lugares[a].x >= kMargem - 0.01f);
            CHECK(lugares[a].x <= 1200.0f - kMargem + 0.01f);
            CHECK(lugares[a].y >= 230.0f);
            CHECK(lugares[a].y <= 470.0f);
            for (size_t b = a + 1; b < lugares.size(); ++b) CHECK((lugares[a] - lugares[b]).Length() > 150.0f);
        }
    }
}

TEST_CASE("Polimorfismo: a duracao conta o voo, a revelacao e cada chamada do laco") {
    const Chamada i{Comando::Instancia, {Classe::Circulo}};
    const Chamada d{Comando::TipoDinamico, {Classe::Circulo}};
    const Chamada l{Comando::Laco, {Classe::Circulo, Classe::Estrela, Classe::Circulo}};
    CHECK(Duracao(i) == doctest::Approx(kVooDoNew + kAntesDaChamada + kTraco));
    CHECK(Duracao(d) == doctest::Approx(Duracao(i) + kRevelacao));
    CHECK(Duracao(l) == doctest::Approx(Duracao(i) + 2.0f * kEntreChamadas));
}

TEST_CASE("Polimorfismo: os pesos de cada luta ficam em volta dos de sempre") {
    for (std::uint32_t s = 0; s < 100; ++s) {
        Variacao::Sorteio sorteio(s);
        const Plano p = Sortear(sorteio);
        CHECK(p.pesoDaInstancia >= 0.2f);
        CHECK(p.pesoDaInstancia <= 0.35f);
        CHECK(p.pesoDoLaco >= 0.3f);
        CHECK(p.pesoDoDinamico <= 0.45f);
    }
}

TEST_CASE("Polimorfismo: o degrau do Normal e o de sempre, e o do Dificil aperta tudo") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.menorPasso == kMenorPasso);
    CHECK(n.maiorPasso == kMaiorPasso);
    CHECK(n.noLaco == kNoLaco);
    CHECK(n.pontos == kPontos);
    CHECK(n.velocidade == kVelocidade);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.menorPasso < n.menorPasso);
    CHECK(d.maiorPasso < n.maiorPasso);
    CHECK(d.noLaco > n.noLaco);
    CHECK(d.pontos > n.pontos);
    CHECK(d.velocidade > n.velocidade);
    // So no Dificil a forma gira enquanto cresce.
    CHECK(n.giro == 0.0f);
    CHECK(d.giro > 0.0f);
    // A forma que sai a kRaio do objeto leva tempo para chegar ao aluno: os objetos ficam a mais de
    // 200 px do fundo do campo.
    CHECK(200.0f / d.velocidade > 0.9f);
    // E o vertice vive o bastante para atravessar a faixa dos objetos ate o fundo do campo.
    for (const Degrau& g : {n, d}) CHECK(kRaio + g.velocidade * (kVidaDoTraco - kTraco) > 500.0f);
}
