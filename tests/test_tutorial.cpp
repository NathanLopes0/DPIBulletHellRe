//
// A fase tutorial (INF 191): o ritmo das etapas e os desenhos.
//

#include "doctest.h"

#include "../Source/Attacks/Tutorial.h"

using namespace Tutorial;

TEST_CASE("Tutorial: as etapas vem em ordem, e o professor encerra na festa do fim") {
    CHECK(EtapaEm(0.0f) == Etapa::Chegada);
    CHECK(EtapaEm(ComecoDa(Etapa::Desviar)) == Etapa::Desviar);
    CHECK(EtapaEm(ComecoDa(Etapa::Atirar) - 0.01f) == Etapa::Desviar);
    CHECK(EtapaEm(ComecoDa(Etapa::SobeEDesce) + 1.0f) == Etapa::SobeEDesce);
    CHECK(EtapaEm(ComecoDa(Etapa::Foco) + 1.0f) == Etapa::Foco);
    CHECK(EtapaEm(ComecoDa(Etapa::PontoExtra) + 1.0f) == Etapa::PontoExtra);
    CHECK(EtapaEm(1000.0f) == Etapa::Fim);
    CHECK(NaEtapa(ComecoDa(Etapa::Atirar) + 2.0f) == doctest::Approx(2.0f));
    // Cada etapa tem tempo de ensinar; o tutorial inteiro cabe em pouco mais de um minuto.
    for (const float d : kDuracoes) CHECK(d >= 3.0f);
    CHECK_FALSE(Acabou(ComecoDa(Etapa::Fim)));
    CHECK(Acabou(ComecoDa(Etapa::Fim) + kAplausos));
    CHECK(ComecoDa(Etapa::Fim) + kAplausos <= 75.0f);
    // A fase do arquivo dura mais que isso: quem encerra e o professor, e nao o relogio.
    float total = 0.0f;
    for (const float d : kDuracoes) total += d;
    CHECK(total > ComecoDa(Etapa::Fim) + kAplausos);
}

TEST_CASE("Tutorial: quem ja desvia e acerta passa a etapa antes do tempo") {
    Desempenho d;
    d.naEtapa = 2.0f;
    d.semApanhar = 99.0f;
    d.acertosDados = 99;
    d.gastouPontoExtra = true;
    // Nunca antes do minimo: o desenho da etapa e visto.
    for (const Etapa e : {Etapa::Desviar, Etapa::Atirar, Etapa::SobeEDesce, Etapa::Foco, Etapa::PontoExtra}) {
        CHECK_FALSE(PodeAvancar(e, d));
    }
    d = Desempenho{};
    d.naEtapa = kMinimoNaEtapa;
    // Desviar e o foco: tempo sem apanhar.
    CHECK_FALSE(PodeAvancar(Etapa::Desviar, d));
    d.semApanhar = kSemApanharParaPassar;
    CHECK(PodeAvancar(Etapa::Desviar, d));
    CHECK(PodeAvancar(Etapa::Foco, d));
    // Atirar: os acertos; a nota que sobe e desce: os dois juntos.
    CHECK_FALSE(PodeAvancar(Etapa::Atirar, d));
    CHECK_FALSE(PodeAvancar(Etapa::SobeEDesce, d));
    d.acertosDados = kAcertosParaPassar;
    CHECK(PodeAvancar(Etapa::Atirar, d));
    CHECK(PodeAvancar(Etapa::SobeEDesce, d));
    d.semApanhar = 1.0f;
    CHECK_FALSE(PodeAvancar(Etapa::SobeEDesce, d));
    // O Ponto Extra: gasto.
    CHECK_FALSE(PodeAvancar(Etapa::PontoExtra, d));
    d.gastouPontoExtra = true;
    CHECK(PodeAvancar(Etapa::PontoExtra, d));
    // A chegada e o fim tem o tempo deles.
    CHECK_FALSE(PodeAvancar(Etapa::Chegada, d));
    CHECK_FALSE(PodeAvancar(Etapa::Fim, d));
    // Cada etapa com tiro tem o gatilho e o ritmo dela; as outras, nenhum.
    for (const Etapa e : {Etapa::Desviar, Etapa::SobeEDesce, Etapa::Foco, Etapa::PontoExtra}) {
        CHECK(GatilhoDe(e) != nullptr);
        CHECK(CadenciaDe(e) > 0.0f);
    }
    CHECK(GatilhoDe(Etapa::Atirar) == nullptr);
    CHECK(GatilhoDe(Etapa::Fim) == nullptr);
}

TEST_CASE("Tutorial: a nota que sobe e a que desce, e o sinal que voa ate a barra") {
    CHECK(SinalDaNota(40.0f, 41.5f) == Sinal::Subiu);
    CHECK(SinalDaNota(40.0f, 37.0f) == Sinal::Desceu);
    CHECK(SinalDaNota(40.0f, 40.001f) == Sinal::Nada);
    const Vector2 de(600.0f, 100.0f), ate(300.0f, 760.0f);
    CHECK((NoVoo(de, ate, 0.0f) - de).Length() < 0.01f);
    CHECK((NoVoo(de, ate, kVooDoSinal) - ate).Length() < 0.01f);
    CHECK((NoVoo(de, ate, 99.0f) - ate).Length() < 0.01f);
    // Devagar no fim: na metade do tempo, ja passou da metade do caminho.
    CHECK((NoVoo(de, ate, kVooDoSinal / 2.0f) - de).Length() > (ate - de).Length() / 2.0f);
}

TEST_CASE("Tutorial: o manche aponta para os quatro lados, e o botao aperta e solta") {
    CHECK(LadoDoManche(0.0f).x < 0.0f);
    CHECK(LadoDoManche(kPorLado).x > 0.0f);
    CHECK(LadoDoManche(2.0f * kPorLado).y < 0.0f);
    CHECK(LadoDoManche(3.0f * kPorLado).y > 0.0f);
    CHECK(LadoDoManche(4.0f * kPorLado).x < 0.0f);   // e da a volta
    CHECK(BotaoApertado(0.0f));
    CHECK_FALSE(BotaoApertado(kToque * 0.75f));
    CHECK(BotaoApertado(kToque * 1.1f));
}
