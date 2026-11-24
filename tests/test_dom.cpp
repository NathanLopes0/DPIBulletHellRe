//
// A <div> das fases 1 e 2 do Lucas (Dom.h).
//

#include "doctest.h"

#include <cmath>
#include <set>
#include <string>

#include "../Source/Attacks/Dom.h"

using namespace Dom;

namespace {
    const Campo kCampo{0.0f, 0.0f, 1200.0f, 720.0f};

    /// A area que a <div> cobre agora (as quatro caixas da grade somadas).
    float Area(const Div& d, const float agora) {
        float a = 0.0f;
        for (const Caixa& k : Caixas(d, agora)) a += k.largura * k.altura;
        return a;
    }
}

TEST_CASE("Dom: a div nasce pequena, quica na parte de cima e so desce com o z-index") {
    Variacao::Sorteio s(7u);
    Div d = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
    CHECK(d.largura == doctest::Approx(kLarguraInicial));
    CHECK(d.velocidade.Length() == doctest::Approx(110.0f));
    float maisBaixo = 0.0f;
    for (int k = 0; k < 4000; ++k) {
        Andar(d, 0.016f, k * 0.016f, Vector2(600.0f, 650.0f), kCampo, s);
        CHECK(d.centro.x - d.largura / 2.0f >= kCampo.x - 0.01f);
        CHECK(d.centro.x + d.largura / 2.0f <= kCampo.x + kCampo.w + 0.01f);
        CHECK(d.centro.y - d.altura / 2.0f >= kCampo.y - 0.01f);
        maisBaixo = std::max(maisBaixo, d.centro.y + d.altura / 2.0f);
    }
    CHECK(maisBaixo <= kCampo.h * kFaixaDaDiv + 0.01f);
    Aplicar(d, Efeito::Desce, 100.0f);
    maisBaixo = 0.0f;
    for (int k = 0; k < 4000; ++k) {
        Andar(d, 0.016f, 100.0f + k * 0.016f, Vector2(600.0f, 650.0f), kCampo, s);
        maisBaixo = std::max(maisBaixo, d.centro.y + d.altura / 2.0f);
        CHECK(d.centro.y + d.altura / 2.0f <= kCampo.h + 0.01f);
    }
    CHECK(maisBaixo > kCampo.h * 0.9f);
}

TEST_CASE("Dom: toda regra CSS piora a div, se acumula, e a div nunca some") {
    for (const Efeito e : kDoCss) {
        CAPTURE(static_cast<int>(e));
        Variacao::Sorteio s(1u);
        Div d = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
        const Div antes = d;
        CHECK_FALSE(Inutil(d, e));
        CHECK_FALSE(Aplicar(d, e, 1.0f));
        // Nada encolhe, nada fica mais lento, nada transborda menos.
        CHECK(d.largura >= antes.largura);
        CHECK(d.altura >= antes.altura);
        CHECK(d.velocidade.Length() >= antes.velocidade.Length() - 0.01f);
        CHECK(d.entreTransbordos <= antes.entreTransbordos);
        // E algo mudou: a forma, o jeito de andar ou o transbordo.
        const bool mudou = d.largura != antes.largura || d.altura != antes.altura || d.redonda || d.grade || d.pulsa ||
                           d.centraliza || d.desce || d.entreTransbordos != antes.entreTransbordos ||
                           d.velocidade.Length() > antes.velocidade.Length() + 0.01f;
        CHECK(mudou);
        CHECK_FALSE(EhJs(e));
        CHECK(std::string(Texto(e)).find(';') != std::string::npos);
        for (int k = 0; k < 30; ++k) Aplicar(d, e, 2.0f + k);
        for (float t = 0.0f; t < 10.0f; t += 0.1f) CHECK_FALSE(Caixas(d, t).empty());
    }
    Variacao::Sorteio s(2u);
    Div d = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
    const float l0 = d.largura;
    Aplicar(d, Efeito::Largura, 1.0f);
    const float l1 = d.largura;
    Aplicar(d, Efeito::Largura, 1.0f);
    CHECK(l1 > l0);
    CHECK(d.largura > l1);                                   // acumula
    for (int k = 0; k < 10; ++k) Aplicar(d, Efeito::Largura, 1.0f);
    CHECK(d.largura == doctest::Approx(kMaiorLargura));      // com teto
    CHECK(Inutil(d, Efeito::Largura));
    CHECK(d.mudancas == 12);
    // display: grid vira quatro caixas.
    Aplicar(d, Efeito::Grade, 10.0f);
    CHECK(Caixas(d, 10.0f).size() == 4u);
    // animation: o tamanho vai e volta.
    Div pulsante = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
    Aplicar(pulsante, Efeito::Pulsa, 0.0f);
    float menor = 1e9f, maior = 0.0f;
    for (float t = 0.0f; t < kPeriodoDoPulso; t += 0.02f) {
        menor = std::min(menor, Area(pulsante, t));
        maior = std::max(maior, Area(pulsante, t));
    }
    CHECK(menor == doctest::Approx(kLarguraInicial * kAlturaInicial).epsilon(0.02));
    CHECK(maior > menor * 1.4f);
    // margin: 0 auto: fica em cima do aluno.
    Div centrada = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
    Aplicar(centrada, Efeito::Centraliza, 0.0f);
    const Vector2 aluno(150.0f, 650.0f);
    for (int k = 0; k < 600; ++k) Andar(centrada, 0.016f, k * 0.016f, aluno, kCampo, s);
    CHECK(std::fabs(centrada.centro.x - aluno.x) < 120.0f);
}

TEST_CASE("Dom: a div transborda sozinha - para baixo, e a redonda em leque") {
    Variacao::Sorteio s(4u);
    Div d = Nova(kCampo, 0.0f, 2.4f, 0.0f, s);
    d.centro = Vector2(600.0f, 200.0f);
    CHECK_FALSE(Atirar(d, 1.0f, Vector2(0.0f, 700.0f)).transbordo);
    CHECK(Atirar(d, 2.4f, Vector2(0.0f, 700.0f)).transbordo);
    CHECK_FALSE(Atirar(d, 2.5f, Vector2(0.0f, 700.0f)).transbordo);
    const auto gotas = Gotas(d, 0.0f);
    CHECK(gotas.size() == 2u);                               // 72 px / 48
    for (const Gota& g : gotas) {
        CHECK(g.rumo.y == doctest::Approx(1.0f));
        CHECK(g.onde.y == doctest::Approx(200.0f + kAlturaInicial / 2.0f));
    }
    // Mais larga, mais gotas.
    Aplicar(d, Efeito::Largura, 3.0f);
    Aplicar(d, Efeito::Largura, 3.0f);
    CHECK(Gotas(d, 3.0f).size() > gotas.size());
    // Na grade, so as caixas de baixo transbordam - as de cima cairiam dentro delas.
    Div grade = d;
    Aplicar(grade, Efeito::Grade, 3.0f);
    for (const Gota& g : Gotas(grade, 3.0f)) CHECK(g.onde.y > grade.centro.y);
    // overflow: scroll aperta o ritmo, ate o piso.
    const float antes = d.entreTransbordos;
    Aplicar(d, Efeito::Transborda, 3.0f);
    CHECK(d.entreTransbordos < antes);
    for (int k = 0; k < 10; ++k) Aplicar(d, Efeito::Transborda, 3.0f);
    CHECK(d.entreTransbordos == doctest::Approx(kMenorTransbordo));
    // border-radius: a metade de baixo do circulo, em leque - para os lados e para baixo, nunca para cima.
    Aplicar(d, Efeito::Redonda, 3.0f);
    const auto redondas = Gotas(d, 3.0f);
    bool esquerda = false, direita = false, desce = false;
    for (const Gota& g : redondas) {
        CHECK(g.rumo.Length() == doctest::Approx(1.0f));
        CHECK(g.rumo.y > 0.0f);
        esquerda = esquerda || g.rumo.x < -0.5f;
        direita = direita || g.rumo.x > 0.5f;
        desce = desce || g.rumo.y > 0.9f;
    }
    CHECK(esquerda);
    CHECK(direita);
    CHECK(desce);
}

TEST_CASE("Dom: as regras JavaScript dao comportamento - atirar, mirar, revidar, despejar, estourar") {
    Variacao::Sorteio s(3u);
    Div d = Nova(kCampo, 110.0f, 100.0f, 0.0f, s);
    d.centro = Vector2(600.0f, 200.0f);
    const Vector2 longe(100.0f, 680.0f), embaixo(600.0f, 680.0f);
    CHECK_FALSE(Atirar(d, 5.0f, longe).anel);
    Aplicar(d, Efeito::Intervalo, 0.0f);
    CHECK_FALSE(Atirar(d, 0.5f, longe).anel);
    CHECK(Atirar(d, 1.0f, longe).anel);
    CHECK_FALSE(Atirar(d, 1.1f, longe).anel);
    const float antes = d.intervalo;
    Aplicar(d, Efeito::Intervalo, 2.0f);
    CHECK(d.intervalo < antes);                               // mais um setInterval aperta o ritmo
    // forEach: o leque mirado.
    Aplicar(d, Efeito::Leque, 3.0f);
    CHECK_FALSE(Atirar(d, 3.2f, longe).leque);
    CHECK(Atirar(d, 3.8f, longe).leque);
    // onmouseover: so com o aluno embaixo dela.
    Aplicar(d, Efeito::Sobre, 4.0f);
    CHECK_FALSE(Atirar(d, 4.0f, longe).queda);
    CHECK(Atirar(d, 4.0f, embaixo).queda);
    CHECK_FALSE(Atirar(d, 4.0f + kEntreQuedas * 0.5f, embaixo).queda);
    CHECK(Atirar(d, 4.0f + kEntreQuedas, embaixo).queda);
    // setTimeout: estoura uma vez - e a div continua la.
    Aplicar(d, Efeito::Bomba, 10.0f);
    CHECK(Inutil(d, Efeito::Bomba));
    CHECK(Atirar(d, 10.0f + kPavio, longe).explosao);
    CHECK(d.bombaEm < 0.0f);
    CHECK_FALSE(Caixas(d, 10.0f + kPavio).empty());
    CHECK_FALSE(Inutil(d, Efeito::Bomba));
    // onclick.
    CHECK_FALSE(Revidar(d, 1.0f));
    Aplicar(d, Efeito::Revide, 1.0f);
    CHECK(Revidar(d, 1.0f));
    CHECK_FALSE(Revidar(d, 1.0f + kEntreRevides * 0.5f));
    CHECK(Revidar(d, 1.0f + kEntreRevides));
    CHECK(Aplicar(d, Efeito::Clone, 3.0f));                   // a batalha clona
    // mousemove: a div vira para o aluno.
    Div seguidora = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
    Aplicar(seguidora, Efeito::Segue, 0.0f);
    Aplicar(seguidora, Efeito::Desce, 0.0f);
    const Vector2 aluno(600.0f, 680.0f);
    const float distancia = (seguidora.centro - aluno).Length();
    for (int k = 0; k < 120; ++k) Andar(seguidora, 0.016f, k * 0.016f, aluno, kCampo, s);
    CHECK((seguidora.centro - aluno).Length() < distancia);
    // Acertou o aluno: recua, e por um tempo nao vai atras dele.
    const float perto = (seguidora.centro - aluno).Length();
    Recuar(seguidora, aluno, 2.0f);
    for (int k = 0; k < 40; ++k) Andar(seguidora, 0.016f, 2.0f + k * 0.016f, aluno, kCampo, s);
    CHECK((seguidora.centro - aluno).Length() > perto);
    // Todas as regras tem texto, e cada linguagem sorteia so as dela - todas elas.
    for (const Efeito e : kDoJs) { CHECK(EhJs(e)); CHECK_FALSE(std::string(Texto(e)).empty()); }
    std::set<int> vistas;
    for (int k = 0; k < 400; ++k) {
        const Efeito e = Sortear(true, s);
        CHECK(EhJs(e));
        vistas.insert(static_cast<int>(e));
    }
    CHECK(vistas.size() == std::size(kDoJs));
    // Com o alvo, quase nunca sai uma regra que nao muda nada nele.
    Div redonda = Nova(kCampo, 110.0f, 2.4f, 0.0f, s);
    for (const Efeito e : {Efeito::Redonda, Efeito::Pulsa, Efeito::Centraliza, Efeito::Desce}) Aplicar(redonda, e, 0.0f);
    int inuteis = 0;
    for (int k = 0; k < 400; ++k) inuteis += Inutil(redonda, Sortear(false, s, &redonda)) ? 1 : 0;
    CHECK(inuteis < 10);
}

TEST_CASE("Dom: as pecas cobrem a div, e a redonda so o circulo") {
    Variacao::Sorteio s(5u);
    Div d = Nova(kCampo, 0.0f, 2.4f, 0.0f, s);
    d.centro = Vector2(600.0f, 300.0f);
    d.largura = 120.0f;
    d.altura = 60.0f;
    const auto pecas = Pecas(d, 0.0f, 30.0f);
    CHECK(pecas.size() == 8u);
    for (const Vector2& p : pecas) CHECK(Toca(d, p, 0.0f, 0.0f));
    CHECK_FALSE(Toca(d, Vector2(600.0f, 400.0f), 10.0f, 0.0f));
    CHECK(Toca(d, Vector2(600.0f, 335.0f), 10.0f, 0.0f));
    d.redonda = true;
    for (const Vector2& p : Pecas(d, 0.0f, 20.0f)) CHECK((p - d.centro).Length() <= 30.0f + 0.01f);
    // O Dificil e mais apertado em tudo.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), df = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(df.entreLances < n.entreLances);
    CHECK(static_cast<float>(df.regrasPorLance) / df.entreLances > static_cast<float>(n.regrasPorLance) / n.entreLances);
    CHECK(df.velocidadeDasRegras > n.velocidadeDasRegras);
    CHECK(df.entreTransbordos < n.entreTransbordos);
    // Mesmo no Normal, as regras chegam em quantidade: mais de uma por segundo.
    CHECK(static_cast<float>(n.regrasPorLance) / n.entreLances > 1.0f);
}

TEST_CASE("Dom: a regra mirada na div faz a curva, mas nao vira de uma vez") {
    // Indo para a direita, com a div bem acima: num quadro, vira so o giro de um quadro.
    const Vector2 v(170.0f, 0.0f);
    const Vector2 um = Curvar(v, Vector2(0.0f, 0.0f), Vector2(0.0f, -300.0f), kGiroDaRegra, 0.016f);
    CHECK(um.Length() == doctest::Approx(170.0f));
    const float virou = -Math::ToDegrees(std::atan2(um.y, um.x));
    CHECK(virou == doctest::Approx(kGiroDaRegra * 0.016f).epsilon(0.01));
    // Com tempo, aponta para a div, e nao passa dela.
    const Vector2 muito = Curvar(v, Vector2(0.0f, 0.0f), Vector2(0.0f, -300.0f), kGiroDaRegra, 5.0f);
    CHECK(muito.x == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(muito.y == doctest::Approx(-170.0f));
    // Ja na direcao certa, nada muda.
    const Vector2 reta = Curvar(v, Vector2(0.0f, 0.0f), Vector2(500.0f, 0.0f), kGiroDaRegra, 0.016f);
    CHECK(reta.x == doctest::Approx(170.0f));
    CHECK(reta.y == doctest::Approx(0.0f));
}
