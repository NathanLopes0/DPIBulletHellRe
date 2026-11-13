//
// O Daniel (INF 425 - Ciencia de Dados): a camada pura das quatro provas e do sorteio.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#include "../Source/Attacks/Agrupamento.h"
#include "../Source/Attacks/Chamada.h"
#include "../Source/Attacks/Classificacao.h"
#include "../Source/Attacks/Explicacao.h"
#include "../Source/Attacks/Fase.h"
#include "../Source/Attacks/Preparacao.h"

namespace {
    Vector2 V(const float x, const float y) { return Vector2(x, y); }

    bool Perto(const Vector2 a, const Vector2 b, const float tolerancia = 1e-3f) {
        return (a - b).Length() < tolerancia;
    }
}

// ---- A preparacao ----

TEST_CASE("Preparacao: as etapas somam a fase, e o min-max e a padronizacao fazem o que dizem") {
    using namespace Preparacao;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(kDuracoes[0] + 0.1f) == 1);
    CHECK(EtapaNoTempo(19.9f) == 2);
    for (const Passo& p : kPassos) CHECK(EtapaNoTempo(p.quando) == 2);
    CHECK(EtapaNoTempo(kEstouro) == 1);
    CHECK(EtapaNoTempo(kImputacao) == 1);

    const std::vector<Vector2> ps = {V(0.1f, 0.2f), V(0.5f, 0.9f), V(0.3f, 0.4f)};
    // O min-max leva a caixa dos pontos para a de destino: os cantos vao para os cantos.
    const Caixa destino{Vector2(0.6f, 0.6f), Vector2(1.0f, 1.0f)};
    const auto mm = MinMax(ps, destino);
    CHECK(Perto(mm[0], Vector2(0.6f, 0.6f)));
    CHECK(Perto(mm[1], Vector2(1.0f, 1.0f)));
    CHECK(Perto(mm[2], Vector2(0.8f, 0.6f + 0.4f * (0.2f / 0.7f))));
    // A padronizacao: a media vai para o centro, e o desvio vira a escala.
    const auto z = Padronizar(ps, Vector2(0.5f, 0.5f), Vector2(0.2f, 0.2f));
    CHECK(Perto(Media(z), Vector2(0.5f, 0.5f)));
    CHECK(Perto(Desvio(z), Vector2(0.2f, 0.2f)));
    // Um eixo sem largura vai para o meio.
    const auto reto = MinMax({V(0.2f, 0.5f), V(0.4f, 0.5f)}, destino);
    CHECK(reto[0].y == doctest::Approx(0.8f));
    // O min-max vai para o canto do chao em que o aluno esta: embaixo, do lado dele.
    CHECK(CantoDoAluno(0.2f) == 0);
    CHECK(CantoDoAluno(0.8f) == 1);
    CHECK(CaixaNoCanto(CantoDoAluno(0.2f), 0.45f).de.y == doctest::Approx(0.0f));
    CHECK(CaixaNoCanto(CantoDoAluno(0.8f), 0.45f).ate.x == doctest::Approx(1.0f));
    // A caixa do canto cabe no grafico.
    for (int c = 0; c < 4; ++c) {
        const Caixa k = CaixaNoCanto(c, 0.45f);
        CHECK(k.de.x >= 0.0f);
        CHECK(k.ate.x <= 1.0f + 1e-6f);
        CHECK(k.ate.x - k.de.x == doctest::Approx(0.45f));
    }
}

TEST_CASE("Preparacao: a coleta faz tres especies, e o z-score acha os outliers dela") {
    using namespace Preparacao;
    Variacao::Sorteio s(425u);
    const auto especies = Especies(s);
    for (int i = 0; i < kEspecies; ++i) {
        for (int j = i + 1; j < kEspecies; ++j) {
            CHECK((especies[static_cast<size_t>(i)] - especies[static_cast<size_t>(j)]).Length() > 0.25f);
        }
    }
    std::vector<Dado> dados;
    for (int k = 0; k < 60; ++k) dados.push_back(Coletar(s, especies, 0.12f, 0.12f));
    int outliers = 0, nulos = 0;
    std::vector<Vector2> onde;
    for (const Dado& d : dados) {
        CHECK(d.onde.x >= 0.0f);
        CHECK(d.onde.x <= 1.0f);
        CHECK(d.fonte >= 0);
        CHECK(d.fonte < static_cast<int>(kFontes.size()));
        if (d.tipo == Tipo::Outlier) ++outliers;
        if (d.tipo == Tipo::Nulo) ++nulos;
        if (d.tipo != Tipo::Nulo) onde.push_back(d.onde);
    }
    CHECK(outliers > 0);
    CHECK(nulos > 0);
    // Quem o z-score marca e quase so outlier: os dados comuns ficam perto das especies.
    const auto marcados = Outliers(onde, kLimiteDoOutlier);
    int comunsMarcados = 0, achados = 0;
    size_t i = 0;
    for (const Dado& d : dados) {
        if (d.tipo == Tipo::Nulo) continue;
        if (marcados[i] && d.tipo == Tipo::Comum) ++comunsMarcados;
        if (marcados[i] && d.tipo == Tipo::Outlier) ++achados;
        ++i;
    }
    CHECK(comunsMarcados == 0);
    CHECK(achados >= 1);
    // O Dificil coleta mais, mais depressa, e avisa menos.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreColetas < n.entreColetas);
    CHECK(d.maisDados > n.maisDados);
    CHECK(d.aviso < n.aviso);
}

// ---- O agrupamento ----

TEST_CASE("Agrupamento: a PCA acha a direcao de maior variancia, e a projecao cai na reta") {
    using namespace Agrupamento;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    // Pontos numa diagonal com um pouco de ruido: o eixo e a diagonal.
    std::vector<Vector2> ps;
    for (int k = -5; k <= 5; ++k) ps.emplace_back(10.0f * k + (k % 2 ? 1.0f : -1.0f), 10.0f * k);
    const Eixo e = ComponentePrincipal(ps);
    CHECK(std::fabs(Vector2::Dot(e.direcao, Vector2::Normalize(Vector2(1.0f, 1.0f)))) == doctest::Approx(1.0f).epsilon(0.01));
    const Vector2 p = Projetar(e, Vector2(30.0f, 0.0f));
    // A projecao esta na reta: o que sobra e perpendicular a ela.
    CHECK(std::fabs(Vector2::Dot(Vector2(30.0f, 0.0f) - p, e.direcao)) < 1e-3f);
    // Sem variancia, a horizontal.
    CHECK(Perto(ComponentePrincipal({V(1.0f, 1.0f), V(1.0f, 1.0f)}).direcao, Vector2(1.0f, 0.0f)));
    CHECK(Perto(Girar(Vector2(1.0f, 0.0f), Vector2::Zero, 90.0f), Vector2(0.0f, 1.0f)));
}

TEST_CASE("Agrupamento: as k-medias - o centroide vai para a media, e o aluno puxa o da celula dele") {
    using namespace Agrupamento;
    const std::vector<Vector2> pontos = {V(0, 0), V(2, 0), V(0, 2), V(2, 2), V(10, 10), V(12, 10)};
    const std::vector<Vector2> centroides = {V(1, 1), V(11, 9)};
    const auto grupos = Atribuir(pontos, centroides);
    CHECK(grupos == std::vector<int>{0, 0, 0, 0, 1, 1});
    const auto novos = Atualizar(pontos, grupos, centroides, {}, 0.0f);
    CHECK(Perto(novos[0], Vector2(1.0f, 1.0f)));
    CHECK(Perto(novos[1], Vector2(11.0f, 10.0f)));
    // O aluno no grupo 1, pesando 2: o centroide anda para ele.
    const auto puxados = Atualizar(pontos, grupos, centroides, {Vector2(14.0f, 13.0f)}, 2.0f);
    CHECK(Perto(puxados[1], Vector2((10.0f + 12.0f + 28.0f) / 4.0f, (10.0f + 10.0f + 26.0f) / 4.0f)));
    CHECK(Perto(puxados[0], novos[0]));
    // A inercia cai quando os centroides vao para as medias.
    CHECK(Inercia(pontos, grupos, novos) < Inercia(pontos, grupos, centroides));
    // A celula de Voronoi: a metade de quem esta mais perto, dentro da caixa.
    const auto celula = Celula({V(-5.0f, 0.0f), V(5.0f, 0.0f)}, 0, Vector2(-10.0f, -10.0f), Vector2(10.0f, 10.0f));
    REQUIRE(celula.size() >= 4u);
    for (const Vector2& v : celula) CHECK(v.x <= 1e-3f);
    // Um grupo sem ninguem fica onde esta.
    CHECK(Perto(Atualizar({V(0, 0)}, {0}, {V(0, 0), V(50, 50)}, {}, 0.0f)[1], Vector2(50.0f, 50.0f)));
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).entreIteracoes < DegrauDo(Dificuldade::Nivel::Normal).entreIteracoes);
}

// ---- A classificacao ----

TEST_CASE("Classificacao: o perceptron aprende a reta com os erros, e o comite vota") {
    using namespace Classificacao;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    const Reta vertical = RetaPeloAngulo(0.0f, 0.2f);   // x >= 0,2 e a classe 1
    CHECK(Lado(vertical, Vector2(0.5f, 0.0f)) == 1);
    CHECK(Lado(vertical, Vector2(0.0f, 0.9f)) == 0);
    Vector2 de, ate;
    REQUIRE(Segmento(vertical, de, ate));
    CHECK(de.x == doctest::Approx(0.2f));
    CHECK(ate.x == doctest::Approx(0.2f));
    CHECK(std::fabs(de.y - ate.y) == doctest::Approx(2.0f));
    CHECK_FALSE(Segmento(RetaPeloAngulo(0.0f, 3.0f), de, ate));

    // Treinando do zero, os erros caem: a reta chega perto da verdade.
    Variacao::Sorteio s(7u);
    const Reta verdade = RetaPeloAngulo(35.0f, 0.1f);
    Reta modelo = RetaPeloAngulo(200.0f, 0.0f);
    int primeiros = 0, ultimos = 0;
    for (int epoca = 0; epoca < 30; ++epoca) {
        const Epoca e = Treinar(modelo, verdade, s, kAmostrasPorEpoca, kTaxa);
        if (epoca < 3) primeiros += e.erros;
        if (epoca >= 27) ultimos += e.erros;
        modelo = e.modelo;
        CHECK(modelo.a * modelo.a + modelo.b * modelo.b == doctest::Approx(1.0f));
    }
    CHECK(ultimos < primeiros);
    int concordam = 0;
    for (int k = 0; k < 200; ++k) {
        const Vector2 p(s.Real(-1.0f, 1.0f), s.Real(-1.0f, 1.0f));
        if (Lado(modelo, p) == Lado(verdade, p)) ++concordam;
    }
    CHECK(concordam > 170);

    // O comite: a arvore certinha acerta as quatro quinas, e o voto de tres segue a maioria.
    const Vector2 cortes(0.1f, -0.2f);
    const Arvore certa = Plantar(s, cortes, 0.0f, 0.0f);
    for (const Vector2 q : {Vector2(-0.9f, -0.9f), Vector2(0.9f, -0.9f), Vector2(-0.9f, 0.9f), Vector2(0.9f, 0.9f)}) {
        CHECK(Classe(certa, q) == Verdade(cortes, q));
    }
    Arvore toda1;
    toda1.folhas = {1, 1, 1, 1};
    Arvore toda0;
    toda0.folhas = {0, 0, 0, 0};
    CHECK(Voto({toda1, toda1, toda0}, Vector2(0.3f, 0.3f)) == 1);
    CHECK(Voto({toda1, toda0, toda0}, Vector2(0.3f, 0.3f)) == 0);
    // A rajada troca de classe quase sempre.
    int trocas = 0, c = ClasseDaRajada(s, -1);
    for (int k = 0; k < 100; ++k) {
        const int nova = ClasseDaRajada(s, c);
        if (nova != c) ++trocas;
        c = nova;
    }
    CHECK(trocas > 60);
    CHECK(trocas < 95);
}

// ---- A explicacao ----

TEST_CASE("Explicacao: a pergunta adianta o aviso, e a cascata do SHAP fica no campo") {
    using namespace Explicacao;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(Antecedencia(0, Dificuldade::Nivel::Normal) < Antecedencia(3, Dificuldade::Nivel::Normal));
    CHECK(Antecedencia(100, Dificuldade::Nivel::Normal) == doctest::Approx(kAntecedenciaMaxima));
    CHECK(Antecedencia(2, Dificuldade::Nivel::Dificil) < Antecedencia(2, Dificuldade::Nivel::Normal));
    CHECK(Rumos(Palpite::Anel, 0.0f, 0.0f, 8).size() == 8u);
    CHECK(Rumos(Palpite::Leque, 90.0f, 0.0f, 5)[2] == doctest::Approx(90.0f));
    CHECK(Rumos(Palpite::Espiral, 0.0f, 0.0f, 10).size() == 10u);

    const std::vector<float> c = {40.0f, -25.0f, 10.0f};
    const auto barras = Cascata(300.0f, c);
    REQUIRE(barras.size() == 3u);
    CHECK(barras[0].de == doctest::Approx(300.0f));
    CHECK(barras[1].de == doctest::Approx(barras[0].ate));
    CHECK(barras[2].ate == doctest::Approx(325.0f));
    Variacao::Sorteio s(11u);
    for (int rodada = 0; rodada < 50; ++rodada) {
        const auto contrib = Contribuicoes(s, static_cast<int>(kAtributos.size()), 40.0f, 140.0f, 220.0f);
        REQUIRE(contrib.size() == kAtributos.size());
        float soma = 0.0f;
        for (const float v : contrib) {
            soma += v;
            CHECK(std::fabs(soma) <= 220.0f + 1e-3f);
        }
    }
    CHECK(Legenda({12.0f, -8.0f}, 1.0f) == "ataque +12  \xc2\xb7  defesa -8");
}

// ---- O sorteio ----

TEST_CASE("Chamada: tres perguntas por aula, a certa e a primeira, e a roleta para em alguem") {
    using namespace Chamada;
    std::set<std::string> caras;
    for (int a = 0; a < kAulas; ++a) {
        for (int q = 0; q < kPerguntasPorAula; ++q) {
            const Pergunta& p = Revisao(a)[static_cast<size_t>(q)];
            CHECK(std::string(p.enunciado).size() > 5u);
            for (int o = 0; o < kOpcoes; ++o) {
                CHECK(std::string(p.opcoes[static_cast<size_t>(o)]).size() > 1u);
                caras.insert(Cara(a, q, o));
            }
        }
    }
    CHECK(caras.size() == static_cast<size_t>(kAulas * kPerguntasPorAula * kOpcoes));
    CHECK(Cara(0, 1, 2) == "R0Q1O2");
    // Sozinho: voce ou um colega; em dupla, os dois e um colega.
    Variacao::Sorteio s(3u);
    std::set<int> sozinho, dupla;
    for (int k = 0; k < 60; ++k) {
        sozinho.insert(Sortear(s, 1));
        dupla.insert(Sortear(s, 2));
    }
    CHECK(sozinho == std::set<int>{-1, 0});
    CHECK(dupla == std::set<int>{-1, 0, 1});
    CHECK(Colega(0) != Colega(1));
    CHECK(Colega(kColegas) == Colega(0));
    // A roleta desacelera: troca mais no comeco que no fim.
    int noComeco = 0, noFim = 0;
    for (float t = 0.0f; t < kRoleta; t += 0.01f) {
        const bool trocou = NomeNaRoleta(t, 50) != NomeNaRoleta(t + 0.01f, 50);
        if (t < kRoleta / 3.0f) noComeco += trocou ? 1 : 0;
        else if (t > 2.0f * kRoleta / 3.0f) noFim += trocou ? 1 : 0;
    }
    CHECK(noComeco > noFim);
}
