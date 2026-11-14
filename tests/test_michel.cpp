//
// O Michel (INF 429 - Aprendizado de Maquina com Dados Visuais): a camada pura das quatro provas.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "../Source/Attacks/CameraEscura.h"
#include "../Source/Attacks/Convolucao.h"
#include "../Source/Attacks/Fase.h"
#include "../Source/Attacks/Laboratorio.h"
#include "../Source/Attacks/RedeNeural.h"

namespace {
    Vector2 V(const float x, const float y) { return Vector2(x, y); }

    bool Perto(const Vector2 a, const Vector2 b, const float tolerancia = 1e-3f) {
        return (a - b).Length() < tolerancia;
    }
}

// ---- A camera escura ----

TEST_CASE("CameraEscura: a imagem sai invertida do outro lado do furo, e o caminho de volta acha o objeto") {
    using namespace CameraEscura;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(kDuracoes[0] + 0.1f) == 1);
    CHECK(EtapaNoTempo(19.9f) == 2);

    const Vector2 furo = V(300.0f, 400.0f);
    // Em cima e a esquerda do furo vira embaixo e a direita.
    const Vector2 im = Imagem(V(250.0f, 300.0f), furo, 1.0f);
    CHECK(Perto(im, V(350.0f, 500.0f)));
    // O aumento afasta a imagem do furo, na mesma reta.
    CHECK(Perto(Imagem(V(250.0f, 300.0f), furo, 2.0f), V(400.0f, 600.0f)));
    // O que cai na cena (desce) sobe na imagem.
    const Vector2 antes = Imagem(V(280.0f, 200.0f), furo, 1.3f);
    const Vector2 depois = Imagem(V(280.0f, 260.0f), furo, 1.3f);
    CHECK(depois.y < antes.y);
    // O caminho de volta: o objeto cuja imagem e o aluno.
    for (const float a : {0.6f, 1.0f, 1.7f}) {
        const Vector2 aluno = V(123.0f, 640.0f);
        CHECK(Perto(Imagem(ObjetoDa(aluno, furo, a), furo, a), aluno, 1e-2f));
    }
}

TEST_CASE("CameraEscura: o foco oscila, o foco automatico persegue o aluno e o borrao vira anel") {
    using namespace CameraEscura;
    CHECK(AumentoNoTempo(0.0f, 3.0f, 0.7f, 1.5f) == doctest::Approx(0.7f));
    CHECK(AumentoNoTempo(1.5f, 3.0f, 0.7f, 1.5f) == doctest::Approx(1.5f));
    CHECK(AumentoNoTempo(3.0f, 3.0f, 0.7f, 1.5f) == doctest::Approx(0.7f).epsilon(1e-3));

    // Longe do plano de foco o anel cresce, ate o maximo; nele, e um ponto.
    CHECK(RaioDoBorrao(500.0f, 500.0f, 0.3f, 60.0f) == doctest::Approx(0.0f));
    CHECK(RaioDoBorrao(600.0f, 500.0f, 0.3f, 60.0f) == doctest::Approx(30.0f));
    CHECK(RaioDoBorrao(100.0f, 500.0f, 0.3f, 60.0f) == doctest::Approx(60.0f));

    float foco = 0.0f;
    for (int k = 0; k < 60; ++k) foco = Focar(foco, 100.0f, 1.0f / 60.0f, 1.5f);
    CHECK(foco > 60.0f);
    CHECK(foco < 100.0f);

    const auto anel = Borrao(V(10.0f, 10.0f), 20.0f, 6, 0.0f);
    REQUIRE(anel.size() == 6);
    for (const Vector2& p : anel) CHECK((p - V(10.0f, 10.0f)).Length() == doctest::Approx(20.0f));
    for (const Vector2& p : Borrao(V(5.0f, 5.0f), 0.0f, 4, 1.0f)) CHECK(Perto(p, V(5.0f, 5.0f)));
    CHECK(Borrao(V(0.0f, 0.0f), 10.0f, 0, 0.0f).empty());

    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreArremessos < n.entreArremessos);
    CHECK(d.leque >= n.leque);
}

// ---- A convolucao ----

TEST_CASE("Convolucao: as setas do descritor apontam para o aluno, e o giro da imagem as poe de lado") {
    using namespace Convolucao;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(Bin(V(1.0f, 0.0f)) == 0);
    CHECK(Bin(V(0.0f, 1.0f)) == 2);
    CHECK(Bin(V(-1.0f, 0.0f)) == 4);
    CHECK(Bin(V(0.0f, -1.0f)) == 6);
    CHECK(Bin(V(1.0f, 1.0f)) == 1);
    CHECK(Bin(V(1.0f, -0.1f)) == 0);
    for (int b = 0; b < kBins; ++b) CHECK(Bin(DirecaoDoBin(b)) == b);

    // O aluno embaixo da celula: a seta desce. Com o giro de 90 graus, fica de lado.
    const int bin = BinDaCelula(V(100.0f, 100.0f), V(100.0f, 500.0f), 0);
    CHECK(bin == 2);
    const int girado = BinDaCelula(V(100.0f, 100.0f), V(100.0f, 500.0f), kGiroDaImagem);
    CHECK(std::fabs(Vector2::Dot(DirecaoDoBin(girado), V(0.0f, 1.0f))) < 1e-4f);
}

TEST_CASE("Convolucao: os tiros acendem a grade, o filtro responde, o pooling fica com o maximo") {
    using namespace Convolucao;
    Grade g = Vazia(kColunas, kLinhas);
    CHECK(g.v.size() == static_cast<size_t>(kColunas * kLinhas));
    CHECK(Global(g) == doctest::Approx(0.0f));

    Pintar(g, 3, 2, 0.6f, 1.0f);
    Pintar(g, 3, 2, 0.6f, 1.0f);
    CHECK(g.Em(3, 2) == doctest::Approx(1.0f));   // o teto
    Pintar(g, -1, 2, 1.0f, 1.0f);                  // fora da grade: nada
    CHECK(g.Em(-1, 2) == doctest::Approx(0.0f));

    // A resposta no centro e o peso do meio; ao lado, o da borda; longe, zero.
    CHECK(Resposta(g, 3, 2, kGaussiano) == doctest::Approx(4.0f / 16.0f));
    CHECK(Resposta(g, 4, 2, kGaussiano) == doctest::Approx(2.0f / 16.0f));
    CHECK(Resposta(g, 4, 3, kGaussiano) == doctest::Approx(1.0f / 16.0f));
    CHECK(Resposta(g, 6, 5, kGaussiano) == doctest::Approx(0.0f));
    // Uma coluna inteira acesa (o aluno atirando parado) responde mais que uma celula so.
    for (int l = 0; l < kLinhas; ++l) Pintar(g, 3, l, 1.0f, 1.0f);
    CHECK(Resposta(g, 3, 2, kGaussiano) == doctest::Approx(8.0f / 16.0f));
    // O padding de zeros: na borda, a resposta e menor.
    CHECK(Resposta(g, 3, 0, kGaussiano) < Resposta(g, 3, 2, kGaussiano));

    CHECK(Relu(0.3f, 0.12f) == doctest::Approx(0.18f));
    CHECK(Relu(0.05f, 0.12f) == doctest::Approx(0.0f));

    // A varredura: linha por linha, e volta.
    CHECK(Janela(0, kColunas, kLinhas) == std::make_pair(0, 0));
    CHECK(Janela(kColunas - 1, kColunas, kLinhas) == std::make_pair(kColunas - 1, 0));
    CHECK(Janela(kColunas, kColunas, kLinhas) == std::make_pair(0, 1));
    CHECK(Janela(kColunas * kLinhas, kColunas, kLinhas) == std::make_pair(0, 0));

    const Grade p = MaxPool(g);
    CHECK(p.colunas == kColunas / 2);
    CHECK(p.linhas == kLinhas / 2);
    CHECK(p.Em(1, 0) == doctest::Approx(1.0f));   // a coluna 3 cai no bloco 1
    CHECK(p.Em(0, 0) == doctest::Approx(0.0f));
    const Grade impar = MaxPool(Vazia(5, 3));
    CHECK(impar.colunas == 3);
    CHECK(impar.linhas == 2);

    const float antes = Global(g);
    Esmaecer(g, 1.6f, 1.6f);
    CHECK(Global(g) == doctest::Approx(antes * 0.5f));

    CHECK(TirosDaAtivacao(0.0f, 3, 11, 0.4f) == 0);
    CHECK(TirosDaAtivacao(0.01f, 3, 11, 0.4f) == 3);
    CHECK(TirosDaAtivacao(1.0f, 3, 11, 0.4f) == 11);
}

// ---- A rede ----

TEST_CASE("RedeNeural: as camadas, as ligacoes fortes e os atalhos de duas camadas") {
    using namespace RedeNeural;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(CamadasDa(0).size() == 3);
    CHECK(CamadasDa(2).size() == 5);

    Variacao::Sorteio s(7u);
    Rede r = Montar({4, 5, 3}, s, 0.15f, 1.0f);
    REQUIRE(r.pesos.size() == 2);
    CHECK(r.pesos[0].size() == 20);
    CHECK(r.pesos[1].size() == 15);
    for (const auto& camada : r.pesos) {
        for (const float w : camada) {
            CHECK(w >= 0.15f);
            CHECK(w <= 1.0f);
        }
    }
    r.Peso(0, 1, 0) = 0.9f;
    r.Peso(0, 1, 1) = 0.2f;
    const auto ligadas = Ligacoes(r, 0, 1, 0.55f);
    CHECK(std::find(ligadas.begin(), ligadas.end(), 0) != ligadas.end());
    CHECK(std::find(ligadas.begin(), ligadas.end(), 1) == ligadas.end());
    CHECK(Ligacoes(r, 2, 0, 0.0f).empty());   // a saida nao liga em ninguem

    CHECK(Alinhado(0, 4, 3) == 0);
    CHECK(Alinhado(3, 4, 3) == 2);
    CHECK(Alinhado(0, 1, 3) == 1);
    const auto atalhos = Atalhos({4, 4, 4, 4, 3}, 2);
    CHECK(atalhos.size() == 8);   // da camada 0 a 2 e da 2 a 4
    for (const Atalho& a : atalhos) {
        CHECK(a.pulo == 2);
        CHECK((a.camada == 0 || a.camada == 2));
    }
    CHECK(Atalhos({4, 5, 3}, 2).size() == 4);
    CHECK(Atalhos({4, 5}, 2).empty());
}

TEST_CASE("RedeNeural: o neuronio dispara no limiar, e a retropropagacao engrossa o caminho ate o alvo") {
    using namespace RedeNeural;
    float soma = 0.0f;
    CHECK_FALSE(Somar(soma, 0.6f, 1.0f));
    CHECK(Somar(soma, 0.6f, 1.0f));
    CHECK(soma == doctest::Approx(0.2f));

    Variacao::Sorteio s(11u);
    Rede r = Montar({4, 5, 3}, s, 0.15f, 1.0f);
    const auto entrada = [&r](const int j) {
        float soma = 0.0f;
        for (int i = 0; i < r.camadas[1]; ++i) soma += r.Peso(1, i, j);
        return soma;
    };
    const float antesAlvo = entrada(2), antesOutra = entrada(0);
    for (int k = 0; k < 6; ++k) Retropropagar(r, 2, 0.18f, 0.15f, 1.0f);
    CHECK(entrada(2) > antesAlvo);
    CHECK(entrada(0) < antesOutra);
    for (const auto& camada : r.pesos) {
        for (const float w : camada) {
            CHECK(w >= 0.15f);
            CHECK(w <= 1.0f);
        }
    }

    CHECK(SaidaDoAluno({100.0f, 300.0f, 500.0f}, 330.0f) == 1);
    CHECK(SaidaDoAluno({100.0f, 300.0f, 500.0f}, 0.0f) == 0);
    CHECK(SaidaDoAluno({}, 0.0f) == -1);
}

// ---- O laboratorio ----

TEST_CASE("Laboratorio: a mascara do decoder afina, e o atalho traz o passo anterior") {
    using namespace Laboratorio;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    for (size_t k = 1; k < kDecoder.size(); ++k) {
        CHECK(kDecoder[k].n == 2 * kDecoder[k - 1].n);
        CHECK(kDecoder[k].quando > kDecoder[k - 1].quando + kDecoder[k - 1].aviso);
    }
    CHECK(kDecoder.back().quando + kDecoder.back().aviso < kDuracoes[0]);

    CHECK(CelulaDe(V(0.1f, 0.9f), 2) == std::make_pair(0, 1));
    CHECK(CelulaDe(V(1.0f, 1.0f), 4) == std::make_pair(3, 3));
    CHECK(CelulaDe(V(-0.2f, 0.5f), 4) == std::make_pair(0, 2));

    const Mascara m = Segmentar(4, {V(0.6f, 0.6f)}, {}, 0);
    CHECK(m.Quantas() == 1);
    CHECK(m.Em(2, 2));
    const Mascara comAtalho = Segmentar(4, {V(0.6f, 0.6f)}, {V(0.1f, 0.1f)}, 0);
    CHECK(comAtalho.Quantas() == 2);
    CHECK(comAtalho.Em(0, 0));
    // A dilatacao poe as vizinhas, e a borda corta.
    CHECK(Segmentar(8, {V(0.5f, 0.5f)}, {}, 1).Quantas() == 9);
    CHECK(Segmentar(8, {V(0.0f, 0.0f)}, {}, 1).Quantas() == 4);

    const auto ps = Pixels(V(0.0f, 0.0f), V(100.0f, 60.0f), 30.0f);
    CHECK(ps.size() == 6);   // 3 x 2
    for (const Vector2& p : ps) {
        CHECK(p.x > 0.0f);
        CHECK(p.x < 100.0f);
        CHECK(p.y > 0.0f);
        CHECK(p.y < 60.0f);
    }
    CHECK(Pixels(V(0.0f, 0.0f), V(10.0f, 10.0f), 30.0f).size() == 1);
}

TEST_CASE("Laboratorio: a recorrente preve a reta, e a LSTM preve a curva") {
    using namespace Laboratorio;
    Memoria m;
    const float dt = 1.0f / 60.0f;
    for (int k = 0; k <= 120; ++k) Lembrar(m, V(100.0f + 120.0f * dt * static_cast<float>(k), 300.0f), dt, 3.0f);
    CHECK(m.velocidade.x == doctest::Approx(120.0f).epsilon(0.02));
    CHECK(std::fabs(m.giro) < 0.05f);
    CHECK(Perto(Prever(m, 1.0f, false), m.onde + V(120.0f, 0.0f), 3.0f));

    // Em circulo: a reta erra, a curva acerta.
    Memoria c;
    const float w = 1.5f, raio = 120.0f;
    const auto noCirculo = [&](const float t) {
        return V(400.0f + raio * Math::Cos(w * t), 300.0f + raio * Math::Sin(w * t));
    };
    for (int k = 0; k <= 240; ++k) Lembrar(c, noCirculo(dt * static_cast<float>(k)), dt, 3.0f);
    CHECK(c.giro == doctest::Approx(w).epsilon(0.05));
    const Vector2 certo = noCirculo(dt * 240.0f + 0.8f);
    const float erroDaCurva = (Prever(c, 0.8f, true) - certo).Length();
    const float erroDaReta = (Prever(c, 0.8f, false) - certo).Length();
    // A rapidez lembrada fica um pouco abaixo da real na curva: o erro e de uns poucos por cento.
    CHECK(erroDaCurva < 20.0f);
    CHECK(erroDaCurva * 3.0f < erroDaReta);

    // O encontro: o tiro chega onde a previsao estara.
    const Vector2 origem = V(400.0f, 60.0f);
    const float t = Encontro(m, origem, 240.0f, false);
    CHECK((Prever(m, t, false) - origem).Length() == doctest::Approx(240.0f * t).epsilon(0.02));
}

TEST_CASE("Laboratorio: o detector - a confianca, o IoU e o NMS") {
    using namespace Laboratorio;
    const Caixa a = CaixaEm(V(0.0f, 0.0f), 10.0f);
    CHECK(Iou(a, a) == doctest::Approx(1.0f));
    CHECK(Iou(a, CaixaEm(V(5.0f, 0.0f), 10.0f)) == doctest::Approx(50.0f / 150.0f));
    CHECK(Iou(a, CaixaEm(V(50.0f, 0.0f), 10.0f)) == doctest::Approx(0.0f));

    // A mais confiante fica; a que a cobre sai; a longe entra.
    const std::vector<Caixa> caixas = {CaixaEm(V(0.0f, 0.0f), 10.0f), CaixaEm(V(2.0f, 0.0f), 10.0f),
                                       CaixaEm(V(100.0f, 0.0f), 10.0f)};
    const std::vector<float> conf = {0.5f, 0.9f, 0.3f};
    const auto escolhidas = Nms(caixas, conf, 0.4f, 2);
    REQUIRE(escolhidas.size() == 2);
    CHECK(escolhidas[0] == 1);
    CHECK(escolhidas[1] == 2);
    CHECK(Nms(caixas, conf, 0.4f, 1) == std::vector<int>{1});

    // Quieto, a confianca cai para a base; andando e atirando, sobe.
    float quieto = 0.8f, agitado = 0.1f;
    for (int k = 0; k < 180; ++k) {
        quieto = Confianca(quieto, 0.0f, 0.0f, 1.0f / 60.0f);
        agitado = Confianca(agitado, 300.0f, 8.0f, 1.0f / 60.0f);
    }
    CHECK(quieto == doctest::Approx(kConfiancaBase).epsilon(0.05));
    CHECK(agitado > 0.9f);
    CHECK(Confianca(0.3f, 100.0f, 0.0f, 1.0f / 60.0f) < Confianca(0.3f, 100.0f, 4.0f, 1.0f / 60.0f));

    for (int k = 0; k < kColegas; ++k) CHECK_FALSE(Colega(k).empty());

    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.rodadaDoDetector < n.rodadaDoDetector);
    CHECK(d.procura < d.rodadaDoDetector);
    CHECK(n.procura + n.avisoDoCerco < n.rodadaDoDetector);
}
