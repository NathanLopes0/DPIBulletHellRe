//
// A conta da nota de uma batalha: o bonus de fase limpa, o acerto sofrido e o Ponto Extra.
//

#include "doctest.h"

#include "../Source/Regras/ContaDaNota.h"
#include "../Source/Regras/PontoExtra.h"

TEST_CASE("Conta: comeca na nota dada, sem acertos e sem fases limpas") {
    const Nota::Conta conta(40.0f, 2);
    CHECK(conta.Valor() == doctest::Approx(40.0f));
    CHECK(conta.AcertosSofridos() == 0);
    CHECK(conta.FasesLimpas() == 0);
}

TEST_CASE("Conta: a primeira fase fechada e a entrada na primeira fase, e nao paga") {
    Nota::Conta conta(40.0f);
    CHECK_FALSE(conta.FecharFase());
    CHECK(conta.Valor() == doctest::Approx(40.0f));
    CHECK(conta.FasesLimpas() == 0);
}

TEST_CASE("Conta: a fase sem acerto paga o bonus inteiro, sozinho e na dupla") {
    for (const std::size_t jogadores : {std::size_t{1}, std::size_t{2}}) {
        CAPTURE(jogadores);
        Nota::Conta conta(40.0f, jogadores);
        conta.FecharFase();
        CHECK(conta.FecharFase());
        CHECK(conta.Valor() == doctest::Approx(Nota::Somar(40.0f, Nota::kBonusDeFaseLimpa, 0)));
        CHECK(conta.FasesLimpas() == 1);
    }
}

TEST_CASE("Conta: um acerto tira o bonus da fase dele, e a fase seguinte comeca do zero") {
    Nota::Conta conta(50.0f);
    conta.FecharFase();
    conta.Descer();
    const float depoisDoAcerto = conta.Valor();
    CHECK_FALSE(conta.FecharFase());
    CHECK(conta.Valor() == doctest::Approx(depoisDoAcerto));
    CHECK(conta.FecharFase());
    CHECK(conta.FasesLimpas() == 1);
}

TEST_CASE("Conta: o acerto sofrido conta, e Descer devolve quanto a nota caiu") {
    Nota::Conta conta(50.0f);
    const float perda = conta.Descer();
    CHECK(perda == doctest::Approx(Nota::PerdaPorJogador(Nota::kPerdaPorAcerto, 1)));
    CHECK(conta.Valor() == doctest::Approx(50.0f - perda));
    CHECK(conta.AcertosSofridos() == 1);

    // Perto do zero, cai so o que havia.
    Nota::Conta quase(2.0f);
    CHECK(quase.Descer() == doctest::Approx(2.0f));
    CHECK(quase.Valor() == doctest::Approx(0.0f));
}

TEST_CASE("Conta: o multiplicador da perda multiplica o que cai") {
    Nota::Conta um(50.0f), dois(50.0f);
    CHECK(dois.Descer(2.0f) == doctest::Approx(2.0f * um.Descer(1.0f)));
}

TEST_CASE("Conta: na dupla o ganho de um e dividido, e a perda e dividida e aliviada") {
    Nota::Conta conta(50.0f, 2);
    CHECK(conta.PorJogador(1.0f) == doctest::Approx(Nota::PorJogador(1.0f, 2)));
    CHECK(conta.Descer() == doctest::Approx(Nota::PerdaPorJogador(Nota::kPerdaPorAcerto, 2)));
}

TEST_CASE("Conta: o teto cai com os acertos sofridos") {
    Nota::Conta conta(90.0f);
    for (int i = 0; i < Nota::kAcertosQuePerdemACheia; ++i) conta.Descer();
    conta.Subir(100.0f);
    CHECK(conta.Valor() <= doctest::Approx(Nota::TetoCom(Nota::kAcertosQuePerdemACheia)));
    CHECK_FALSE(Nota::ECheia(conta.Valor()));
}

TEST_CASE("Conta: o Ponto Extra vale os acertos da sobrecarga, repartidos") {
    for (const std::size_t jogadores : {std::size_t{1}, std::size_t{2}}) {
        CAPTURE(jogadores);
        Nota::Conta conta(40.0f, jogadores);
        conta.GastarPontoExtra();
        CHECK(conta.Valor() == doctest::Approx(PontoExtra::NotaDepois(
                                   40.0f, 0, Nota::PorJogador(Nota::kGanhoPorAcerto, jogadores),
                                   PontoExtra::TirosNaSobrecarga())));
        CHECK(conta.Valor() > 40.0f);
    }
}

TEST_CASE("Conta: a transacao - ROLLBACK se a nota caiu (e os acertos deixam de contar), COMMIT se nao") {
    // Caiu: volta a nota, o teto e a fase limpa.
    Nota::Conta c(60.0f);
    c.FecharFase();                      // a entrada na primeira fase
    const Nota::Conta::Marca marca = c.Marcar();
    c.Descer();
    c.Descer();
    CHECK(c.Valor() < 60.0f);
    CHECK(c.AcertosSofridos() == 2);
    CHECK(c.Desfazer(marca));
    CHECK(c.Valor() == doctest::Approx(60.0f));
    CHECK(c.AcertosSofridos() == 0);
    CHECK(c.FecharFase());               // sem acerto que conte: a fase paga o bonus
    CHECK(c.FasesLimpas() == 1);

    // Os acertos de antes do BEGIN continuam contando.
    Nota::Conta d(60.0f);
    d.FecharFase();
    d.Descer();
    const Nota::Conta::Marca depoisDeUm = d.Marcar();
    d.Descer();
    CHECK(d.Desfazer(depoisDeUm));
    CHECK(d.Valor() == doctest::Approx(depoisDeUm.nota));
    CHECK(d.AcertosSofridos() == 1);
    CHECK_FALSE(d.FecharFase());

    // Nao caiu: COMMIT, e nada muda - nem os acertos que a nota ja recuperou.
    Nota::Conta e(60.0f);
    const Nota::Conta::Marca antes = e.Marcar();
    e.Descer();
    for (int i = 0; i < 400; ++i) e.Subir(Nota::kGanhoPorAcerto);
    const float subiu = e.Valor();
    REQUIRE(subiu > 60.0f);
    CHECK_FALSE(e.Desfazer(antes));
    CHECK(e.Valor() == doctest::Approx(subiu));
    CHECK(e.AcertosSofridos() == 1);
}
