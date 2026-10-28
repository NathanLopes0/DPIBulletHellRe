// A curva da nota e o teto por dano. Camada pura.
//
// O que estes testes protegem nao e um numero bonito: e a propriedade de que a
// nota continua servindo de ranking. Com ganho constante, 100 era questao de
// tempo de tela, e um ranking em que todos tem 100 nao ordena nada.

#include "doctest.h"

#include "../Source/Nota.h"
#include "../Source/Progresso.h"

// ---------------------------------------------------------------- a curva

TEST_CASE("Nota: abaixo de 60 o ganho e inteiro") {

    // A faixa ate a aprovacao tem de continuar tao facil quanto era antes da
    // curva existir - foi pedido explicitamente.
    CHECK(Nota::Ganho(0.0f, 0.56f) == doctest::Approx(0.56f));
    CHECK(Nota::Ganho(40.0f, 0.56f) == doctest::Approx(0.56f));
    CHECK(Nota::Ganho(59.9f, 0.56f) == doctest::Approx(0.56f));
}

TEST_CASE("Nota: em 60 a curva nao tem degrau") {

    // O fator vale exatamente 1 no inicio da faixa dificil. Sem isso haveria um
    // tranco visivel no instante em que o aluno passa de 60.
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

    // ESTA E A PROPRIEDADE QUE SEPARA OS DOIS FREIOS. Se a curva zerasse em
    // 100, a nota tenderia a 100 sem chegar, e a comemoracao de nota cheia
    // nunca aconteceria. Quem impede o 100 e o teto por dano, nao a curva.
    CHECK(Nota::Ganho(100.0f, 1.0f) > 0.0f);
    CHECK(Nota::Ganho(99.0f, 1.0f) > 0.0f);
}

TEST_CASE("Nota: os ultimos pontos custam muito mais que os primeiros") {

    // Em numeros: subir de 90 para 91 tem de custar bem mais esforco do que
    // subir de 60 para 61.
    //
    // ISTO E UM PISO PARA kExpoente, e nao so uma checagem de monotonia: um
    // expoente abaixo de ~1,1 faz o ganho minguar tao devagar que a curva deixa
    // de morder, e e esta linha que recusa. Se ela falhar depois de alguem
    // baixar o expoente, a falha e a resposta - nao o teste.
    const float em60 = Nota::Ganho(60.0f, 1.0f);
    const float em90 = Nota::Ganho(90.0f, 1.0f);
    const float em99 = Nota::Ganho(99.0f, 1.0f);

    CHECK(em90 < em60 / 2.0f);
    CHECK(em99 < em90 / 2.0f);
}

TEST_CASE("Nota: a nota cheia cabe numa batalha, e so na boa") {

    // O NUMERO QUE JUSTIFICA O EXPOENTE. Medido no jogo rodando: uma batalha
    // entrega entre 268 e 382 acertos no chefe. A nota cheia tem de custar menos
    // que o piso dessa faixa (senao ninguem chega) e mais que a metade dela
    // (senao chega todo mundo, e a nota para de ordenar um ranking).
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

    // Ganho negativo passa reto. Quem perde, perde inteiro - fazer o dano
    // minguar junto deixaria quem esta perto de 100 quase imune.
    CHECK(Nota::Ganho(95.0f, -6.0f) == doctest::Approx(-6.0f));
}

// ---------------------------------------------------------------- o teto

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

    // Se fosse 100, quem levou dano demais empataria com quem fez corrida limpa
    // - e o ranking perderia justamente a distincao que o teto existe para criar.
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

    // O dano ja custa nota na hora que acontece; se tambem encolhesse o ganho,
    // puniria duas vezes. Entao ate 99.99 a subida de quem levou dano e igual a
    // de quem nao levou.
    const float semDano = Nota::Somar(70.0f, 1.0f, 0);
    const float comDano = Nota::Somar(70.0f, 1.0f, Nota::kAcertosQuePerdemACheia);
    CHECK(semDano == doctest::Approx(comDano));
}

// ---------------------------------------------------------------- limites

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

    // A fase comeca em 40 e aprovar e 60. Com ganho de 0,56 por acerto, sao
    // 36 acertos - e a curva nao pode ter mudado isso.
    float n = 40.0f;
    int acertos = 0;
    while (n < 60.0f && acertos < 1000) {
        n = Nota::Somar(n, 0.56f, 0);
        ++acertos;
    }
    CHECK(acertos == 36);
}

// ------------------------------------------------------------- a nota cheia

TEST_CASE("Nota: so 100 e nota cheia - o teto por dano nao e") {

    // O QUE SEPARA O DOURADO DO QUASE. Quem levou dano demais chega a 99,99 e
    // para ali; se isso acendesse o dourado, o jogador leria nota cheia sem ter
    // tirado, e as duas coisas que o teto cria - a distincao e a comemoracao -
    // morreriam juntas.
    CHECK(Nota::ECheia(Nota::kNotaMaxima));
    CHECK_FALSE(Nota::ECheia(Nota::kTetoComDano));
    CHECK_FALSE(Nota::ECheia(99.9f));
    CHECK_FALSE(Nota::ECheia(60.0f));
    CHECK_FALSE(Nota::ECheia(0.0f));
}

TEST_CASE("Nota: quem sobe limpo chega na cheia; quem leva dano nao chega") {

    // A travessia inteira, do inicio de fase ate onde cada um para. Protege a
    // ligacao entre as tres pecas - curva, teto e limiar do dourado - que
    // separadas ja estao testadas, mas que so juntas respondem "o dourado
    // acende?".
    float limpo = 40.0f, machucado = 40.0f;
    for (int i = 0; i < 400; ++i) {
        limpo     = Nota::Somar(limpo,     0.56f, 0);
        machucado = Nota::Somar(machucado, 0.56f, Nota::kAcertosQuePerdemACheia);
    }

    CHECK(Nota::ECheia(limpo));
    CHECK_FALSE(Nota::ECheia(machucado));
    CHECK(machucado == doctest::Approx(Nota::kTetoComDano));
}

// ------------------------------------------- o que vem depois da fase 3

TEST_CASE("Nota: a nota cheia leva ao teste final, e nao ao fim da batalha") {

    // O PEDIDO. Quem chega a 100 ganha a fase final como premio: subiu a curva
    // inteira, entao enfrenta o ultimo padrao em vez de a batalha acabar ali.
    CHECK(Nota::AposTerceiraFase(Nota::kNotaMaxima) == Nota::Desfecho::VaiParaFinal);
}

TEST_CASE("Nota: o teto por dano NAO leva ao teste final") {

    // 99,99 e a nota de quem foi atingido demais. Ele passou, e passa ali mesmo:
    // mandar para a fase final quem perdeu a nota cheia daria ao castigo a mesma
    // recompensa que a corrida limpa.
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

    // Varre o intervalo inteiro de centesimo em centesimo e confere que o
    // conjunto que vai para a fase final e exatamente [40, 60) mais o 100. Um
    // buraco aqui seria uma nota que nem termina a batalha nem continua nela.
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

    // Os dois numeros valem 40 e TEM de continuar iguais: a faixa de exame
    // significa "nao perdeu terreno e nao passou". Se a nota inicial mudasse
    // sozinha, existiria uma faixa abaixo dela que reprova sem ninguem nunca ter
    // estado la, ou uma acima que da exame a quem ja subiu.
    CHECK(Nota::kNotaDeExame == doctest::Approx(Progresso::kNotaInicial));
}

TEST_CASE("Nota: a aprovacao nao se move junto com o joelho da curva") {

    // Valem 60 os dois hoje. Sao conceitos diferentes - regra academica e
    // dificuldade - e este teste existe para que mexer num nao mexa no outro sem
    // alguem perceber.
    CHECK(Nota::kNotaAprovacao == doctest::Approx(60.0f));
    CHECK(Nota::kFaixaFacil    == doctest::Approx(60.0f));
}

// ------------------------------------------- o bonus por fase limpa

TEST_CASE("Nota: o bonus de fase limpa passa pela curva, como todo ganho") {

    // O QUE IMPEDE O BONUS DE BANALIZAR O 100. Ele e um ganho como outro
    // qualquer, entao vale cheio embaixo e quase nada no topo. Se fosse somado
    // cru, tres fases limpas dariam 36 pontos de presente a quem ja estivesse
    // em 95 - e a nota cheia deixaria de significar o que significa.
    const float embaixo = Nota::Somar(45.0f, Nota::kBonusDeFaseLimpa, 0) - 45.0f;
    const float emCima   = Nota::Somar(97.0f, Nota::kBonusDeFaseLimpa, 0) - 97.0f;

    CHECK(embaixo == doctest::Approx(Nota::kBonusDeFaseLimpa));
    CHECK(emCima < embaixo / 5.0f);
    CHECK(emCima > 0.0f);
}

TEST_CASE("Nota: tres fases limpas nao bastam para a nota cheia") {

    // Quem so desvia e nao atira nao chega a 100 por isso. A fase comeca em 40;
    // tres bonus seguidos, sem acerto nenhum, tem de parar bem longe da cheia.
    float n = Nota::kNotaDeExame;
    for (int fase = 0; fase < 3; ++fase) n = Nota::Somar(n, Nota::kBonusDeFaseLimpa, 0);

    CAPTURE(n);
    CHECK_FALSE(Nota::ECheia(n));
    CHECK(n < 80.0f);
}

TEST_CASE("Nota: o bonus tambem respeita o teto de quem levou dano") {

    // Caso impossivel na pratica - quem levou dano nao tem fase limpa - mas a
    // aritmetica nao pode depender disso para nao furar o teto.
    CHECK(Nota::Somar(99.9f, Nota::kBonusDeFaseLimpa, Nota::kAcertosQuePerdemACheia)
          == doctest::Approx(Nota::kTetoComDano));
}

// ---------------------------------------------------------------------------
// A REGRA DA NOTA COMPARTILHADA (requisitos-coop, 2.1)
//
// A dupla tem UMA nota. Sem repartir o que cada um causa, ela seria outra coisa
// em cada direcao: dois atiradores enchem a mesma barra no dobro da velocidade,
// e dois alvos em campo a drenam no dobro. Repartido, o co-op sobe e desce no
// ritmo de um solo - e o que sobra a favor dele e so os ataques mirados se
// dividirem entre os dois.
//
// O QUE ESTES TESTES PROTEGEM DE VERDADE e o caso de UM jogador. Dividir por um
// nao pode mudar numero nenhum: e a linha que, errada, estragaria em silencio
// todas as notas de solo ja gravadas.
// ---------------------------------------------------------------------------

TEST_CASE("Nota: com um jogador, repartir nao muda nada") {

    // O SOLO E O CASO NORMAL (requisitos-coop, RNF4). Esta verificacao vale
    // mais que todas as de co-op juntas: o jogo inteiro passa por aqui.
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

    // ESTE E O TESTE DA REGRA. Um solo chega aos 100 com 256 acertos; a dupla
    // soma o dobro no mesmo tempo, com cada acerto valendo metade. As duas
    // notas tem de dar praticamente a mesma coisa.
    //
    // "Praticamente" porque a curva le a nota a cada passo, e 512 passos de
    // meio ganho nao sao identicos a 256 de ganho inteiro.
    //
    // MEDIDO, e nao suposto: eu tinha escrito aqui que a dupla sairia um pouco
    // ACIMA do solo, por cada passo ver uma nota menor. Rodando, ela sai um
    // pouco ABAIXO - o acumulo de arredondamento em 512 somas pesa mais que
    // isso. A diferenca maxima na faixa inteira e de 0,05 ponto, em 83:
    //
    //   acertos    25     50    100    150    200    256
    //   solo    54,00  67,18  83,07  91,50  96,51  100,00
    //   dupla   54,00  67,16  83,02  91,46  96,48  100,00
    //
    // O que o teste protege e o TAMANHO dessa diferenca. Se ela crescer, a
    // regra deixou de valer e o co-op virou outro jogo.
    constexpr float kGanhoBruto = 0.56f;   // GRADE_CHANGE_UP

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

    // Dois alvos em campo apanham mais vezes que um. Com a perda repartida, o
    // mesmo numero de acertos SOMADOS custa o mesmo que custaria a um solo.
    constexpr float kPerdaBruta = 6.0f;   // GRADE_CHANGE_DOWN

    float solo = 100.0f;
    for (int i = 0; i < 5; ++i) solo = Nota::Subtrair(solo, kPerdaBruta);

    float dupla = 100.0f;
    const float metade = Nota::PorJogador(kPerdaBruta, 2);
    for (int i = 0; i < 10; ++i) dupla = Nota::Subtrair(dupla, metade);

    CHECK(dupla == doctest::Approx(solo));
}

TEST_CASE("Nota: nenhum jogador em campo devolve o bruto") {

    // Nao deveria acontecer, e se acontecer dividir por zero daria infinito -
    // trocar um caso impossivel por um estrago real. Devolver o bruto deixa a
    // batalha seguir com o numero de sempre.
    CHECK(Nota::PorJogador(6.0f, 0) == doctest::Approx(6.0f));
}

TEST_CASE("Nota: repartir e proporcional, e nunca inverte o sinal do efeito") {

    // Tres jogadores nao existem no gabinete, mas a regra nao pode depender
    // disso: ela diz "pelo numero de jogadores", e e isso que ela faz.
    CHECK(Nota::PorJogador(12.0f, 3) == doctest::Approx(4.0f));
    CHECK(Nota::PorJogador(12.0f, 4) == doctest::Approx(3.0f));

    // E repartir sempre encolhe, nunca cresce nem vira negativo.
    for (size_t quantos = 1; quantos <= 4; ++quantos) {
        const float r = Nota::PorJogador(6.0f, quantos);
        CHECK(r > 0.0f);
        CHECK(r <= 6.0f);
    }
}

// ------------------------------------ o alivio da perda em grupo

TEST_CASE("Nota: apanhar em dupla custa 2,4, e nao 3,0") {

    // A DIVISAO SOZINHA DAVA 3,0 (metade de 6,0), e o total ja era favoravel a
    // dupla: medido, ela apanhava 57% mais vezes e pagava 21% menos no fim
    // (requisitos-coop, secao 8). O que o alivio trata nao e o total, e a
    // FREQUENCIA: a barra caia quase no dobro das vezes, e e a frequencia que o
    // jogador sente.
    CHECK(Nota::PerdaPorJogador(6.0f, 2) == doctest::Approx(2.4f));
    CHECK(Nota::PerdaPorJogador(6.0f, 2) < Nota::PorJogador(6.0f, 2));
}

TEST_CASE("Nota: o solo nao ganha alivio nenhum") {

    // O CASO NORMAL NAO MUDA POR CAUSA DE UMA REGRA DO CO-OP (RNF4). Se o solo
    // passasse a pagar menos, a curva inteira da nota - que foi calibrada nele -
    // mudaria de sentido sem ninguem pedir.
    CHECK(Nota::PerdaPorJogador(6.0f, 1) == doctest::Approx(6.0f));
    CHECK(Nota::PerdaPorJogador(6.0f, 1) == doctest::Approx(Nota::PorJogador(6.0f, 1)));

    // Nem o caso impossivel: zero jogadores devolve o bruto, como PorJogador.
    CHECK(Nota::PerdaPorJogador(6.0f, 0) == doctest::Approx(6.0f));
}

TEST_CASE("Nota: o alivio e SO da perda, e o ganho continua inteiro na divisao") {

    // ESTE E O CASO QUE PROTEGE A REGRA 2.1. O ganho e dividido justamente para
    // a barra subir no ritmo de um solo; aliviar o ganho junto faria a dupla
    // subir mais devagar que o solo, que e o contrario do que a regra quer.
    CHECK(Nota::PorJogador(0.56f, 2) == doctest::Approx(0.28f));   // metade exata
    CHECK(Nota::PorJogador(10.0f, 2) == doctest::Approx(5.0f));    // o Ponto Extra tambem
}

TEST_CASE("Nota: os dois atingidos no mesmo quadro custam menos que um acerto de solo") {

    // ERA O PIOR CASO, E ERA EXATAMENTE UM ACERTO DE SOLO: 3,0 + 3,0 = 6,0.
    // Com o alivio sao 4,8 - a dupla desatenta ainda paga caro, so que nao o
    // preco cheio de um erro que um jogador sozinho nao teria como cometer.
    const float osDoisDeUmaVez = 2.0f * Nota::PerdaPorJogador(6.0f, 2);

    CHECK(osDoisDeUmaVez == doctest::Approx(4.8f));
    CHECK(osDoisDeUmaVez < 6.0f);
}

TEST_CASE("Nota: a dupla ainda paga pelo erro - o alivio nao e imunidade") {

    // Um alivio grande demais tiraria o peso do erro e o co-op viraria o modo
    // facil, que e o que a secao 8 descartou. Metade do dano e o piso.
    for (const float bruto : {6.0f, 3.0f, 12.0f}) {
        CAPTURE(bruto);
        const float emDupla = Nota::PerdaPorJogador(bruto, 2);
        CHECK(emDupla > 0.0f);
        CHECK(emDupla > bruto * 0.25f);     // continua doendo
        CHECK(emDupla < bruto * 0.5f);      // e e menos que a divisao pura
    }
}

TEST_CASE("Nota: o ganho por acerto acompanha a duracao da fase, e a nota sobe por segundo o que subia") {
    // O GANHO DE UMA FASE INTEIRA DE TIRO PERFEITO e o mesmo das fases de
    // 17 s: o tiro sai no mesmo ritmo, e com mais tempo cada acerto vale menos.
    CHECK(Nota::kGanhoPorAcerto * Fase::kDuracao ==
          doctest::Approx(Nota::kGanhoPorAcertoEm17s * Nota::kDuracaoDaFaseMedida));

    CHECK(Nota::kGanhoPorAcerto < Nota::kGanhoPorAcertoEm17s);

    // A NOTA CHEIA CUSTA MAIS ACERTOS, na mesma proporcao do tempo, e cabe
    // nos 500 tiros de uma batalha de 60 s (um a cada 0,12 s).
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
