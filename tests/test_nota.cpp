// A curva da nota e o teto por dano.

#include "doctest.h"

#include "../Source/Regras/Nota.h"
#include "../Source/Aluno/Progresso.h"

// ---- A curva ----

TEST_CASE("Nota: abaixo de 60 o ganho e inteiro") {

    CHECK(Nota::Ganho(0.0f, 0.56f) == doctest::Approx(0.56f));
    CHECK(Nota::Ganho(40.0f, 0.56f) == doctest::Approx(0.56f));
    CHECK(Nota::Ganho(59.9f, 0.56f) == doctest::Approx(0.56f));
}

TEST_CASE("Nota: em 60 a curva nao tem degrau") {

    // O fator vale exatamente 1 no inicio da faixa dificil.
    CHECK(Nota::Ganho(60.0f, 0.56f) == doctest::Approx(0.56f));
}

TEST_CASE("Nota: acima de 60 o ganho so diminui") {

    float anterior = Nota::Ganho(60.0f, 1.0f);
    for (float n = 61.0f; n <= 100.0f; n += 1.0f) {
        const float agora = Nota::Ganho(n, 1.0f);
        CAPTURE(n);
        CHECK(agora <= anterior);
        anterior = agora;
    }
}

TEST_CASE("Nota: em 100 ainda sobra ganho") {

    // Esta e a propriedade que separa os dois freios.
    CHECK(Nota::Ganho(100.0f, 1.0f) > 0.0f);
    CHECK(Nota::Ganho(99.0f, 1.0f) > 0.0f);
}

TEST_CASE("Nota: os ultimos pontos custam muito mais que os primeiros") {

    // Em numeros: subir de 90 para 91 tem de custar bem mais esforco do que subir de 60 para 61.
    const float em60 = Nota::Ganho(60.0f, 1.0f);
    const float em90 = Nota::Ganho(90.0f, 1.0f);
    const float em99 = Nota::Ganho(99.0f, 1.0f);

    CHECK(em90 < em60 / 2.0f);
    CHECK(em99 < em90 / 2.0f);
}

TEST_CASE("Nota: a nota cheia cabe numa batalha, e so na boa") {

    // O numero que justifica o expoente.
    int acertos = 0;
    float n = 40.0f;
    while (n < Nota::kNotaMaxima && acertos < 10000) {
        n = Nota::Somar(n, 0.56f, 0);
        ++acertos;
    }

    CAPTURE(acertos);
    CHECK(acertos < 268);
    CHECK(acertos > 134);
}

TEST_CASE("Nota: a curva nao mexe em perda") {

    // Ganho negativo passa reto.
    CHECK(Nota::Ganho(95.0f, -6.0f) == doctest::Approx(-6.0f));
}

// ---- O teto ----

TEST_CASE("Nota: poucos acertos sofridos nao tiram a nota cheia") {

    for (int i = 0; i < Nota::kAcertosQuePerdemACheia; ++i) {
        CAPTURE(i);
        CHECK_FALSE(Nota::PerdeuACheia(i));
        CHECK(Nota::TetoCom(i) == doctest::Approx(Nota::kNotaMaxima));
    }
}

TEST_CASE("Nota: a partir do limite de acertos o teto cai") {

    CHECK(Nota::PerdeuACheia(Nota::kAcertosQuePerdemACheia));
    CHECK(Nota::TetoCom(Nota::kAcertosQuePerdemACheia) == doctest::Approx(Nota::kTetoComDano));
    CHECK(Nota::TetoCom(99) == doctest::Approx(Nota::kTetoComDano));
}

TEST_CASE("Nota: o teto e abaixo de 100, e nao 100") {

    // Se fosse 100, quem levou dano demais empataria com quem fez corrida limpa.
    CHECK(Nota::kTetoComDano < Nota::kNotaMaxima);
}

TEST_CASE("Nota: somar respeita o teto de quem levou dano") {

    const int muitoDano = Nota::kAcertosQuePerdemACheia;

    // Um ganho enorme nao fura o teto.
    CHECK(Nota::Somar(99.0f, 1000.0f, muitoDano) == doctest::Approx(Nota::kTetoComDano));

    // E sem dano, o mesmo ganho chega a 100.
    CHECK(Nota::Somar(99.0f, 1000.0f, 0) == doctest::Approx(Nota::kNotaMaxima));
}

TEST_CASE("Nota: o teto nao desacelera a subida, so a interrompe") {

    // O dano ja custa nota na hora que acontece; se tambem encolhesse o ganho, puniria duas vezes.
    const float semDano = Nota::Somar(70.0f, 1.0f, 0);
    const float comDano = Nota::Somar(70.0f, 1.0f, Nota::kAcertosQuePerdemACheia);
    CHECK(semDano == doctest::Approx(comDano));
}

// ---- Limites ----

TEST_CASE("Nota: nunca passa de 100 nem cai abaixo de zero") {

    CHECK(Nota::Somar(99.9f, 50.0f, 0) == doctest::Approx(Nota::kNotaMaxima));
    CHECK(Nota::Subtrair(3.0f, 6.0f) == doctest::Approx(0.0f));
    CHECK(Nota::Subtrair(0.0f, 6.0f) == doctest::Approx(0.0f));
}

TEST_CASE("Nota: perder e perder, qualquer que seja a nota") {

    // A perda e a mesma em 40 e em 99: a curva so governa o ganho.
    CHECK(Nota::Subtrair(40.0f, 6.0f) == doctest::Approx(34.0f));
    CHECK(Nota::Subtrair(99.0f, 6.0f) == doctest::Approx(93.0f));
}

TEST_CASE("Nota: chegar a 60 continua custando o mesmo de sempre") {

    // A fase comeca em 40 e aprovar e 60.
    float n = 40.0f;
    int acertos = 0;
    while (n < 60.0f && acertos < 1000) {
        n = Nota::Somar(n, 0.56f, 0);
        ++acertos;
    }
    CHECK(acertos == 36);
}

// ---- A nota cheia ----

TEST_CASE("Nota: so 100 e nota cheia - o teto por dano nao e") {

    // O que separa o dourado do quase.
    CHECK(Nota::ECheia(Nota::kNotaMaxima));
    CHECK_FALSE(Nota::ECheia(Nota::kTetoComDano));
    CHECK_FALSE(Nota::ECheia(99.9f));
    CHECK_FALSE(Nota::ECheia(60.0f));
    CHECK_FALSE(Nota::ECheia(0.0f));
}

TEST_CASE("Nota: quem sobe limpo chega na cheia; quem leva dano nao chega") {

    // A travessia inteira, do inicio de fase ate onde cada um para.
    float limpo = 40.0f, machucado = 40.0f;
    for (int i = 0; i < 400; ++i) {
        limpo     = Nota::Somar(limpo,     0.56f, 0);
        machucado = Nota::Somar(machucado, 0.56f, Nota::kAcertosQuePerdemACheia);
    }

    CHECK(Nota::ECheia(limpo));
    CHECK_FALSE(Nota::ECheia(machucado));
    CHECK(machucado == doctest::Approx(Nota::kTetoComDano));
}

// ---- O que vem depois da fase 3 ----

TEST_CASE("Nota: a nota cheia leva ao teste final, e nao ao fim da batalha") {

    // O pedido.
    CHECK(Nota::AposTerceiraFase(Nota::kNotaMaxima) == Nota::Desfecho::VaiParaFinal);
}

TEST_CASE("Nota: o teto por dano NAO leva ao teste final") {

    // 99,99 e a nota de quem foi atingido demais.
    CHECK(Nota::AposTerceiraFase(Nota::kTetoComDano) == Nota::Desfecho::Aprovado);
    CHECK(Nota::AposTerceiraFase(99.9f)              == Nota::Desfecho::Aprovado);
}

TEST_CASE("Nota: a faixa de exame continua sendo de 40 a 59") {

    CHECK(Nota::AposTerceiraFase(40.0f) == Nota::Desfecho::VaiParaFinal);
    CHECK(Nota::AposTerceiraFase(50.0f) == Nota::Desfecho::VaiParaFinal);
    CHECK(Nota::AposTerceiraFase(59.9f) == Nota::Desfecho::VaiParaFinal);
}

TEST_CASE("Nota: aprovado entre 60 e 99,99; reprovado abaixo de 40") {

    CHECK(Nota::AposTerceiraFase(60.0f) == Nota::Desfecho::Aprovado);
    CHECK(Nota::AposTerceiraFase(85.0f) == Nota::Desfecho::Aprovado);
    CHECK(Nota::AposTerceiraFase(39.9f) == Nota::Desfecho::Reprovado);
    CHECK(Nota::AposTerceiraFase(0.0f)  == Nota::Desfecho::Reprovado);
}

TEST_CASE("Nota: so tres notas no mundo inteiro levam a fase final") {

    // Um buraco aqui seria uma nota que nem termina a batalha nem continua nela.
    for (int centesimos = 0; centesimos <= 10000; ++centesimos) {
        const float n = static_cast<float>(centesimos) / 100.0f;
        const auto d = Nota::AposTerceiraFase(n);
        const bool deveriaIrAFinal = (n >= Nota::kNotaDeExame && n < Nota::kNotaAprovacao)
                                     || n >= Nota::kNotaMaxima;
        CAPTURE(n);
        CHECK((d == Nota::Desfecho::VaiParaFinal) == deveriaIrAFinal);
    }
}

TEST_CASE("Nota: a faixa de exame comeca onde a fase comeca") {

    // Os dois numeros valem 40 e tem de continuar iguais.
    CHECK(Nota::kNotaDeExame == doctest::Approx(Progresso::kNotaInicial));
}

TEST_CASE("Nota: a aprovacao nao se move junto com o joelho da curva") {

    // Valem 60 os dois hoje.
    CHECK(Nota::kNotaAprovacao == doctest::Approx(60.0f));
    CHECK(Nota::kFaixaFacil    == doctest::Approx(60.0f));
}

// ---- O bonus por fase limpa ----

TEST_CASE("Nota: o bonus de fase limpa passa pela curva, como todo ganho") {

    // O que impede o bonus de banalizar o 100.
    const float embaixo = Nota::Somar(45.0f, Nota::kBonusDeFaseLimpa, 0) - 45.0f;
    const float emCima   = Nota::Somar(97.0f, Nota::kBonusDeFaseLimpa, 0) - 97.0f;

    CHECK(embaixo == doctest::Approx(Nota::kBonusDeFaseLimpa));
    CHECK(emCima < embaixo / 5.0f);
    CHECK(emCima > 0.0f);
}

TEST_CASE("Nota: tres fases limpas nao bastam para a nota cheia") {

    // Quem so desvia e nao atira nao chega a 100 por isso.
    float n = Nota::kNotaDeExame;
    for (int fase = 0; fase < 3; ++fase) n = Nota::Somar(n, Nota::kBonusDeFaseLimpa, 0);

    CAPTURE(n);
    CHECK_FALSE(Nota::ECheia(n));
    CHECK(n < 80.0f);
}

TEST_CASE("Nota: o bonus tambem respeita o teto de quem levou dano") {

    // Caso impossivel na pratica - quem levou dano nao tem fase limpa.
    CHECK(Nota::Somar(99.9f, Nota::kBonusDeFaseLimpa, Nota::kAcertosQuePerdemACheia)
          == doctest::Approx(Nota::kTetoComDano));
}

// A regra da nota compartilhada. A dupla tem uma nota.

TEST_CASE("Nota: com um jogador, repartir nao muda nada") {

    // O solo e o caso normal.
    for (const float bruto : {0.0f, 0.56f, 6.0f, 10.0f, 12.0f, 99.5f}) {
        CHECK(Nota::PorJogador(bruto, 1) == doctest::Approx(bruto));
    }
}

TEST_CASE("Nota: com dois jogadores, cada um causa metade") {

    CHECK(Nota::PorJogador(0.56f, 2) == doctest::Approx(0.28f));
    CHECK(Nota::PorJogador(6.0f,  2) == doctest::Approx(3.0f));
    CHECK(Nota::PorJogador(10.0f, 2) == doctest::Approx(5.0f));
}

TEST_CASE("Nota: a dupla precisa do mesmo tanto de acertos que um solo") {

    // Este e o teste da regra.
    constexpr float kGanhoBruto = 0.56f;   // GRADE_CHANGE_UP.

    float solo = Nota::kNotaDeExame;
    for (int i = 0; i < 256; ++i) solo = Nota::Somar(solo, kGanhoBruto, 0);

    float dupla = Nota::kNotaDeExame;
    const float metade = Nota::PorJogador(kGanhoBruto, 2);
    for (int i = 0; i < 512; ++i) dupla = Nota::Somar(dupla, metade, 0);

    CAPTURE(solo);
    CAPTURE(dupla);
    CHECK(dupla == doctest::Approx(solo).epsilon(0.01));
}

TEST_CASE("Nota: a dupla tambem perde no ritmo de um solo") {

    // Dois alvos em campo apanham mais vezes que um.
    constexpr float kPerdaBruta = 6.0f;   // GRADE_CHANGE_DOWN.

    float solo = 100.0f;
    for (int i = 0; i < 5; ++i) solo = Nota::Subtrair(solo, kPerdaBruta);

    float dupla = 100.0f;
    const float metade = Nota::PorJogador(kPerdaBruta, 2);
    for (int i = 0; i < 10; ++i) dupla = Nota::Subtrair(dupla, metade);

    CHECK(dupla == doctest::Approx(solo));
}

TEST_CASE("Nota: nenhum jogador em campo devolve o bruto") {

    // Nao deveria acontecer, e se acontecer dividir por zero daria infinito.
    CHECK(Nota::PorJogador(6.0f, 0) == doctest::Approx(6.0f));
}

TEST_CASE("Nota: repartir e proporcional, e nunca inverte o sinal do efeito") {

    // Tres jogadores nao existem no gabinete, mas a regra nao pode depender disso.
    CHECK(Nota::PorJogador(12.0f, 3) == doctest::Approx(4.0f));
    CHECK(Nota::PorJogador(12.0f, 4) == doctest::Approx(3.0f));

    // E repartir sempre encolhe, nunca cresce nem vira negativo.
    for (size_t quantos = 1; quantos <= 4; ++quantos) {
        const float r = Nota::PorJogador(6.0f, quantos);
        CHECK(r > 0.0f);
        CHECK(r <= 6.0f);
    }
}

// ---- O alivio da perda em grupo ----

TEST_CASE("Nota: apanhar em dupla custa 2,4, e nao 3,0") {

    // O que o alivio trata nao e o total, e a frequencia.
    CHECK(Nota::PerdaPorJogador(6.0f, 2) == doctest::Approx(2.4f));
    CHECK(Nota::PerdaPorJogador(6.0f, 2) < Nota::PorJogador(6.0f, 2));
}

TEST_CASE("Nota: o solo nao ganha alivio nenhum") {

    // O caso normal nao muda por causa de uma regra do co-op (RNF4).
    CHECK(Nota::PerdaPorJogador(6.0f, 1) == doctest::Approx(6.0f));
    CHECK(Nota::PerdaPorJogador(6.0f, 1) == doctest::Approx(Nota::PorJogador(6.0f, 1)));

    // Nem o caso impossivel: zero jogadores devolve o bruto, como PorJogador.
    CHECK(Nota::PerdaPorJogador(6.0f, 0) == doctest::Approx(6.0f));
}

TEST_CASE("Nota: o alivio e SO da perda, e o ganho continua inteiro na divisao") {

    // Este e o caso que protege a regra 2.1.
    CHECK(Nota::PorJogador(0.56f, 2) == doctest::Approx(0.28f));   // metade exata
    CHECK(Nota::PorJogador(10.0f, 2) == doctest::Approx(5.0f));    // o Ponto Extra tambem
}

TEST_CASE("Nota: os dois atingidos no mesmo quadro custam menos que um acerto de solo") {

    const float osDoisDeUmaVez = 2.0f * Nota::PerdaPorJogador(6.0f, 2);

    CHECK(osDoisDeUmaVez == doctest::Approx(4.8f));
    CHECK(osDoisDeUmaVez < 6.0f);
}

TEST_CASE("Nota: a dupla ainda paga pelo erro - o alivio nao e imunidade") {

    // Metade do dano e o piso.
    for (const float bruto : {6.0f, 3.0f, 12.0f}) {
        CAPTURE(bruto);
        const float emDupla = Nota::PerdaPorJogador(bruto, 2);
        CHECK(emDupla > 0.0f);
        CHECK(emDupla > bruto * 0.25f);     // continua doendo
        CHECK(emDupla < bruto * 0.5f);      // e e menos que a divisao pura
    }
}

TEST_CASE("Nota: o ganho por acerto acompanha a duracao da fase, e a nota sobe por segundo o que subia") {
    // O ganho de uma fase inteira de tiro perfeito e o mesmo das fases de 17 s.
    CHECK(Nota::kGanhoPorAcerto * Fase::kDuracao ==
          doctest::Approx(Nota::kGanhoPorAcertoEm17s * Nota::kDuracaoDaFaseMedida));

    CHECK(Nota::kGanhoPorAcerto < Nota::kGanhoPorAcertoEm17s);

    int acertos = 0;
    float n = 40.0f;
    while (n < Nota::kNotaMaxima && acertos < 10000) {
        n = Nota::Somar(n, Nota::kGanhoPorAcerto, 0);
        ++acertos;
    }
    CAPTURE(acertos);
    CHECK(acertos > 256);
    CHECK(acertos == doctest::Approx(256.0f * Fase::kDuracao / Nota::kDuracaoDaFaseMedida).epsilon(0.02));
    CHECK(static_cast<float>(acertos) < 3.0f * Fase::kDuracao / 0.12f);
}
