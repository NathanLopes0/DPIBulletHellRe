//
// A fase 2 do Lucas: o DevTools aberto, e o CSS mudando ao vivo.
//

#include "doctest.h"

#include <cmath>
#include <set>

#include "../Source/Attacks/Lucas/Estilo.h"

using namespace Estilo;

namespace {
    float Distancia(const Ponto& a, const Ponto& b) { return std::hypot(a.x - b.x, a.y - b.y); }

    void Extensao(const std::array<Ponto, kCartoes>& p, float& largura, float& altura) {
        float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
        for (const Ponto& q : p) { x0 = std::min(x0, q.x); x1 = std::max(x1, q.x); y0 = std::min(y0, q.y); y1 = std::max(y1, q.y); }
        largura = x1 - x0;
        altura = y1 - y0;
    }
}

TEST_CASE("Estilo: a grade e 3 x 6 com o gap entre vizinhos, e o CSS a transforma inteira") {
    const Ponto centro{600.0f, 290.0f};
    Css css;
    const auto p = Posicoes(css, centro);
    CHECK(Distancia(p[0], p[1]) == doctest::Approx(kGapEstreito));          // vizinhos na linha
    CHECK(Distancia(p[0], p[kColunas]) == doctest::Approx(kGapEstreito));   // vizinhos na coluna
    float w = 0, h = 0;
    Extensao(p, w, h);
    CHECK(w > h);

    // flex-direction: column DEITA a grade: a largura vira altura.
    css.coluna = true;
    float wc = 0, hc = 0;
    Extensao(Posicoes(css, centro), wc, hc);
    CHECK(wc == doctest::Approx(h));
    CHECK(hc == doctest::Approx(w));

    // O ROTATE MANTEM CADA CARTAO A MESMA DISTANCIA DO CENTRO, e o scale a
    // multiplica.
    Css girada;
    girada.rotacao = 45.0f;
    Css grande;
    grande.escala = kEscalaGrande;
    const auto pg = Posicoes(girada, centro);
    const auto pe = Posicoes(grande, centro);
    for (int i = 0; i < kCartoes; ++i) {
        CHECK(Distancia(pg[i], centro) == doctest::Approx(Distancia(p[i], centro)));
        CHECK(Distancia(pe[i], centro) == doctest::Approx(kEscalaGrande * Distancia(p[i], centro)));
    }
    CHECK(Distancia(pg[0], p[0]) > 1.0f);
}

TEST_CASE("Estilo: o justify-content redistribui cada linha no conteiner, como o flexbox") {
    const Ponto centro{600.0f, 290.0f};
    Css css;
    css.justificar = Flex::Justificar::EspacoEntre;
    const auto p = Posicoes(css, centro);
    // SPACE-BETWEEN: a ponta de cada linha encosta na borda do conteiner.
    CHECK(p[0].x == doctest::Approx(centro.x - kConteiner / 2.0f + kGapEstreito / 2.0f));
    CHECK(p[kColunas - 1].x == doctest::Approx(centro.x + kConteiner / 2.0f - kGapEstreito / 2.0f));
    css.justificar = Flex::Justificar::Inicio;
    const auto q = Posicoes(css, centro);
    CHECK(q[0].x == doctest::Approx(centro.x - kConteiner / 2.0f + kGapEstreito / 2.0f));
    CHECK(q[1].x - q[0].x == doctest::Approx(kGapEstreito));
}

TEST_CASE("Estilo: display: none esconde uma linha flex inteira, deitada ou nao") {
    Css css;
    css.linhaOculta = 1;
    int ocultos = 0;
    for (int i = 0; i < kCartoes; ++i) ocultos += Oculto(css, i) ? 1 : 0;
    CHECK(ocultos == kColunas);
    css.coluna = true;   // deitada, a linha flex continua com 6 - agora em pe
    ocultos = 0;
    for (int i = 0; i < kCartoes; ++i) ocultos += Oculto(css, i) ? 1 : 0;
    CHECK(ocultos == kColunas);
}

TEST_CASE("Estilo: as regras se revezam, toda regra aparece, e a grade torta volta") {
    Variacao::Sorteio s = Variacao::SorteioDa(3u, kFaseDoEstilo);
    Css css;
    Tipo anterior = Tipo::Hover;
    std::set<int> vistos;
    int voltou = 0;
    for (int k = 0; k < 400; ++k) {
        const float antes = css.rotacao;
        const Regra r = Proxima(s, css, anterior);
        CHECK(r.tipo != anterior);
        CHECK_FALSE(r.declaracao.empty());
        vistos.insert(static_cast<int>(r.tipo));
        if (antes != 0.0f && css.rotacao == 0.0f) ++voltou;
        anterior = r.tipo;
    }
    CHECK(vistos.size() == 9u);
    CHECK(voltou > 10);
}

TEST_CASE("Estilo: o placar e CSS, o fixed espalha de borda a borda, e o hover desce e volta") {
    Regra r;
    r.seletor = ".grade";
    r.declaracao = "gap: 96px;";
    CHECK(Placar(r) == ".grade {\ngap: 96px; }");
    Css css;
    css.rotacao = 45.0f;
    css.escala = kEscalaGrande;
    Variacao::Sorteio s = Variacao::SorteioDa(1u, kFaseDoEstilo);
    for (int k = 0; k < 50; ++k) {
        const Regra q = Proxima(s, css, Tipo::Hover);
        if (q.tipo == Tipo::Girar || q.tipo == Tipo::Escalar) {
            CHECK(q.declaracao.find("transform: rotate(") == 0u);
            CHECK(q.declaracao.find("scale(") != std::string::npos);
        }
    }
    CHECK(Espalhado(0, 1200.0f, 700.0f).x > 0.0f);
    CHECK(Espalhado(kCartoes - 1, 1200.0f, 700.0f).x < 1200.0f);
    CHECK(Hover(0.0f) == 0.0f);
    CHECK(Hover(kDuracaoDoHover / 2.0f) == doctest::Approx(kDescidaDoHover));
    CHECK(Hover(kDuracaoDoHover) == 0.0f);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).porRegra < kPorRegra);
    CHECK(XDaGrade(300.0f, 1200.0f) == doctest::Approx(900.0f));   // o espelho do professor
    CHECK(XDaGrade(600.0f, 1200.0f) == doctest::Approx(600.0f));
    CHECK_FALSE(Repinta(Tipo::Hover));
    CHECK_FALSE(Repinta(Tipo::Fixar));
    CHECK(Repinta(Tipo::Girar));
    CHECK(Repinta(Tipo::Esconder));
}
