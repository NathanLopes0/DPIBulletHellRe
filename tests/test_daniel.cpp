//
// O Daniel (INF 425 - Ciencia de Dados): a camada pura das provas - o triangulo dos tipos, a coleta,
// as k-medias e o overfitting.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#include "../Source/Attacks/Agrupamento.h"
#include "../Source/Attacks/Ecossistema.h"
#include "../Source/Attacks/Sobreajuste.h"
#include "../Source/Attacks/Tipos.h"

namespace {
    Vector2 V(const float x, const float y) { return Vector2(x, y); }

    bool Perto(const Vector2 a, const Vector2 b, const float tolerancia = 1e-3f) {
        return (a - b).Length() < tolerancia;
    }
}

// ---- O triangulo ----

TEST_CASE("Tipos: fogo, agua e planta - o triangulo, o que passa, o que se absorve e o que machuca") {
    using namespace Tipos;
    CHECK(Vence(kFogo, kPlanta));
    CHECK(Vence(kPlanta, kAgua));
    CHECK(Vence(kAgua, kFogo));
    CHECK_FALSE(Vence(kPlanta, kFogo));
    CHECK_FALSE(Vence(kFogo, kFogo));
    CHECK_FALSE(Vence(-1, kFogo));
    for (int t = 0; t < kTipos; ++t) {
        CHECK(Vence(QueVence(t), t));
        CHECK(Vence(t, Vencido(t)));
        CHECK(QueVence(t) != Vencido(t));
        CHECK(Contra(t, t) == Efeito::Passa);
        CHECK(Contra(t, Vencido(t)) == Efeito::Absorve);
        CHECK(Contra(t, QueVence(t)) == Efeito::Machuca);
        CHECK(Contra(t, -1) == Efeito::Machuca);
        CHECK(std::string(Nome(t)).size() >= 4u);
    }
    CHECK(Contra(-1, kAgua) == Efeito::Machuca);
    CHECK(QueVence(-1) == -1);
    const std::vector<Vector2> lugares = {V(100, 500), V(300, 500), V(500, 500)};
    CHECK(MaisPerto(lugares, V(320, 100)) == 1);
    CHECK(MaisPerto(lugares, V(0, 0)) == 0);
    CHECK(MaisPerto({}, V(0, 0)) == -1);
}

TEST_CASE("Tipos: o triangulo vale no ataque - o dobro no que voce vence, a metade no que te vence") {
    using namespace Tipos;
    for (int t = 0; t < kTipos; ++t) {
        CHECK(GolpeContra(t, Vencido(t)) == Golpe::Vantagem);
        CHECK(FatorDoDano(t, Vencido(t)) == doctest::Approx(kVantagem));
        CHECK(GolpeContra(t, QueVence(t)) == Golpe::Resistencia);
        CHECK(FatorDoDano(t, QueVence(t)) == doctest::Approx(kDesvantagem));
        CHECK(GolpeContra(t, t) == Golpe::Normal);
        CHECK(FatorDoDano(t, -1) == doctest::Approx(1.0f));   // o que nao tem tipo vale o normal
    }
    CHECK(kVantagem > 1.0f);
    CHECK(kDesvantagem < 1.0f);
    // Nenhum inicial fica sem saida: contra os tres tipos juntos, todos tiram o mesmo.
    float soma[kTipos] = {};
    for (int t = 0; t < kTipos; ++t) for (int a = 0; a < kTipos; ++a) soma[t] += FatorDoDano(t, a);
    CHECK(soma[0] == doctest::Approx(soma[1]));
    CHECK(soma[1] == doctest::Approx(soma[2]));
}

TEST_CASE("Tipos: os efeitinhos - a fagulha sobe, a gota cai, a folha desce devagar") {
    using namespace Tipos;
    Variacao::Sorteio s(3u);
    const auto fagulhas = Explodir(kFogo, V(0, 0), 30, 120.0f, s);
    const auto gotas = Explodir(kAgua, V(0, 0), 30, 120.0f, s);
    const auto folhas = Explodir(kPlanta, V(0, 0), 30, 120.0f, s);
    REQUIRE(fagulhas.size() == 30u);
    const auto alturaMedia = [](std::vector<Particula> ps, const float ate) {
        for (Particula& p : ps) for (float t = 0.0f; t < ate; t += 0.02f) Andar(p, 0.02f);
        float y = 0.0f;
        for (const Particula& p : ps) y += p.onde.y;
        return y / static_cast<float>(ps.size());
    };
    CHECK(alturaMedia(fagulhas, 0.5f) < -20.0f);   // sobe (o y cresce para baixo)
    CHECK(alturaMedia(gotas, 0.5f) > 10.0f);       // cai
    const float folha = alturaMedia(folhas, 0.5f);
    CHECK(folha > alturaMedia(fagulhas, 0.5f));
    CHECK(folha < alturaMedia(gotas, 0.5f));       // desce, mas devagar
    for (const Particula& p : folhas) CHECK(p.tipo == kPlanta);
    Particula p = fagulhas.front();
    CHECK(Viva(p));
    for (int k = 0; k < 100; ++k) Andar(p, 0.02f);
    CHECK_FALSE(Viva(p));
}

TEST_CASE("Tipos: a escolha fica longe de onde o aluno nasce, e nenhum monstrinho logo acima dele") {
    using namespace Tipos;
    const float x = 0.0f, y = 54.0f, largura = 1200.0f, altura = 746.0f;
    const std::vector<Vector2> lugares = LugaresDaEscolha(x, y, largura, altura);
    REQUIRE(lugares.size() == static_cast<size_t>(kTipos));
    // O aluno nasce no chao, no meio (Battle: um sprite acima do fundo do campo).
    const Vector2 nasce(x + largura / 2.0f, y + altura - 64.0f);
    for (const Vector2& l : lugares) {
        CHECK((l - nasce).Length() > 380.0f);
        CHECK(l.y < y + altura * 0.5f);   // na metade de cima
    }
    // Andar 120 px para cima, como antes, nao chega em nenhum.
    CHECK((lugares[1] - (nasce + Vector2(0.0f, -120.0f))).Length() > kToqueDaEscolha);
    // E escolher pede ficar em cima, nao so passar: o anel enche em kParaEscolher.
    CHECK(kParaEscolher > 2.0f * kToqueDaEscolha / 300.0f);   // mais que atravessar o circulo a toda
}

// ---- A coleta e o conjunto de teste ----

TEST_CASE("Ecossistema: as ondas ensinam o triangulo - absorver, passar, desviar, e tudo junto") {
    using namespace Ecossistema;
    Variacao::Sorteio s(5u);
    CHECK(OndaNoTempo(1.0f) == Onda::Nenhuma);                 // a escolha
    CHECK(OndaNoTempo(kFimDaEscolha + 0.1f) == Onda::Absorver);
    CHECK(OndaNoTempo(kFimDoAbsorver + 0.1f) == Onda::Passar);
    CHECK(OndaNoTempo(kFimDoPassar + 0.1f) == Onda::Desviar);
    CHECK(OndaNoTempo(19.9f) == Onda::Misturada);
    CHECK(kFimDaEscolha >= Tipos::kTempoDaEscolha);
    for (int t = 0; t < Tipos::kTipos; ++t) {
        CHECK(Tipos::Contra(t, TipoDaOnda(Onda::Absorver, t, s)) == Tipos::Efeito::Absorve);
        CHECK(Tipos::Contra(t, TipoDaOnda(Onda::Passar, t, s)) == Tipos::Efeito::Passa);
        CHECK(Tipos::Contra(t, TipoDaOnda(Onda::Desviar, t, s)) == Tipos::Efeito::Machuca);
    }
    std::set<int> misturados;
    for (int k = 0; k < 60; ++k) misturados.insert(TipoDaOnda(Onda::Misturada, 0, s));
    CHECK(misturados.size() == static_cast<size_t>(Tipos::kTipos));
    CHECK(TipoDaOnda(Onda::Nenhuma, 0, s) == -1);
}

TEST_CASE("Ecossistema: cada especie mora numa regiao do grafico, e no teste a distribuicao muda") {
    using namespace Ecossistema;
    Variacao::Sorteio s(9u);
    // Fogo ataca muito e defende pouco; planta o contrario.
    CHECK(CentroDaEspecie(Tipos::kFogo).x > CentroDaEspecie(Tipos::kPlanta).x);
    CHECK(CentroDaEspecie(Tipos::kFogo).y < CentroDaEspecie(Tipos::kPlanta).y);
    for (int t = 0; t < Tipos::kTipos; ++t) {
        Vector2 media;
        for (int k = 0; k < 200; ++k) {
            const Vector2 p = LugarNoGrafico(t, s);
            CHECK(p.x >= 0.0f);
            CHECK(p.x <= 1.0f);
            CHECK(p.y >= 0.0f);
            CHECK(p.y <= 1.0f);
            media += p * (1.0f / 200.0f);
        }
        CHECK(Perto(media, CentroDaEspecie(t), 0.05f));
        // No conjunto de teste, o ataque se inverte.
        CHECK(CentroDaEspecie(t, true).x == doctest::Approx(1.0f - CentroDaEspecie(t).x));
    }
    // As fontes: no alto, uma de cada lado e uma no meio; o teste vem das laterais.
    CHECK(XDaFonte(0) < XDaFonte(1));
    CHECK(XDaFonte(1) < XDaFonte(2));
    CHECK(OrigemDoTeste(0, 1.0f).x == 0.0f);
    CHECK(OrigemDoTeste(1, 1.0f).x == 1.0f);
    // O Dificil manda mais e mais rapido.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreAmostras < n.entreAmostras);
    CHECK(d.velocidade > n.velocidade);
}

// ---- As k-medias ----

TEST_CASE("Agrupamento: as k-medias - o centroide vai para a media, e o pesado puxa o da celula dele") {
    using namespace Agrupamento;
    const std::vector<Vector2> pontos = {V(0, 0), V(2, 0), V(0, 2), V(2, 2), V(10, 10), V(12, 10)};
    const std::vector<Vector2> centroides = {V(1, 1), V(11, 9)};
    const auto grupos = Atribuir(pontos, centroides);
    CHECK(grupos == std::vector<int>{0, 0, 0, 0, 1, 1});
    const auto novos = Atualizar(pontos, grupos, centroides, {}, 0.0f);
    CHECK(Perto(novos[0], Vector2(1.0f, 1.0f)));
    CHECK(Perto(novos[1], Vector2(11.0f, 10.0f)));
    // O outlier no grupo 1, pesando 2: o centroide anda para ele.
    const auto puxados = Atualizar(pontos, grupos, centroides, {Vector2(14.0f, 13.0f)}, 2.0f);
    CHECK(Perto(puxados[1], Vector2((10.0f + 12.0f + 28.0f) / 4.0f, (10.0f + 10.0f + 26.0f) / 4.0f)));
    CHECK(Perto(puxados[0], novos[0]));
    // Um grupo sem ninguem fica onde esta.
    CHECK(Perto(Atualizar({V(0, 0)}, {0}, {V(0, 0), V(50, 50)}, {}, 0.0f)[1], Vector2(50.0f, 50.0f)));
}

TEST_CASE("Agrupamento: o tipo do grupo e o da maioria, as sementes nascem longe, e o outlier mais ainda") {
    using namespace Agrupamento;
    const std::vector<int> tipos = {0, 0, 1, 2, 2, 2};
    const std::vector<int> grupos = {0, 0, 0, 1, 1, 1};
    CHECK(TipoDoGrupo(tipos, grupos, 0) == 0);
    CHECK(TipoDoGrupo(tipos, grupos, 1) == 2);
    CHECK(TipoDoGrupo(tipos, grupos, 2) == -1);
    Variacao::Sorteio s(4u);
    const std::vector<Vector2> pontos = {V(0, 0), V(1, 0), V(0, 1), V(100, 0), V(101, 1), V(50, 90), V(51, 91)};
    for (int k = 0; k < 10; ++k) {
        const auto sementes = Sementes(pontos, 3, s);
        REQUIRE(sementes.size() == 3u);
        for (size_t i = 0; i < sementes.size(); ++i) {
            for (size_t j = i + 1; j < sementes.size(); ++j) CHECK((sementes[i] - sementes[j]).Length() > 40.0f);
        }
    }
    const std::vector<Vector2> centros = {V(200, 200), V(300, 220)};
    const Vector2 outlier = LugarDoOutlier(centros, V(0, 0), V(1000, 600), s);
    for (const Vector2& c : centros) CHECK((outlier - c).Length() > 250.0f);
    CHECK(kOutliersDesde < kInstavelDesde);
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreIteracoes < n.entreIteracoes);
    CHECK(d.tirosDoAnel > n.tirosDoAnel);
}

// ---- O overfitting ----

TEST_CASE("Sobreajuste: o modelo grava os passos, preve para onde vai e decora onde mais passou") {
    using namespace Sobreajuste;
    Memoria m;
    for (int k = 0; k < 100; ++k) Registrar(m, V(static_cast<float>(k) * 10.0f, 500.0f), static_cast<float>(k) * 0.1f);
    CHECK(static_cast<int>(m.pontos.size()) == kMaisRegistros);   // so os ultimos
    // Andando para a direita, a previsao fica a frente do ultimo ponto.
    Memoria reta;
    for (int k = 0; k < 8; ++k) Registrar(reta, V(100.0f + 50.0f * static_cast<float>(k), 400.0f), kEntreRegistros * static_cast<float>(k));
    const Vector2 prev = Previsao(reta, 0.5f);
    CHECK(prev.x > reta.pontos.back().x + 50.0f);
    CHECK(prev.y == doctest::Approx(400.0f));
    CHECK(Perto(Previsao(Memoria{}, 1.0f), Vector2::Zero));
    // Parado num canto quase o tempo todo: e la que ele decorou.
    Memoria parado;
    for (int k = 0; k < 20; ++k) Registrar(parado, k < 16 ? V(80, 600) : V(900, 300), kEntreRegistros * static_cast<float>(k));
    const auto decorados = Decorados(parado, 60.0f, 2);
    REQUIRE(decorados.size() == 2u);
    CHECK(Perto(decorados[0], V(80, 600), 1.0f));
    CHECK(Perto(decorados[1], V(900, 300), 1.0f));
}

TEST_CASE("Sobreajuste: a curva passa por todos os pontos, com voltas, e os nucleos abrem um por vez") {
    using namespace Sobreajuste;
    const std::vector<Vector2> pontos = {V(100, 500), V(200, 450), V(300, 520), V(420, 480), V(500, 500)};
    const auto curva = Curva(pontos, 8, 20.0f);
    // Passa exatamente por cada ponto (o overfitting: erro zero no treino).
    for (const Vector2& p : pontos) {
        float perto = 1e9f;
        for (const Vector2& c : curva) perto = std::min(perto, (c - p).Length());
        CHECK(perto < 0.01f);
    }
    // E balanca entre eles: vai mais longe da reta que os proprios pontos.
    float maisLonge = 0.0f;
    for (const Vector2& c : curva) maisLonge = std::max(maisLonge, std::fabs(c.y - 490.0f));
    CHECK(maisLonge > 30.0f);
    // Os nucleos: um aberto de cada vez, todos na validacao.
    for (float t = 0.0f; t < kValidacao; t += 0.3f) {
        int abertos = 0;
        for (int k = 0; k < kNucleos; ++k) abertos += Aberto(k, t) ? 1 : 0;
        CHECK(abertos == 1);
    }
    for (int k = 0; k < kNucleos; ++k) CHECK(Aberto(k, kValidacao + 0.1f));
    CHECK(kTreino < kValidacao);
    CHECK(kValidacao + kRiscando < 20.0f);
    // Em volta do professor, na orbita.
    for (int k = 0; k < kNucleos; ++k) {
        const float d = (OndeDoNucleo(k, V(600, 100), 2.0f) - V(600, 100)).Length();
        CHECK(d <= kRaioDaOrbita + 0.01f);
        CHECK(d >= kRaioDaOrbita * 0.5f);
    }
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreTiros < n.entreTiros);
    CHECK(d.vidaDoNucleo > n.vidaDoNucleo);
}
