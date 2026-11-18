//
// O Lucas Ferreira (INF 216 - Projeto e Implementacao de Jogos Digitais): a camada pura das quatro provas.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../Source/Attacks/Fase.h"
#include "../Source/Attacks/JogosDigitais.h"
#include "../Source/Regras/Nota.h"
#include "../Source/Regras/Plataforma.h"

namespace {

    /// O campo da batalha: 1200 x 800 menos a faixa da nota.
    const JogosDigitais::Campo kCampo{0.0f, 0.0f, 1200.0f, 746.0f};
    /// O aluno: o desenho de 64 px (o centro para a um desenho da borda) e o colisor de 6,4 px.
    constexpr float kChao = 64.0f;
    constexpr float kPes = 32.0f;
    constexpr float kRaioDoAluno = 6.4f;
    /// O raio do colisor de uma peca de 28 px (colisorDivisor 2.0) e o de um tiro de 20 px (2.6).
    constexpr float kRaioDaPeca = 14.0f;
    constexpr float kRaioDoTiro = 20.0f / 2.6f;

    template <std::size_t N>
    void ConferirEtapas(const std::array<float, N>& d, const float duracao) {
        float soma = 0.0f;
        for (const float x : d) soma += x;
        CHECK(soma == doctest::Approx(duracao));
        CHECK(JogosDigitais::Etapa(d, 0.0f) == 0);
        float inicio = 0.0f;
        for (std::size_t k = 0; k < N; ++k) {
            CHECK(JogosDigitais::Etapa(d, inicio + 0.01f) == static_cast<int>(k));
            CHECK(JogosDigitais::Etapa(d, inicio + d[k] - 0.01f) == static_cast<int>(k));
            CHECK(JogosDigitais::NaEtapa(d, inicio + d[k] / 2.0f) == doctest::Approx(d[k] / 2.0f));
            inicio += d[k];
        }
        CHECK(JogosDigitais::Etapa(d, 999.0f) == static_cast<int>(N) - 1);
    }

    /// O aluno parte do centro de um anel da abertura a 'rapidez' px/s no rumo 'rumo' (graus): a menor
    /// distancia entre ele e um tiro do anel, ate o anel fechar.
    float MenorDistancia(const HudHostil::AnelDaAbertura& a, const float vao, const float rumo, const float rapidez) {
        float menor = 1e9f;
        const float fecha = HudHostil::Fechamento(a);
        const float r = Math::ToRadians(rumo);
        for (float t = 0.0f; t <= fecha; t += 0.002f) {
            const Vector2 aluno = Vector2(std::cos(r), std::sin(r)) * (rapidez * t);
            const float raio = a.raio - a.velocidade * t;
            for (int i = 0; i < a.tiros; ++i) {
                if (HudHostil::NoVao(i, a.tiros, vao, a.vao)) continue;
                const float ang = Math::ToRadians(HudHostil::AnguloNoAnel(i, a.tiros));
                const Vector2 tiro = Vector2(std::cos(ang), std::sin(ang)) * raio;
                menor = std::min(menor, (tiro - aluno).Length());
            }
        }
        return menor;
    }
}

TEST_CASE("JogosDigitais: as fases dele duram o dobro - a das regras, mais -, e as etapas somam a fase") {
    using JogosDigitais::DuracaoDa;
    CHECK(JogosDigitais::kDuracaoDaFase == doctest::Approx(2.0f * Fase::kDuracao));
    CHECK(RegrasTrocadas::kDuracaoDaFase > JogosDigitais::kDuracaoDaFase);
    ConferirEtapas(HudHostil::kDuracoes, DuracaoDa(HudHostil::kFase));
    ConferirEtapas(Gravidade::kDuracoes, DuracaoDa(Gravidade::kFase));
    ConferirEtapas(RegrasTrocadas::kDuracoes, DuracaoDa(RegrasTrocadas::kFase));
    ConferirEtapas(Lancamento::kDuracoes, DuracaoDa(Lancamento::kFase));
    CHECK(DuracaoDa(HudHostil::kFase) == doctest::Approx(JogosDigitais::kDuracaoDaFase));
    CHECK(DuracaoDa(RegrasTrocadas::kFase) == doctest::Approx(RegrasTrocadas::kDuracaoDaFase));
    CHECK(DuracaoDa(Lancamento::kFase) == doctest::Approx(JogosDigitais::kDuracaoDaFase));
    // Quatro provas, quatro fases do jogo.
    std::set<std::string> fases = {HudHostil::kFase, Gravidade::kFase, RegrasTrocadas::kFase, Lancamento::kFase};
    CHECK(fases.size() == 4u);
}

// ---- A interface: a abertura ----

TEST_CASE("HudHostil: a abertura e rapida - tres aneis que fecham antes do seguinte, todos na etapa") {
    using namespace HudHostil;
    for (const Dificuldade::Nivel n : {Dificuldade::Nivel::Normal, Dificuldade::Nivel::Dificil}) {
        const auto aneis = AneisDaAbertura(n);
        for (int k = 0; k < kAneisDaAbertura; ++k) {
            const AnelDaAbertura& a = aneis[static_cast<size_t>(k)];
            CHECK(a.quando >= 0.0f);
            // Fecha em menos de meio segundo: o susto.
            CHECK(Fechamento(a) < 0.55f);
            CHECK(Fechamento(a) > 0.3f);
            CHECK(a.quando + Fechamento(a) < kDuracoes[kEtapaDaAbertura]);
            if (k + 1 < kAneisDaAbertura) CHECK(a.quando + Fechamento(a) < aneis[static_cast<size_t>(k + 1)].quando);
            CHECK(a.vao >= 3);
            CHECK(a.vao * 4 < a.tiros);
        }
    }
    // O Dificil fecha mais rapido, com o vao mais estreito.
    const auto normal = AneisDaAbertura(Dificuldade::Nivel::Normal);
    const auto dificil = AneisDaAbertura(Dificuldade::Nivel::Dificil);
    CHECK(Fechamento(dificil[0]) < Fechamento(normal[0]));
    CHECK(dificil[0].vao < normal[0].vao);
}

TEST_CASE("HudHostil: o vao da abertura fica em cima, tem o tamanho dito e muda de lugar") {
    using namespace HudHostil;
    for (const int tiros : {44, 46, 48}) {
        for (const int largura : {4, 5}) {
            for (float vao = kVaoDe; vao <= kVaoAte; vao += 7.0f) {
                int faltam = 0;
                for (int i = 0; i < tiros; ++i) {
                    if (!NoVao(i, tiros, vao, largura)) continue;
                    ++faltam;
                    // Os que faltam estao em volta do angulo pedido.
                    float d = AnguloNoAnel(i, tiros) - vao;
                    while (d > 180.0f) d -= 360.0f;
                    while (d < -180.0f) d += 360.0f;
                    CHECK(std::fabs(d) <= 360.0f / static_cast<float>(tiros) * (static_cast<float>(largura) / 2.0f + 0.6f));
                }
                CHECK(faltam == largura);
            }
        }
    }
    Variacao::Sorteio s(5u);
    float anterior = -999.0f;
    for (int k = 0; k < 60; ++k) {
        const float v = VaoDaAbertura(s, anterior);
        CHECK(v >= kVaoDe);
        CHECK(v <= kVaoAte);
        if (anterior >= -180.0f) CHECK(std::fabs(v - anterior) >= 50.0f);
        anterior = v;
    }
    // Perto de uma parede, o vao aponta para onde ha campo: para a direita, para baixo.
    for (const float rumo : {0.0f, 90.0f, 180.0f}) {
        for (int k = 0; k < 30; ++k) {
            const float v = VaoDaAbertura(s, -999.0f, rumo);
            CHECK(v >= rumo - kAberturaDoVao);
            CHECK(v <= rumo + kAberturaDoVao);
        }
    }
}

TEST_CASE("HudHostil: da para escapar da abertura correndo para o vao - e so assim") {
    using namespace HudHostil;
    const float pega = kRaioDoTiro + kRaioDoAluno;
    for (const Dificuldade::Nivel n : {Dificuldade::Nivel::Normal, Dificuldade::Nivel::Dificil}) {
        for (const AnelDaAbertura& a : AneisDaAbertura(n)) {
            for (const float vao : {-150.0f, -120.0f, -90.0f, -61.0f, -30.0f}) {
                CAPTURE(vao);
                // Correndo direto para o vao, na velocidade de andar, passa.
                CHECK(MenorDistancia(a, vao, vao, 300.0f) > pega);
                // Parado, ou correndo para baixo, nao.
                CHECK(MenorDistancia(a, vao, vao, 0.0f) < pega);
                CHECK(MenorDistancia(a, vao, 90.0f, 300.0f) < pega);
                // Nem correndo para o lado errado de cima.
                CHECK(MenorDistancia(a, vao, vao + 180.0f, 300.0f) < pega);
            }
        }
    }
}

// ---- A interface: a barra de verdade ----

TEST_CASE("HudHostil: a barra enche ate a nota, e com 100 nao ha por onde passar") {
    using namespace HudHostil;
    CHECK(Cheios(0.0f, 40) == 0);
    CHECK(Cheios(50.0f, 40) == 20);
    CHECK(Cheios(100.0f, 40) == 40);            // impossivel escapar
    CHECK(Cheios(99.99f, 40) == 39);            // o teto de quem ja foi acertado deixa um segmento
    CHECK(Cheios(250.0f, 40) == 40);
    CHECK(Cheios(-3.0f, 40) == 0);
    CHECK(Cheios(50.0f, 0) == 0);
    for (int n = 0; n < 100; n += 7) CHECK(Cheios(static_cast<float>(n), 40) <= Cheios(static_cast<float>(n + 7), 40));
    int normal = 0, espelhada = 0;
    for (int k = 0; k < 40; ++k) {
        normal += Cheio(k, 40, 30.0f, false) ? 1 : 0;
        espelhada += Cheio(k, 40, 30.0f, true) ? 1 : 0;
    }
    CHECK(normal == 12);
    CHECK(espelhada == 12);
    CHECK(Cheio(0, 40, 30.0f, false));
    CHECK_FALSE(Cheio(39, 40, 30.0f, false));
    CHECK(Cheio(39, 40, 30.0f, true));
    CHECK_FALSE(Cheio(0, 40, 30.0f, true));
    CHECK_FALSE(Cheio(-1, 40, 100.0f, false));
    CHECK_FALSE(Cheio(40, 40, 100.0f, false));
}

TEST_CASE("HudHostil: a varredura vai e volta, e a barra espelha antes de cada passada") {
    using namespace HudHostil;
    const float ida = 2.5f;
    CHECK(NaVarredura(-0.5f, ida).parada);
    CHECK(NaVarredura(-0.5f, ida).onde == doctest::Approx(0.0f));
    CHECK_FALSE(NaVarredura(-0.5f, ida).espelhada);
    const Varredura meio = NaVarredura(ida / 2.0f, ida);
    CHECK(meio.onde == doctest::Approx(0.5f));
    CHECK(meio.passada == 0);
    CHECK_FALSE(meio.parada);
    CHECK_FALSE(meio.espelhada);
    const Varredura descanso = NaVarredura(ida + kPausaDaBarra / 2.0f, ida);
    CHECK(descanso.parada);
    CHECK(descanso.onde == doctest::Approx(1.0f));
    CHECK(descanso.espelhada);
    const Varredura volta = NaVarredura(ida + kPausaDaBarra + ida / 4.0f, ida);
    CHECK(volta.passada == 1);
    CHECK(volta.onde == doctest::Approx(0.75f));
    CHECK(volta.espelhada);
    // As passadas enchem a etapa, e o Dificil passa mais vezes, mais rapido.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.passadasDeitada > n.passadasDeitada);
    CHECK(d.passadasDePe > n.passadasDePe);
    CHECK(IdaDeitada(d) < IdaDeitada(n));
    CHECK(IdaDePe(d) < IdaDePe(n));
    CHECK(IdaDePe(d) > 1.0f);
}

TEST_CASE("HudHostil: a barra sai do rodape, varre o campo deitada, gira, varre de pe e volta - sem saltos") {
    using namespace HudHostil;
    const Vector2 encaixe(600.0f, 760.0f);
    const float noEncaixe = 1120.0f;
    for (const Dificuldade::Nivel nivel : {Dificuldade::Nivel::Normal, Dificuldade::Nivel::Dificil}) {
        const Degrau g = DegrauDo(nivel);
        // No comeco ela ainda esta no rodape, tremendo, e nao machuca.
        const PoseDaBarra comeco = Pose(kEtapaDaBarraDeitada, 0.0f, kCampo, encaixe, noEncaixe, g);
        CHECK(comeco.noEncaixe);
        CHECK_FALSE(comeco.perigo);
        CHECK((comeco.centro - encaixe).Length() < 1.0f);
        CHECK(comeco.comprimento == doctest::Approx(noEncaixe));
        // Arrancada ate o alto, do tamanho do campo, ainda sem machucar.
        const PoseDaBarra alto = Pose(kEtapaDaBarraDeitada, kTremorDaBarra + kArrancadaDaBarra, kCampo, encaixe, noEncaixe, g);
        CHECK(alto.centro.y == doctest::Approx(kCampo.y + kRecuoDaBarra));
        CHECK(alto.comprimento == doctest::Approx(kCampo.w));

        Vector2 antes = comeco.centro;
        float menorY = 1e9f, maiorY = -1e9f, menorX = 1e9f, maiorX = -1e9f;
        int espelhos = 0;
        bool espelhoAntes = false;
        for (const int etapa : {kEtapaDaBarraDeitada, kEtapaDaBarraDePe}) {
            for (float te = 0.0f; te <= kDuracoes[static_cast<size_t>(etapa)]; te += 0.016f) {
                const PoseDaBarra p = Pose(etapa, te, kCampo, encaixe, noEncaixe, g);
                CAPTURE(etapa);
                CAPTURE(te);
                // Nada de salto: nem dentro da etapa nem na troca de uma para a outra.
                CHECK((p.centro - antes).Length() < 40.0f);
                antes = p.centro;
                if (p.espelhada != espelhoAntes) ++espelhos;
                espelhoAntes = p.espelhada;
                if (!p.perigo) continue;
                // Machucando, ela esta inteira no campo, no tamanho dele.
                if (etapa == kEtapaDaBarraDeitada) {
                    CHECK(p.angulo == doctest::Approx(0.0f));
                    CHECK(p.comprimento == doctest::Approx(kCampo.w));
                    menorY = std::min(menorY, p.centro.y);
                    maiorY = std::max(maiorY, p.centro.y);
                } else {
                    CHECK(p.angulo == doctest::Approx(-90.0f));
                    CHECK(p.comprimento == doctest::Approx(kCampo.h));
                    menorX = std::min(menorX, p.centro.x);
                    maiorX = std::max(maiorX, p.centro.x);
                }
            }
        }
        // Deitada, ela passa do alto ao pe do campo; de pe, de um lado ao outro - onde o aluno nao
        // chega, nas pontas.
        CHECK(menorY < kCampo.y + kChao - kRaioDaPeca - kRaioDoAluno);
        CHECK(maiorY > kCampo.y + kCampo.h - kChao + kRaioDaPeca + kRaioDoAluno);
        CHECK(menorX < kCampo.x + kChao - kRaioDaPeca - kRaioDoAluno);
        CHECK(maiorX > kCampo.x + kCampo.w - kChao + kRaioDaPeca + kRaioDoAluno);
        // Espelha a cada passada.
        CHECK(espelhos >= g.passadasDeitada + g.passadasDePe - 2);
        // E volta ao rodape no fim.
        const PoseDaBarra fim = Pose(kEtapaDaBarraDePe, kDuracoes[kEtapaDaBarraDePe], kCampo, encaixe, noEncaixe, g);
        CHECK(fim.noEncaixe);
        CHECK_FALSE(fim.perigo);
        CHECK((fim.centro - encaixe).Length() < 1.0f);
        CHECK(fim.comprimento == doctest::Approx(noEncaixe));
        CHECK(std::fabs(std::fmod(std::fabs(fim.angulo), 180.0f)) < 0.01f);
    }
}

TEST_CASE("HudHostil: os segmentos cobrem a barra de ponta a ponta, e de pe ela enche de baixo") {
    using namespace HudHostil;
    const Degrau g = DegrauDo(Dificuldade::Nivel::Normal);
    const Vector2 encaixe(600.0f, 760.0f);
    const PoseDaBarra dePe = Pose(kEtapaDaBarraDePe, kGiroDaBarra + kRespiroDaBarra + 0.5f, kCampo, encaixe, 1120.0f, g);
    REQUIRE(dePe.perigo);
    const int n = static_cast<int>(dePe.comprimento / kSegmento);
    const Vector2 primeiro = CentroDoSegmento(dePe, 0, n);
    const Vector2 ultimo = CentroDoSegmento(dePe, n - 1, n);
    CHECK(primeiro.y > ultimo.y);                 // o 0 e o de baixo
    CHECK(std::fabs(primeiro.x - ultimo.x) < 0.5f);
    CHECK((ultimo - primeiro).Length() == doctest::Approx(static_cast<float>(n - 1) * kSegmento).epsilon(1e-3));
    CHECK(primeiro.y < kCampo.y + kCampo.h);
    CHECK(ultimo.y > kCampo.y);
    const PoseDaBarra deitada = Pose(kEtapaDaBarraDeitada, 5.0f, kCampo, encaixe, 1120.0f, g);
    const int m = static_cast<int>(deitada.comprimento / kSegmento);
    CHECK(CentroDoSegmento(deitada, 0, m).x < CentroDoSegmento(deitada, m - 1, m).x);
}

// ---- A interface: o inimigo e a IA dele ----

TEST_CASE("IaDoInimigo: o corpo e um fantasma com dois olhos, simetrico e do tamanho dito") {
    using namespace IaDoInimigo;
    const auto blocos = LugaresDosBlocos();
    const auto olhos = LugaresDosOlhos();
    CHECK(blocos.size() == 38u);
    REQUIRE(olhos.size() == 2u);
    // Os olhos na mesma altura, um de cada lado do meio, na metade de cima.
    CHECK(olhos[0].y == doctest::Approx(olhos[1].y));
    CHECK(olhos[0].x == doctest::Approx(-olhos[1].x));
    CHECK(olhos[0].y < 0.0f);
    // Cada bloco dentro do corpo, sem dois no mesmo lugar, e o corpo espelhado.
    std::set<std::pair<int, int>> lugares;
    for (const Vector2& b : blocos) {
        CHECK(std::fabs(b.x) <= kMeiaLargura - kBloco / 2.0f + 0.01f);
        CHECK(std::fabs(b.y) <= kMeiaAltura - kBloco / 2.0f + 0.01f);
        lugares.insert({static_cast<int>(std::lround(b.x)), static_cast<int>(std::lround(b.y))});
    }
    CHECK(lugares.size() == blocos.size());
    for (const Vector2& b : blocos) CHECK(lugares.count({static_cast<int>(std::lround(-b.x)), static_cast<int>(std::lround(b.y))}) == 1);
    // Gigante: bem maior que o aluno (o desenho dele tem 64 px).
    CHECK(2.0f * kMeiaLargura > 2.5f * 64.0f);
    // A cupula: a linha de cima e mais estreita que a do meio.
    float larguraDeCima = 0.0f, larguraDoMeio = 0.0f;
    for (const Vector2& b : blocos) {
        if (b.y < -kMeiaAltura + kBloco) larguraDeCima = std::max(larguraDeCima, std::fabs(b.x));
        if (std::fabs(b.y) < 1.0f) larguraDoMeio = std::max(larguraDoMeio, std::fabs(b.x));
    }
    CHECK(larguraDeCima < larguraDoMeio);
    // A barra: quatro pes, nas colunas pares, que ondulam - cada um ao contrario do vizinho.
    std::vector<int> pes;
    for (const Vector2& b : blocos) {
        if (NaBarra(b)) pes.push_back(Coluna(b));
    }
    std::sort(pes.begin(), pes.end());
    CHECK(pes == std::vector<int>{0, 2, 4, 6});
    for (const float t : {0.13f, 0.7f, 1.9f}) {
        CHECK(std::fabs(Onda(0, t)) <= kOnda + 1e-4f);
        CHECK(Onda(0, t) == doctest::Approx(-Onda(2, t)).epsilon(0.001));
        CHECK(Onda(2, t) == doctest::Approx(-Onda(4, t)).epsilon(0.001));
        CHECK(std::fabs(Flutua(t)) <= kFlutua + 1e-4f);
    }
    CHECK(Coluna(Vector2(0.0f, 0.0f)) == kColunas / 2);
    CHECK_FALSE(NaBarra(Vector2(0.0f, 0.0f)));
}

TEST_CASE("IaDoInimigo: nasce, patrulha, ve o aluno, avisa, persegue cada vez mais rapido e desiste de longe") {
    using namespace IaDoInimigo;
    const Degrau g = DegrauDo(Dificuldade::Nivel::Normal);
    Ia ia;
    Percepcao longe;
    longe.distancia = 900.0f;
    longe.blocos = 35;
    CHECK_FALSE(Machuca(ia.estado));   // nascendo, os blocos ainda se juntam
    CHECK(Rapidez(ia, g) == doctest::Approx(0.0f));
    for (float t = 0.0f; t < kNascendo + 0.05f; t += 0.05f) Passo(ia, longe, 0.05f, g);
    CHECK(ia.estado == Estado::Patrulha);
    CHECK(Machuca(ia.estado));
    CHECK(Rapidez(ia, g) == doctest::Approx(g.ronda));
    // O aluno entra no raio de visao: alerta, parado, e entao a perseguicao.
    Percepcao perto = longe;
    perto.distancia = g.raioDeVisao - 10.0f;
    CHECK(Passo(ia, perto, 0.016f, g));
    CHECK(ia.estado == Estado::Alerta);
    CHECK(Rapidez(ia, g) == doctest::Approx(0.0f));
    for (float t = 0.0f; t < kAlerta + 0.05f; t += 0.05f) Passo(ia, perto, 0.05f, g);
    CHECK(ia.estado == Estado::Persegue);
    const float comeco = Rapidez(ia, g);
    CHECK(comeco == doctest::Approx(g.persegue).epsilon(0.02));
    for (int k = 0; k < 400; ++k) Passo(ia, perto, 0.05f, g);
    CHECK(Rapidez(ia, g) > comeco);
    CHECK(Rapidez(ia, g) == doctest::Approx(g.maxima));
    CHECK(g.maxima < 300.0f);   // o aluno sempre e mais rapido
    // Longe por pouco tempo nao basta; longe por kParaPerder, ele desiste.
    Percepcao fora = longe;
    fora.distancia = g.raioDePerder + 20.0f;
    Passo(ia, fora, kParaPerder / 2.0f, g);
    Passo(ia, perto, 0.05f, g);
    Passo(ia, fora, kParaPerder / 2.0f, g);
    CHECK(ia.estado == Estado::Persegue);
    for (float t = 0.0f; t < kParaPerder + 0.1f; t += 0.05f) Passo(ia, fora, 0.05f, g);
    CHECK(ia.estado == Estado::Patrulha);
    // Quem acerta o aluno recua para a ronda: nao fica em cima dele.
    Ia caca;
    caca.estado = Estado::Persegue;
    Percepcao acertou = perto;
    acertou.acertouOAluno = true;
    CHECK(Passo(caca, acertou, 0.016f, g));
    CHECK(caca.estado == Estado::Patrulha);
    // No recuo, ele nao ve o aluno colado nele; passado o recuo, ve.
    for (float t = 0.0f; t < kRecuo - 0.2f; t += 0.05f) Passo(caca, perto, 0.05f, g);
    CHECK(caca.estado == Estado::Patrulha);
    for (float t = 0.0f; t < 0.4f; t += 0.05f) Passo(caca, perto, 0.05f, g);
    CHECK(caca.estado == Estado::Alerta);
    CHECK(kRecuo < kRondaCansa);
    // Fugindo, encostar no aluno nao e acerto (os blocos sao moeda).
    Ia fugindo;
    fugindo.estado = Estado::Fugindo;
    fugindo.susto = g.susto;
    Passo(fugindo, acertou, 0.016f, g);
    CHECK(fugindo.estado == Estado::Fugindo);
    // A ronda que nao ve ninguem cansa e vira alerta assim mesmo: ele nao fica parado para sempre.
    Ia ronda;
    ronda.estado = Estado::Patrulha;
    float quando = -1.0f;
    for (float t = 0.0f; t < kRondaCansa + 0.5f && quando < 0.0f; t += 0.05f) {
        Passo(ronda, longe, 0.05f, g);
        if (ronda.estado == Estado::Alerta) quando = t + 0.05f;
    }
    CHECK(quando == doctest::Approx(kRondaCansa).epsilon(0.03));
}

TEST_CASE("IaDoInimigo: o tiro no olho atordoa, o power-up faz fugir, e pegar o ultimo bloco o derrota") {
    using namespace IaDoInimigo;
    const Degrau g = DegrauDo(Dificuldade::Nivel::Normal);
    Ia ia;
    ia.estado = Estado::Persegue;
    Percepcao p;
    p.distancia = 200.0f;
    p.blocos = 35;
    // O olho: atordoado e parado, e depois volta a perseguir.
    Percepcao olho = p;
    olho.olhoAtingido = true;
    CHECK(Passo(ia, olho, 0.016f, g));
    CHECK(ia.estado == Estado::Atordoado);
    CHECK(Rapidez(ia, g) == doctest::Approx(0.0f));
    CHECK(Machuca(ia.estado));   // atordoado, o corpo ainda machuca
    for (float t = 0.0f; t < g.atordoado + 0.05f; t += 0.05f) Passo(ia, p, 0.05f, g);
    CHECK(ia.estado == Estado::Persegue);
    // O power-up: ele foge, os blocos nao machucam, e no fim piscam.
    Percepcao poder = p;
    poder.pegouOPoder = true;
    CHECK(Passo(ia, poder, 0.016f, g));
    CHECK(ia.estado == Estado::Fugindo);
    CHECK_FALSE(Machuca(ia.estado));
    CHECK(Rapidez(ia, g) == doctest::Approx(g.foge));
    CHECK_FALSE(Pisca(ia, 0.05f));
    // O olho nao atordoa quem ja foge.
    Passo(ia, olho, 0.016f, g);
    CHECK(ia.estado == Estado::Fugindo);
    // O susto de fora (a inversao das regras) dura o que ela manda, e nao o do power-up.
    {
        Ia invertida;
        invertida.estado = Estado::Persegue;
        Percepcao inversao = p;
        inversao.pegouOPoder = true;
        inversao.susto = 7.5f;
        CHECK(Passo(invertida, inversao, 0.016f, g));
        CHECK(invertida.estado == Estado::Fugindo);
        CHECK(invertida.susto == doctest::Approx(7.5f));
    }
    for (float t = 0.0f; t < g.susto - kPiscaNoFim + 0.1f; t += 0.05f) Passo(ia, p, 0.05f, g);
    CHECK(ia.estado == Estado::Fugindo);
    CHECK((Pisca(ia, 0.05f) || Pisca(ia, 0.15f)));
    CHECK_FALSE((Pisca(ia, 0.05f) && Pisca(ia, 0.15f)));
    // Outro power-up renova o susto.
    Passo(ia, poder, 0.016f, g);
    CHECK(ia.susto == doctest::Approx(g.susto));
    // Sem ninguem pegar todos os blocos, o susto acaba e ele volta a perseguir.
    for (float t = 0.0f; t < g.susto + 0.1f; t += 0.05f) Passo(ia, p, 0.05f, g);
    CHECK(ia.estado == Estado::Persegue);
    // Pego o ultimo bloco: derrotado, e o respawn.
    Passo(ia, poder, 0.016f, g);
    Percepcao vazio = p;
    vazio.blocos = 0;
    CHECK(Passo(ia, vazio, 0.016f, g));
    CHECK(ia.estado == Estado::Derrotado);
    CHECK_FALSE(Machuca(ia.estado));
    // Sem bloco nenhum ele e derrotado em qualquer estado vivo (o Ponto Extra tambem os desfaz).
    Ia outra;
    outra.estado = Estado::Persegue;
    CHECK(Passo(outra, vazio, 0.016f, g));
    CHECK(outra.estado == Estado::Derrotado);
    // Derrotado, o power-up nao o acorda.
    Passo(ia, poder, 0.016f, g);
    CHECK(ia.estado == Estado::Derrotado);
    for (float t = 0.0f; t < kRespawn + 0.05f; t += 0.05f) Passo(ia, p, 0.05f, g);
    CHECK(ia.estado == Estado::Nascendo);
    // Cada estado tem um nome, e os nomes sao diferentes.
    std::set<std::string> nomes;
    for (const Estado e : {Estado::Nascendo, Estado::Patrulha, Estado::Alerta, Estado::Persegue, Estado::Atordoado,
                           Estado::Fugindo, Estado::Derrotado}) nomes.insert(Nome(e));
    CHECK(nomes.size() == 7u);
}

TEST_CASE("IaDoInimigo: ele anda dentro da arena, nasce longe do aluno, e o power-up nasce longe dele") {
    using namespace IaDoInimigo;
    // O centro fica onde o corpo cabe, abaixo da faixa do professor.
    for (const Vector2 p : {Vector2(-500.0f, -500.0f), Vector2(5000.0f, 5000.0f), Vector2(600.0f, 0.0f)}) {
        const Vector2 c = NaArena(p, kCampo);
        CHECK(c.x - kMeiaLargura >= kCampo.x);
        CHECK(c.x + kMeiaLargura <= kCampo.x + kCampo.w);
        CHECK(c.y - kMeiaAltura >= kCampo.y + kTopoDaArena);
        CHECK(c.y + kMeiaAltura <= kCampo.y + kCampo.h);
    }
    const auto ronda = Ronda(kCampo);
    for (const Vector2& k : ronda) CHECK((NaArena(k, kCampo) - k).Length() == doctest::Approx(0.0f));
    // O ninho e o canto mais longe do aluno.
    const Vector2 aluno(150.0f, 700.0f);
    const int n = Ninho(kCampo, aluno);
    for (const Vector2& k : ronda) CHECK((ronda[static_cast<size_t>(n)] - aluno).Length() >= (k - aluno).Length() - 0.01f);
    // O power-up: dentro do campo, abaixo da faixa do professor, e longe do inimigo.
    Variacao::Sorteio s(216u);
    for (int k = 0; k < 200; ++k) {
        const Vector2 inimigo = NaArena(Vector2(s.Real(0.0f, 1200.0f), s.Real(0.0f, 746.0f)), kCampo);
        const Vector2 poder = LugarDoPoder(s, kCampo, inimigo);
        CHECK(poder.x >= kCampo.x + 30.0f);
        CHECK(poder.x <= kCampo.x + kCampo.w - 30.0f);
        CHECK(poder.y >= kCampo.y + kTopoDaArena);
        CHECK(poder.y <= kCampo.y + kCampo.h - 20.0f);
        CHECK((poder - inimigo).Length() >= 300.0f);
    }
    // O Dificil persegue mais rapido, ve mais longe e o power-up dura menos.
    const Degrau normal = DegrauDo(Dificuldade::Nivel::Normal), dificil = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(dificil.persegue > normal.persegue);
    CHECK(dificil.maxima > normal.maxima);
    CHECK(dificil.raioDeVisao > normal.raioDeVisao);
    CHECK(dificil.susto < normal.susto);
    CHECK(dificil.maxima < 300.0f);
    CHECK(dificil.foge < 300.0f);
}

TEST_CASE("JogosDigitais: quem foge vai para longe, escorrega na parede e sai do canto") {
    using JogosDigitais::Fugir;
    const float margem = 30.0f;
    // No meio do campo: para longe, reto.
    const Vector2 longe = Fugir(Vector2(600.0f, 400.0f), Vector2(500.0f, 400.0f), kCampo, margem);
    CHECK(longe.x == doctest::Approx(1.0f));
    CHECK(longe.y == doctest::Approx(0.0f));
    // Na parede de baixo, com o perseguidor em cima e um pouco a esquerda: escorrega para a direita.
    const Vector2 parede = Fugir(Vector2(600.0f, 740.0f), Vector2(590.0f, 500.0f), kCampo, margem);
    CHECK(parede.y <= 0.0f + 1e-4f);
    CHECK(parede.x > 0.9f);
    // No canto de baixo a esquerda, com o perseguidor na diagonal: sai por uma das paredes.
    const Vector2 canto = Fugir(Vector2(10.0f, 740.0f), Vector2(200.0f, 560.0f), kCampo, margem);
    CHECK(canto.Length() == doctest::Approx(1.0f));
    CHECK((canto.x > 0.5f || canto.y < -0.5f));
    // Na parede da esquerda, com o perseguidor bem a direita: anda pela parede (para cima ou para baixo).
    const Vector2 lado = Fugir(Vector2(5.0f, 400.0f), Vector2(300.0f, 400.0f), kCampo, margem);
    CHECK(lado.x == doctest::Approx(0.0f));
    CHECK(std::fabs(lado.y) == doctest::Approx(1.0f));
    // Sempre de tamanho 1, e nunca para fora do campo.
    Variacao::Sorteio s(7u);
    for (int k = 0; k < 300; ++k) {
        const Vector2 eu(s.Real(0.0f, 1200.0f), s.Real(0.0f, 746.0f));
        const Vector2 de(s.Real(0.0f, 1200.0f), s.Real(0.0f, 746.0f));
        const Vector2 d = Fugir(eu, de, kCampo, margem);
        CHECK(d.Length() == doctest::Approx(1.0f).epsilon(0.001));
        if (eu.x <= margem) CHECK(d.x >= -1e-4f);
        if (eu.x >= kCampo.w - margem) CHECK(d.x <= 1e-4f);
        if (eu.y <= margem) CHECK(d.y >= -1e-4f);
        if (eu.y >= kCampo.h - margem) CHECK(d.y <= 1e-4f);
    }
    CHECK(JogosDigitais::Rumo(Vector2(0.0f, 0.0f), Vector2(0.0f, 10.0f)).y == doctest::Approx(1.0f));
    CHECK(JogosDigitais::Rumo(Vector2(5.0f, 5.0f), Vector2(5.0f, 5.0f)).Length() == doctest::Approx(0.0f));
}

TEST_CASE("Gravidade: o primeiro lado e o chao de sempre, os outros tres sao sorteados, e cada um tem o seu desafio") {
    using namespace Gravidade;
    std::set<std::vector<int>> ordens;
    for (std::uint32_t semente = 1; semente <= 30; ++semente) {
        Variacao::Sorteio s(semente);
        const auto ordem = Ordem(s);
        CHECK(ordem[0] == Plataforma::Lado::Baixo);
        std::set<int> lados;
        std::vector<int> como;
        for (const Plataforma::Lado l : ordem) {
            lados.insert(static_cast<int>(l));
            como.push_back(static_cast<int>(l));
        }
        CHECK(lados.size() == 4u);
        ordens.insert(como);
    }
    CHECK(ordens.size() >= 3u);
    std::set<int> desafios;
    for (const Plataforma::Lado l : {Plataforma::Lado::Baixo, Plataforma::Lado::Esquerda, Plataforma::Lado::Cima,
                                     Plataforma::Lado::Direita}) {
        desafios.insert(static_cast<int>(DesafioDo(l)));
        CHECK(Oposto(Oposto(l)) == l);
        CHECK(Oposto(l) != l);
    }
    CHECK(desafios.size() == 4u);
    CHECK(DesafioDo(Plataforma::Lado::Baixo) == Desafio::Pulos);
}

TEST_CASE("Gravidade: (u, h) e o campo visto do chao - ida e volta, nos quatro lados") {
    using namespace Gravidade;
    for (const Plataforma::Lado l : {Plataforma::Lado::Baixo, Plataforma::Lado::Esquerda, Plataforma::Lado::Cima,
                                     Plataforma::Lado::Direita}) {
        CAPTURE(static_cast<int>(l));
        const float comprimento = Comprimento(l, kCampo);
        const float fundo = Profundidade(l, kCampo, kChao);
        CHECK(fundo > 400.0f);
        for (float u = 0.0f; u <= comprimento; u += 97.0f) {
            for (float h = -30.0f; h <= fundo; h += 83.0f) {
                const Vector2 p = Ponto(l, kCampo, kChao, u, h);
                const Vector2 volta = NoChao(l, kCampo, kChao, p);
                CHECK(volta.x == doctest::Approx(u).epsilon(1e-4));
                CHECK(volta.y == doctest::Approx(h).epsilon(1e-4));
                // Dentro do campo.
                CHECK(p.x >= kCampo.x - 0.5f);
                CHECK(p.x <= kCampo.x + kCampo.w + 0.5f);
                CHECK(p.y >= kCampo.y - 0.5f);
                CHECK(p.y <= kCampo.y + kCampo.h + 0.5f);
            }
        }
        // u anda como o chao (Plataforma::AoLongo), e h vai contra a gravidade.
        const Vector2 o = Ponto(l, kCampo, kChao, 100.0f, 0.0f);
        const Vector2 du = Ponto(l, kCampo, kChao, 110.0f, 0.0f) - o;
        const Vector2 dh = Ponto(l, kCampo, kChao, 100.0f, 10.0f) - o;
        CHECK(Vector2::Dot(du, Plataforma::AoLongo(l)) == doctest::Approx(10.0f));
        CHECK(Vector2::Dot(dh, Plataforma::Direcao(l)) == doctest::Approx(-10.0f));
        // h = 0 e onde o centro do aluno para (Player::PrenderNoCampo: um desenho da borda).
        const Vector2 g = Plataforma::Direcao(l);
        const float borda = Vector2::Dot(g, Vector2(g.x > 0.0f ? kCampo.x + kCampo.w : kCampo.x, g.y > 0.0f ? kCampo.y + kCampo.h : kCampo.y));
        CHECK(borda - Vector2::Dot(g, o) == doctest::Approx(kChao));
    }
}

TEST_CASE("Gravidade: o que pular e o que nao - as pilhas, as lancas e o pulo do aluno") {
    using namespace Gravidade;
    // A peca de baixo de uma pilha se apoia nos pes do aluno.
    CHECK(AlturaDaPeca(0, kPes) - kPeca / 2.0f == doctest::Approx(-kPes));
    const auto topo = [](const int altura) { return AlturaDaPeca(altura - 1, kPes); };
    const auto pilhaPega = [](const float h, const int altura) {
        for (int j = 0; j < altura; ++j) {
            if (Pega(h, AlturaDaPeca(j, kPes), kRaioDaPeca, kRaioDoAluno)) return true;
        }
        return false;
    };
    // Parado, ele e pego por qualquer pilha; o pulo segurado passa a mais alta, e sobra.
    for (const int altura : {2, 3, 5}) {
        CHECK(pilhaPega(0.0f, altura));
        CHECK_FALSE(pilhaPega(topo(altura) + kRaioDaPeca + kRaioDoAluno + 1.0f, altura));
        CHECK(Plataforma::AlturaSegurando() > topo(altura) + kRaioDaPeca + kRaioDoAluno + 60.0f);
    }
    // A pilha de dois se passa com um pulinho; a de cinco, nao.
    CHECK_FALSE(pilhaPega(45.0f, 2));
    CHECK(pilhaPega(45.0f, 5));
    // A lanca de baixo pega quem fica; a de cima, quem pula ate ela.
    CHECK(Pega(0.0f, kLancaBaixa, kRaioDaPeca, kRaioDoAluno));
    CHECK_FALSE(Pega(40.0f, kLancaBaixa, kRaioDaPeca, kRaioDoAluno));
    CHECK_FALSE(Pega(0.0f, kLancaAlta, kRaioDaPeca, kRaioDoAluno));
    CHECK(Pega(kLancaAlta, kLancaAlta, kRaioDaPeca, kRaioDoAluno));
    // Um espinho deitado no chao pega quem esta de pe nele.
    CHECK(Pega(0.0f, AlturaDaPeca(0, kPes), kRaioDaPeca, kRaioDoAluno));
}

TEST_CASE("Gravidade: os desafios caem dentro do lado, em ordem, e o Dificil e mais denso") {
    using namespace Gravidade;
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    size_t pilhasN = 0, pilhasD = 0, lancasN = 0, lancasD = 0;
    for (std::uint32_t semente = 1; semente <= 20; ++semente) {
        Variacao::Sorteio s(semente), t(semente);
        const auto pn = Pulos(s, n), pd = Pulos(t, d);
        pilhasN += pn.size();
        pilhasD += pd.size();
        REQUIRE_FALSE(pn.empty());
        for (size_t k = 0; k < pn.size(); ++k) {
            CHECK(pn[k].quando >= kComecaODesafio);
            CHECK(pn[k].quando < kAcabaODesafio);
            CHECK((pn[k].altura == 2 || pn[k].altura == 3 || pn[k].altura == 5));
            CHECK((pn[k].largura == 1 || pn[k].largura == 2));
            CHECK(std::fabs(pn[k].sentido) == doctest::Approx(1.0f));
            if (k == 0) continue;
            const float vao = pn[k].quando - pn[k - 1].quando;
            CHECK(vao >= n.entrePilhasDe - 1e-4f);
            if (pn[k - 1].altura >= 5 || pn[k - 1].largura >= 2) CHECK(vao >= 1.15f - 1e-4f);
        }
        Variacao::Sorteio u(semente), w(semente);
        const auto ln = Lancas(u, n), ld = Lancas(w, d);
        lancasN += ln.size();
        lancasD += ld.size();
        REQUIRE_FALSE(ln.empty());
        for (size_t k = 0; k < ln.size(); ++k) {
            CHECK(ln[k].quando - kAvisoDaLanca >= kComecaODesafio - 1e-4f);
            CHECK(ln[k].quando < kAcabaODesafio + 0.33f);
            CHECK((ln[k].h == kLancaBaixa || ln[k].h == kLancaAlta));
            if (k > 0) CHECK(ln[k].quando > ln[k - 1].quando);
        }
        Variacao::Sorteio c(semente);
        const float comprimento = Comprimento(Plataforma::Lado::Cima, kCampo);
        const auto chuva = Chuva(c, n, comprimento);
        REQUIRE_FALSE(chuva.empty());
        bool moeda = false, bloco = false;
        for (size_t k = 0; k < chuva.size(); ++k) {
            CHECK(chuva[k].u >= 110.0f);
            CHECK(chuva[k].u <= comprimento - 110.0f);
            CHECK(chuva[k].quando < kAcabaODesafio - 0.8f);
            (chuva[k].moeda ? moeda : bloco) = true;
        }
        CHECK(bloco);
        (void)moeda;
        Variacao::Sorteio b(semente);
        for (const Buraco& x : Buracos(b, n)) {
            CHECK(x.espinhos >= 2);
            CHECK(x.espinhos <= 4);
            CHECK(x.quando >= kComecaODesafio);
        }
    }
    CHECK(pilhasD > pilhasN);
    CHECK(lancasD > lancasN);
    CHECK(d.esteira > n.esteira);
    // Contra a esteira o aluno anda, com folga.
    CHECK(d.esteira < 300.0f * 0.7f);
    CHECK(kVirada < kComecaODesafio);
    CHECK(kAcabaODesafio < kDuracoes[0]);
}

// ---- As regras ----

TEST_CASE("RegrasTrocadas: a inversao avisa, vale, avisa de novo e volta") {
    using namespace RegrasTrocadas;
    CHECK(RegraEm(0.0f) == Regra::Aviso);
    CHECK(RegraEm(kComecaAInversao + 0.1f) == Regra::Invertida);
    CHECK(RegraEm(kVoltaEm + 0.1f) == Regra::Voltando);
    CHECK(RegraEm(kVoltou + 0.1f) == Regra::Normal);
    CHECK(kVoltou < kDuracoes[0]);          // sobra etapa com as regras de sempre
    for (const Regra r : {Regra::Aviso, Regra::Invertida, Regra::Voltando, Regra::Normal}) {
        CHECK(TiroEMoeda(r) == AcertoCobra(r));
    }
    CHECK(TiroEMoeda(Regra::Invertida));
    CHECK(TiroEMoeda(Regra::Voltando));
    CHECK_FALSE(TiroEMoeda(Regra::Aviso));
    CHECK_FALSE(TiroEMoeda(Regra::Normal));
    CHECK_FALSE(MoedaPisca(Regra::Invertida, 5.0f));
    bool acesa = false, apagada = false;
    for (float t = kVoltaEm; t < kVoltou; t += 0.03f) (MoedaPisca(Regra::Voltando, t) ? acesa : apagada) = true;
    CHECK(acesa);
    CHECK(apagada);
    CHECK(kMultaPorAcerto > 2.0f * Nota::kGanhoPorAcerto);
    CHECK(kPremioDaMoeda > 0.0f);
}

TEST_CASE("RegrasTrocadas: no fogo amigo o campo da a volta") {
    using namespace RegrasTrocadas;
    CHECK(SaiuPorCima(5.0f, 0.0f));
    CHECK(SaiuPorCima(-12.0f, 0.0f));
    CHECK_FALSE(SaiuPorCima(kFaixaDeSaida + 1.0f, 0.0f));
    CHECK(kFaixaDeSaida > 1600.0f * 0.016f);
    CHECK(SaiuPorBaixo(760.0f, 746.0f));
    CHECK_FALSE(SaiuPorBaixo(740.0f, 746.0f));
    const Vector2 volta = OndeVolta(321.0f, 746.0f);
    CHECK(volta.x == doctest::Approx(321.0f));
    CHECK(volta.y > 746.0f);
    CHECK(kVelocidadeDoRetorno > 0.0f);
}

TEST_CASE("RegrasTrocadas: no FPS o foco derruba o jogo para 30, e o que foi feito sem deltaTime anda com ele") {
    using namespace RegrasTrocadas;
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    // Ninguem focado, o teto; todos, 30; metade da dupla, a media geometrica.
    CHECK(FpsAlvo(0.0f, n.fpsMaximo) == doctest::Approx(n.fpsMaximo));
    CHECK(FpsAlvo(1.0f, n.fpsMaximo) == doctest::Approx(kFpsMinimo));
    CHECK(FpsAlvo(0.5f, 240.0f) == doctest::Approx(std::sqrt(240.0f * kFpsMinimo)));
    CHECK(n.fpsMaximo > kFpsDeReferencia);
    CHECK(d.fpsMaximo > n.fpsMaximo);
    // O passo: cai em kQuedaDoFps, sobe em kSubidaDoFps, e nunca passa do alvo.
    float fps = n.fpsMaximo, t = 0.0f;
    while (fps > kFpsMinimo + 0.01f && t < 5.0f) {
        fps = PassoDoFps(fps, kFpsMinimo, n.fpsMaximo, 0.004f);
        t += 0.004f;
    }
    CHECK(t == doctest::Approx(kQuedaDoFps).epsilon(0.05));
    CHECK(fps >= kFpsMinimo - 0.01f);
    t = 0.0f;
    while (fps < n.fpsMaximo - 0.01f && t < 5.0f) {
        fps = PassoDoFps(fps, n.fpsMaximo, n.fpsMaximo, 0.004f);
        t += 0.004f;
    }
    CHECK(t == doctest::Approx(kSubidaDoFps).epsilon(0.05));
    CHECK(kQuedaDoFps < kSubidaDoFps);
    CHECK(PassoDoFps(60.0f, 60.0f, n.fpsMaximo, 0.016f) == doctest::Approx(60.0f));
    CHECK(PassoDoFps(900.0f, 900.0f, n.fpsMaximo, 0.016f) <= n.fpsMaximo + 0.01f);
    // Sem deltaTime: a 60 FPS, o que esta escrito; o dobro de quadros, o dobro da velocidade.
    CHECK(SemDeltaTime(kFpsDeReferencia) == doctest::Approx(1.0f));
    CHECK(SemDeltaTime(2.0f * kFpsDeReferencia) == doctest::Approx(2.0f));
    CHECK(SemDeltaTime(kFpsMinimo) == doctest::Approx(0.5f));
    // O grafico: 30 no chao, o teto no alto, 60 no meio do caminho.
    CHECK(NoGrafico(kFpsMinimo, n.fpsMaximo) == doctest::Approx(0.0f));
    CHECK(NoGrafico(n.fpsMaximo, n.fpsMaximo) == doctest::Approx(1.0f));
    CHECK(NoGrafico(kFpsDeReferencia, n.fpsMaximo) > 0.2f);
    CHECK(NoGrafico(kFpsDeReferencia, n.fpsMaximo) < 0.8f);
    // Os sete segmentos: o 8 acende todos, o 1 so os da direita, o espaco nenhum; o S e o 5.
    const auto acesos = [](const char c) {
        int k = 0;
        for (const bool s : Segmentos(c)) k += s ? 1 : 0;
        return k;
    };
    CHECK(acesos('8') == 7);
    CHECK(acesos('1') == 2);
    CHECK(Segmentos('1')[1]);
    CHECK(Segmentos('1')[2]);
    CHECK(acesos(' ') == 0);
    CHECK(acesos('0') == 6);
    CHECK_FALSE(Segmentos('0')[6]);
    CHECK(Segmentos('S') == Segmentos('5'));
    CHECK(acesos('F') == 4);
    CHECK(acesos('P') == 5);
}

TEST_CASE("RegrasTrocadas: a parede azul so deixa passar pelo vao, e o vao muda de lado") {
    using namespace RegrasTrocadas;
    // Entre dois tiros da parede nao cabe o aluno: os colisores quase se encostam.
    CHECK(kEntreTirosDaParede - 2.0f * kRaioDoTiro < 2.0f * kRaioDoAluno);
    // O vao cabe o aluno com folga.
    CHECK(kVaoDaParede > 6.0f * kRaioDoAluno + 2.0f * kRaioDoTiro);
    // O vao novo: longe do anterior o bastante para quem esta focado (150 px/s) nao chegar, e perto o
    // bastante para quem solta o foco (300 px/s) chegar antes da parede.
    const Degrau n0 = DegrauDo(Dificuldade::Nivel::Normal);
    const float longe = 1200.0f / 3.0f, perto = VaoAlcancavel(kAlunoSemFoco, n0.entreParedes, longe);
    CHECK(longe > 150.0f * n0.entreParedes);
    CHECK(perto < 300.0f * n0.entreParedes);
    CHECK(perto > longe);
    Variacao::Sorteio s(5u);
    float antes = -1.0f;
    for (int k = 0; k < 80; ++k) {
        const float vao = VaoDaParede(s, antes, 0.0f, 1200.0f, longe, perto);
        CHECK(vao - kVaoDaParede / 2.0f >= -0.01f);
        CHECK(vao + kVaoDaParede / 2.0f <= 1200.01f);
        if (antes >= 0.0f) {
            CHECK(std::fabs(vao - antes) >= longe - 0.01f);
            CHECK(std::fabs(vao - antes) <= perto + 0.01f);
        }
        antes = vao;
    }
    // O Dificil: o vermelho mais rapido e mais denso, as paredes mais frequentes e mais rapidas.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreEspirais < n.entreEspirais);
    CHECK(d.velocidadeVermelha > n.velocidadeVermelha);
    CHECK(d.entreParedes < n.entreParedes);
    CHECK(d.velocidadeAzul > n.velocidadeAzul);
    // Com o foco, o aluno anda a 150 px/s: a parede azul do Normal desce o campo em mais tempo que
    // ele leva para cruza-lo pela metade - focado, so chega ao vao se ja estava perto.
    CHECK(n.velocidadeAzul > 150.0f);
}

TEST_CASE("RegrasTrocadas: o save foge quando vale mais, persegue quando vale menos, e o toque e o load") {
    using namespace RegrasTrocadas;
    // O humor: a nota do save contra a de agora.
    CHECK(HumorDoSave(70.0f, 60.0f) == Humor::Foge);
    CHECK(HumorDoSave(50.0f, 60.0f) == Humor::Persegue);
    CHECK(HumorDoSave(60.2f, 60.0f) == Humor::Parado);
    CHECK(HumorDoSave(59.8f, 60.0f) == Humor::Parado);
    // Parado, nao anda; quanto maior a diferenca, mais rapido - e sempre mais devagar que o aluno.
    CHECK(RapidezDoSave(60.0f, 60.0f) == doctest::Approx(0.0f));
    CHECK(RapidezDoSave(62.0f, 60.0f) < RapidezDoSave(70.0f, 60.0f));
    CHECK(RapidezDoSave(58.0f, 60.0f) < RapidezDoSave(50.0f, 60.0f));
    CHECK(RapidezDoSave(62.0f, 60.0f) == doctest::Approx(RapidezDoSave(58.0f, 60.0f)));
    CHECK(RapidezDoSave(100.0f, 0.0f) == doctest::Approx(kSaveRapido));
    CHECK(kSaveRapido < 300.0f);
    CHECK(RapidezDoSave(61.0f, 60.0f) >= kSaveDevagar);
    // O toque: o raio do disquete e o do aluno.
    CHECK(Encostou(Vector2(100.0f, 100.0f), Vector2(100.0f + kRaioDoSave + 5.0f, 100.0f), 6.4f));
    CHECK_FALSE(Encostou(Vector2(100.0f, 100.0f), Vector2(100.0f + kRaioDoSave + 8.0f, 100.0f), 6.4f));
    // Quando sai um save: logo no comeco, e de novo um pouco depois de cada load, ate o fim chegar perto.
    CHECK_FALSE(HoraDeSalvar(kPrimeiroSave - 0.01f, -1.0f));
    CHECK(HoraDeSalvar(kPrimeiroSave, -1.0f));
    CHECK_FALSE(HoraDeSalvar(4.0f, 3.5f));
    CHECK(HoraDeSalvar(3.5f + kEntreSaves, 3.5f));
    CHECK_FALSE(HoraDeSalvar(kUltimoSave + 0.01f, -1.0f));
    CHECK(kUltimoSave < kDuracoes[3] - 1.5f);
    CHECK(kPrimeiroSave + kEntreSaves < kUltimoSave);
}

TEST_CASE("RegrasTrocadas: o save que foge e alcancavel - encurralado ele trava, e o tiro o faz engasgar") {
    using namespace RegrasTrocadas;
    const JogosDigitais::Campo area{0.0f, 150.0f, 1200.0f, 580.0f};
    // O canto so prende com o aluno perto.
    const Vector2 canto(10.0f, 720.0f);
    CHECK(Encurralado(canto, canto + Vector2(120.0f, -80.0f), area));
    CHECK_FALSE(Encurralado(canto, canto + Vector2(400.0f, -80.0f), area));
    CHECK_FALSE(Encurralado(Vector2(600.0f, 720.0f), Vector2(640.0f, 680.0f), area));   // so uma parede
    // Encurralado, ele fica parado kEncurralado, e depois escapa pela parede; o canto so volta a
    // prender depois da recarga.
    Fuga f;
    const Vector2 aluno = canto + Vector2(150.0f, -100.0f);
    float parado = 0.0f;
    Vector2 eu = canto;
    for (int k = 0; k < 200; ++k) {
        const Vector2 passo = PassoDoSave(f, Humor::Foge, eu, aluno, area, kSaveRapido, 0, 0.01f);
        if (passo.Length() < 1e-6f) parado += 0.01f;
        else break;
    }
    CHECK(parado == doctest::Approx(kEncurralado).epsilon(0.05));
    CHECK(f.recarga > 0.0f);
    CHECK(PassoDoSave(f, Humor::Foge, eu, aluno, area, kSaveRapido, 0, 0.01f).Length() > 0.0f);
    // Em campo aberto, ele foge de quem chega.
    Fuga aberto;
    const Vector2 longe = PassoDoSave(aberto, Humor::Foge, Vector2(600.0f, 400.0f), Vector2(500.0f, 400.0f), area, 200.0f, 0, 0.1f);
    CHECK(longe.x > 0.0f);
    CHECK(longe.Length() == doctest::Approx(20.0f));
    // O tiro: engasga (parado), e logo depois fica imune por um tempo - anda aos trancos.
    Fuga tiro;
    CHECK(PassoDoSave(tiro, Humor::Foge, Vector2(600.0f, 400.0f), Vector2(600.0f, 600.0f), area, 200.0f, 1, 0.01f).Length() < 1e-6f);
    CHECK(Treme(tiro));
    float andou = 0.0f, trancos = 0.0f;
    for (int k = 0; k < 150; ++k) {
        const Vector2 p = PassoDoSave(tiro, Humor::Foge, Vector2(600.0f, 400.0f), Vector2(600.0f, 600.0f), area, 200.0f, 1, 0.01f);
        if (p.Length() > 0.0f) andou += 0.01f;
        else trancos += 0.01f;
    }
    CHECK(andou > 0.3f);     // atirar sem parar nao o congela
    CHECK(trancos > 0.3f);   // mas o para um bocado
    CHECK(kEngasgo * 4.0f > kEngasgoMaximo);
    // Perseguindo, o tiro tambem o segura: e assim que se foge do save que vale menos.
    Fuga persegue;
    CHECK(PassoDoSave(persegue, Humor::Persegue, Vector2(600.0f, 400.0f), Vector2(600.0f, 600.0f), area, 200.0f, 2, 0.01f).Length() < 1e-6f);
    CHECK(PassoDoSave(persegue, Humor::Persegue, Vector2(600.0f, 400.0f), Vector2(600.0f, 600.0f), area, 200.0f, 0, 0.5f).Length() < 1e-6f);
    Fuga solto;
    CHECK(PassoDoSave(solto, Humor::Persegue, Vector2(600.0f, 400.0f), Vector2(600.0f, 600.0f), area, 200.0f, 0, 0.1f).y > 0.0f);
    // Parado (empate), nao anda; e o teto da fuga fica bem abaixo do aluno.
    Fuga empate;
    CHECK(PassoDoSave(empate, Humor::Parado, Vector2(600.0f, 400.0f), Vector2(600.0f, 600.0f), area, 200.0f, 0, 0.1f).Length() < 1e-6f);
    CHECK(kSaveRapido < 0.75f * 300.0f);
}

// ---- O lancamento ----

TEST_CASE("Lancamento: a hitbox descola do desenho") {
    using namespace Lancamento;
    CHECK(Descolamento(0.0f).Length() == doctest::Approx(0.0f));
    CHECK(Descolamento(kAbreODescolamento / 2.0f).Length() == doctest::Approx(kDescolamento / 2.0f).epsilon(1e-3));
    CHECK(Descolamento(kAbreODescolamento + 1.0f).Length() == doctest::Approx(kDescolamento).epsilon(1e-3));
    CHECK(Descolamento(9.0f).Length() == doctest::Approx(kDescolamento).epsilon(1e-3));
    CHECK((Descolamento(5.0f) - Descolamento(8.0f)).Length() > 10.0f);
    CHECK(kDescolamento > 2.0f * 10.0f);
}

TEST_CASE("Lancamento: a travada espera a escolha, e quem nao escolhe ve o jogo voltar acelerado") {
    using namespace Lancamento;
    CHECK(TravadaEm(0.0f) == -1);
    CHECK(TravadaEm(kTravadas[0] + 0.1f) == 0);
    CHECK(TravadaEm(kTravadas[0] + kEspera + 0.1f) == -1);
    CHECK(TravadaEm(kTravadas[1] + 0.1f) == 1);
    CHECK(kTravadas[1] > kTravadas[0] + kEspera);
    CHECK(kTravadas[1] + kEspera < kDuracoes[1]);
    CHECK(FatorDaVolta(Escolha::Esperar) == doctest::Approx(1.0f));
    CHECK(FatorDaVolta(Escolha::Fechar) == doctest::Approx(1.0f));
    CHECK(FatorDaVolta(Escolha::Nenhuma) > 1.3f);
    CHECK(LadoDoEsperar(0) != LadoDoEsperar(1));
    CHECK(LadoDoEsperar(0) >= 0);
    CHECK(LadoDoEsperar(1) <= 1);
}

TEST_CASE("Lancamento: a frota marcha, bate na borda, desce e volta") {
    using namespace Lancamento;
    const float folga = 260.0f;
    CHECK(MarchaEm(0, folga).dx == doctest::Approx(0.0f));
    CHECK(MarchaEm(3, folga).dx == doctest::Approx(3.0f * kPassoDaFrota));
    CHECK(MarchaEm(3, folga).descidas == 0);
    int descidas = 0;
    float menor = 0.0f, maior = 0.0f;
    for (int p = 0; p <= 80; ++p) {
        const Marcha m = MarchaEm(p, folga);
        CHECK(std::fabs(m.dx) <= folga + 1e-3f);
        CHECK(m.descidas >= descidas);
        descidas = m.descidas;
        menor = std::min(menor, m.dx);
        maior = std::max(maior, m.dx);
    }
    CHECK(descidas >= 3);
    CHECK(maior > folga - kPassoDaFrota);
    CHECK(menor < -(folga - kPassoDaFrota));
    const Vector2 topo(600.0f, 150.0f);
    const Marcha parada;
    CHECK(LugarNaFrota(0, kColunasDaFrota / 2, topo, parada).x == doctest::Approx(600.0f));
    CHECK(LugarNaFrota(0, 0, topo, parada).x == doctest::Approx(600.0f - 4.0f * kEspacoX));
    CHECK(LugarNaFrota(2, 0, topo, parada).y == doctest::Approx(150.0f + 2.0f * kEspacoY));
    const Marcha andou{60.0f, 2};
    CHECK(LugarNaFrota(1, 3, topo, andou).x == doctest::Approx(LugarNaFrota(1, 3, topo, parada).x + 60.0f));
    CHECK(LugarNaFrota(1, 3, topo, andou).y == doctest::Approx(LugarNaFrota(1, 3, topo, parada).y + 2.0f * kDescidaDaFrota));
}

TEST_CASE("Lancamento: cada linha dos creditos tem um vao, fora da faixa do texto, e todas sobem a tempo") {
    using namespace Lancamento;
    Variacao::Sorteio s(9u);
    const int blocos = 42;
    bool esquerda = false, direita = false;
    for (int k = 0; k < 200; ++k) {
        const int v = VaoDaLinha(blocos, s);
        CHECK(v >= 1);
        CHECK(v + kVaoDosCreditos <= blocos - 1);
        const bool aEsquerda = v + kVaoDosCreditos <= blocos / 2 - kFaixaDoTexto;
        const bool aDireita = v >= blocos / 2 + kFaixaDoTexto;
        CHECK((aEsquerda || aDireita));
        (aEsquerda ? esquerda : direita) = true;
        int abertos = 0;
        for (int b = 0; b < blocos; ++b) abertos += NoVao(b, v) ? 1 : 0;
        CHECK(abertos == kVaoDosCreditos);
    }
    CHECK(esquerda);
    CHECK(direita);
    CHECK(static_cast<float>(kVaoDosCreditos) * kBloco > 100.0f);
    std::set<std::string> linhas(kCreditos.begin(), kCreditos.end());
    CHECK(linhas.size() == kCreditos.size());
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).subida > DegrauDo(Dificuldade::Nivel::Normal).subida);
    // As oito linhas cabem na etapa.
    for (const Dificuldade::Nivel nivel : {Dificuldade::Nivel::Normal, Dificuldade::Nivel::Dificil}) {
        CHECK(static_cast<float>(kCreditos.size()) * DegrauDo(nivel).entreLinhas < kDuracoes[3]);
    }
}
