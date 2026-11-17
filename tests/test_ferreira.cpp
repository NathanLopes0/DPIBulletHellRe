//
// O Lucas Ferreira (INF 216 - Projeto e Implementacao de Jogos Digitais): a camada pura das quatro provas.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#include "../Source/Attacks/Fase.h"
#include "../Source/Attacks/JogosDigitais.h"
#include "../Source/Regras/Nota.h"

TEST_CASE("JogosDigitais: as fases dele duram o dobro, e as etapas somam a fase") {
    using namespace JogosDigitais;
    CHECK(kDuracaoDaFase == doctest::Approx(2.0f * Fase::kDuracao));
    for (const auto& d : {HudHostil::kDuracoes, RegrasTrocadas::kDuracoes, MotorQuebrado::kDuracoes, Lancamento::kDuracoes}) {
        CHECK(d[0] + d[1] + d[2] == doctest::Approx(kDuracaoDaFase));
        CHECK(Etapa(d, 0.0f) == 0);
        CHECK(Etapa(d, d[0] - 0.01f) == 0);
        CHECK(Etapa(d, d[0] + 0.01f) == 1);
        CHECK(Etapa(d, d[0] + d[1] + 0.01f) == 2);
        CHECK(Etapa(d, 999.0f) == 2);
        CHECK(NaEtapa(d, d[0] + 2.5f) == doctest::Approx(2.5f));
        CHECK(NaEtapa(d, 1.0f) == doctest::Approx(1.0f));
        CHECK(NaEtapa(d, d[0] + d[1] + 3.0f) == doctest::Approx(3.0f));
    }
}

// ---- A interface ----

TEST_CASE("HudHostil: a barra enche ate a nota, e com 100 nao ha por onde passar") {
    using namespace HudHostil;
    CHECK(Cheios(0.0f, 40) == 0);
    CHECK(Cheios(50.0f, 40) == 20);
    CHECK(Cheios(100.0f, 40) == 40);            // impossivel escapar
    CHECK(Cheios(99.99f, 40) == 39);            // o teto de quem ja foi acertado deixa um segmento
    CHECK(Cheios(250.0f, 40) == 40);
    CHECK(Cheios(-3.0f, 40) == 0);
    CHECK(Cheios(50.0f, 0) == 0);
    // A nota maior deixa menos vazio.
    for (int n = 0; n < 100; n += 7) CHECK(Cheios(static_cast<float>(n), 40) <= Cheios(static_cast<float>(n + 7), 40));
    // A direita, vazia; espelhada, e a esquerda que fica vazia - sempre o mesmo tanto.
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
    // As cores sao as do rodape.
    CHECK(std::string(CaraDaBarra(Nota::kNotaDeExame - 1.0f)) == "Vermelha");
    CHECK(std::string(CaraDaBarra(Nota::kNotaAprovacao - 1.0f)) == "Amarela");
    CHECK(std::string(CaraDaBarra(Nota::kNotaAprovacao)) == "Verde");
}

TEST_CASE("HudHostil: a varredura vai e volta, e a barra espelha antes de cada passada") {
    using namespace HudHostil;
    const float ida = 2.5f;
    // No aviso ela esta parada na ponta de onde sai.
    CHECK(NaVarredura(0.5f, ida).parada);
    CHECK(NaVarredura(0.5f, ida).onde == doctest::Approx(0.0f));
    CHECK_FALSE(NaVarredura(0.5f, ida).espelhada);
    // A primeira passada: do 0 ao 1, sem espelhar.
    const Varredura meio = NaVarredura(kAvisoDaBarra + ida / 2.0f, ida);
    CHECK(meio.onde == doctest::Approx(0.5f));
    CHECK(meio.passada == 0);
    CHECK_FALSE(meio.parada);
    CHECK_FALSE(meio.espelhada);
    // O descanso na outra ponta: ja espelhada.
    const Varredura descanso = NaVarredura(kAvisoDaBarra + ida + kPausaDaBarra / 2.0f, ida);
    CHECK(descanso.parada);
    CHECK(descanso.onde == doctest::Approx(1.0f));
    CHECK(descanso.espelhada);
    // A volta: do 1 ao 0, espelhada.
    const Varredura volta = NaVarredura(kAvisoDaBarra + ida + kPausaDaBarra + ida / 4.0f, ida);
    CHECK(volta.passada == 1);
    CHECK(volta.onde == doctest::Approx(0.75f));
    CHECK(volta.espelhada);
    // Sempre dentro do campo, e cabem pelo menos tres passadas na etapa.
    for (float t = 0.0f; t < kDuracoes[0]; t += 0.05f) {
        const Varredura v = NaVarredura(t, ida);
        CHECK(v.onde >= 0.0f);
        CHECK(v.onde <= 1.0f);
    }
    CHECK(NaVarredura(kDuracoes[0] - 0.1f, ida).passada >= 3);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).idaDaBarra < DegrauDo(Dificuldade::Nivel::Normal).idaDaBarra);
}

TEST_CASE("HudHostil: a janela e uma moldura fechada com o X na barra de titulo") {
    using namespace HudHostil;
    const Vector2 c(600.0f, 400.0f);
    const std::vector<Vector2> pecas = Moldura(c);
    const Vector2 x = BotaoDeFechar(c);
    // O X fica na linha de cima, para dentro do canto, e nenhuma peca esta em cima dele.
    CHECK(x.y == doctest::Approx(c.y - kAlturaDaJanela / 2.0f));
    CHECK(x.x < c.x + kLarguraDaJanela / 2.0f - kPecaDaJanela);
    CHECK(x.x > c.x);
    std::set<std::pair<int, int>> vistas;
    for (const Vector2& p : pecas) {
        CHECK((p - x).Length() > kPecaDaJanela / 2.0f);
        // Toda peca esta na borda do retangulo.
        const bool naHorizontal = std::fabs(std::fabs(p.y - c.y) - kAlturaDaJanela / 2.0f) < 0.5f;
        const bool naVertical = std::fabs(std::fabs(p.x - c.x) - kLarguraDaJanela / 2.0f) < 0.5f;
        CHECK((naHorizontal || naVertical));
        CHECK(vistas.insert({static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))}).second);
    }
    // O perimetro inteiro, de peca em peca, menos a do X.
    const int porLinha = static_cast<int>(kLarguraDaJanela / kPecaDaJanela) + 1;
    const int porColuna = static_cast<int>(kAlturaDaJanela / kPecaDaJanela) - 1;
    CHECK(static_cast<int>(pecas.size()) == 2 * porLinha + 2 * porColuna - 1);
    // A janela abre em volta do aluno, sem sair do campo.
    CHECK(CentroDaJanela(Vector2(600.0f, 400.0f), 0.0f, 170.0f, 1200.0f, 746.0f).x == doctest::Approx(600.0f));
    const Vector2 canto = CentroDaJanela(Vector2(5.0f, 740.0f), 0.0f, 170.0f, 1200.0f, 746.0f);
    CHECK(canto.x - kLarguraDaJanela / 2.0f >= 0.0f);
    CHECK(canto.y + kAlturaDaJanela / 2.0f <= 746.0f);
    const Vector2 alto = CentroDaJanela(Vector2(1195.0f, 20.0f), 0.0f, 170.0f, 1200.0f, 746.0f);
    CHECK(alto.x + kLarguraDaJanela / 2.0f <= 1200.0f);
    CHECK(alto.y - kAlturaDaJanela / 2.0f >= 170.0f);
    std::set<std::string> dicas(kDicas.begin(), kDicas.end());
    CHECK(dicas.size() == kDicas.size());
}

// ---- As regras ----

TEST_CASE("RegrasTrocadas: a inversao avisa, vale, avisa de novo e volta") {
    using namespace RegrasTrocadas;
    CHECK(RegraEm(0.0f) == Regra::Aviso);
    CHECK(RegraEm(kComecaAInversao + 0.1f) == Regra::Invertida);
    CHECK(RegraEm(kVoltaEm + 0.1f) == Regra::Voltando);
    CHECK(RegraEm(kVoltou + 0.1f) == Regra::Normal);
    CHECK(kVoltou < kDuracoes[0]);          // sobra etapa com as regras de sempre
    // Invertida: o tiro e moeda e o acerto cobra - os dois juntos, sempre.
    for (const Regra r : {Regra::Aviso, Regra::Invertida, Regra::Voltando, Regra::Normal}) {
        CHECK(TiroEMoeda(r) == AcertoCobra(r));
    }
    CHECK(TiroEMoeda(Regra::Invertida));
    CHECK(TiroEMoeda(Regra::Voltando));       // ate a volta, a moeda ainda paga
    CHECK_FALSE(TiroEMoeda(Regra::Aviso));
    CHECK_FALSE(TiroEMoeda(Regra::Normal));
    // So na volta a moeda pisca, e pisca mesmo: as duas caras aparecem.
    CHECK_FALSE(MoedaPisca(Regra::Invertida, 5.0f));
    bool acesa = false, apagada = false;
    for (float t = kVoltaEm; t < kVoltou; t += 0.03f) (MoedaPisca(Regra::Voltando, t) ? acesa : apagada) = true;
    CHECK(acesa);
    CHECK(apagada);
    // Acertar custa mais do que rende: a multa passa do ganho de um acerto.
    CHECK(kMultaPorAcerto > 2.0f * Nota::kGanhoPorAcerto);
    CHECK(kPremioDaMoeda > 0.0f);
}

TEST_CASE("RegrasTrocadas: o arremesso cai no alvo, e da para subir contra a gravidade") {
    using namespace RegrasTrocadas;
    const Vector2 de(600.0f, 100.0f), alvo(250.0f, 700.0f);
    const Vector2 v = Lancar(de, alvo, kVooDoArremesso, kGravidade);
    const Vector2 chegada = NoArco(de, v, kVooDoArremesso, kGravidade);
    CHECK(chegada.x == doctest::Approx(alvo.x).epsilon(1e-3));
    CHECK(chegada.y == doctest::Approx(alvo.y).epsilon(1e-3));
    CHECK(NoArco(de, v, 0.0f, kGravidade).x == doctest::Approx(de.x));
    // O arco sobe antes de cair quando o alvo esta perto na vertical.
    const Vector2 raso = Lancar(Vector2(600.0f, 300.0f), Vector2(700.0f, 320.0f), kVooDoArremesso, kGravidade);
    CHECK(raso.y < 0.0f);
    // O aluno anda a 300 px/s: a queda e menor, e ele sobe.
    CHECK(kQueda < 300.0f);
    CHECK(kQueda > 100.0f);
}

TEST_CASE("RegrasTrocadas: no fogo amigo o campo da a volta") {
    using namespace RegrasTrocadas;
    CHECK(SaiuPorCima(5.0f, 0.0f));
    CHECK(SaiuPorCima(-12.0f, 0.0f));
    CHECK_FALSE(SaiuPorCima(kFaixaDeSaida + 1.0f, 0.0f));
    // O tiro do aluno anda 25,6 px por quadro (1600 px/s a 16 ms): a faixa pega pelo menos um quadro.
    CHECK(kFaixaDeSaida > 1600.0f * 0.016f);
    CHECK(SaiuPorBaixo(760.0f, 746.0f));
    CHECK_FALSE(SaiuPorBaixo(740.0f, 746.0f));
    const Vector2 volta = OndeVolta(321.0f, 746.0f);
    CHECK(volta.x == doctest::Approx(321.0f));
    CHECK(volta.y > 746.0f);
    CHECK(kVelocidadeDoRetorno > 0.0f);
}

// ---- O motor ----

TEST_CASE("MotorQuebrado: o lag anda aos saltos, e a hitbox descola do desenho") {
    using namespace MotorQuebrado;
    CHECK(QuadroEm(0.0f) == 0);
    CHECK(QuadroEm(kQuadroDoLag - 0.01f) == 0);
    CHECK(QuadroEm(kQuadroDoLag + 0.01f) == 1);
    CHECK(QuadroEm(-1.0f) == 0);
    const Vector2 p = NoQuadro(Vector2(100.0f, 100.0f), Vector2(200.0f, -50.0f), 3);
    CHECK(p.x == doctest::Approx(100.0f + 200.0f * 3.0f * kQuadroDoLag));
    CHECK(p.y == doctest::Approx(100.0f - 50.0f * 3.0f * kQuadroDoLag));
    CHECK((NoQuadro(Vector2(7.0f, 9.0f), Vector2(200.0f, 200.0f), 0) - Vector2(7.0f, 9.0f)).Length() < 1e-4f);
    // A hitbox comeca em cima do desenho, abre aos poucos e depois gira sem mudar de distancia.
    CHECK(Descolamento(0.0f).Length() == doctest::Approx(0.0f));
    CHECK(Descolamento(kAbreODescolamento / 2.0f).Length() == doctest::Approx(kDescolamento / 2.0f).epsilon(1e-3));
    CHECK(Descolamento(kAbreODescolamento + 1.0f).Length() == doctest::Approx(kDescolamento).epsilon(1e-3));
    CHECK(Descolamento(9.0f).Length() == doctest::Approx(kDescolamento).epsilon(1e-3));
    CHECK((Descolamento(5.0f) - Descolamento(8.0f)).Length() > 10.0f);
    // Descolada, a hitbox nao encosta mais no desenho (os dois tem uns 8 px de raio).
    CHECK(kDescolamento > 2.0f * 10.0f);
}

TEST_CASE("MotorQuebrado: cada save tem o load dele, e o load vem depois") {
    using namespace MotorQuebrado;
    REQUIRE(kSaves.size() == kLoads.size());
    for (size_t k = 0; k < kSaves.size(); ++k) {
        CHECK(kSaves[k] < kLoads[k]);
        CHECK(kLoads[k] < kDuracoes[2]);
        if (k > 0) CHECK(kSaves[k] > kLoads[k - 1]);
        CHECK(EventoEntre(kSaves[k] - 0.01f, kSaves[k] + 0.01f) == Evento::Salvar);
        CHECK(EventoEntre(kLoads[k] - 0.01f, kLoads[k] + 0.01f) == Evento::Carregar);
        CHECK(EventoEntre(kSaves[k] + 0.01f, kSaves[k] + 0.02f) == Evento::Nada);
        CHECK(RumoAoLoad(kSaves[k]) == doctest::Approx(0.0f));
        CHECK(RumoAoLoad((kSaves[k] + kLoads[k]) / 2.0f) == doctest::Approx(0.5f));
        CHECK(RumoAoLoad(kLoads[k] + 0.01f) < 0.0f);
    }
    CHECK(RumoAoLoad(0.0f) < 0.0f);
    // Um evento por quadro: quadro a quadro, cada save e cada load aparece uma vez so.
    int saves = 0, loads = 0;
    float antes = -1.0f;
    for (float t = 0.0f; t < kDuracoes[2]; t += 0.016f) {
        const Evento e = EventoEntre(antes, t);
        saves += e == Evento::Salvar ? 1 : 0;
        loads += e == Evento::Carregar ? 1 : 0;
        antes = t;
    }
    CHECK(saves == static_cast<int>(kSaves.size()));
    CHECK(loads == static_cast<int>(kLoads.size()));
}

// ---- O lancamento ----

TEST_CASE("Lancamento: a travada espera a escolha, e quem nao escolhe ve o jogo voltar acelerado") {
    using namespace Lancamento;
    CHECK(TravadaEm(0.0f) == -1);
    CHECK(TravadaEm(kTravadas[0] + 0.1f) == 0);
    CHECK(TravadaEm(kTravadas[0] + kEspera + 0.1f) == -1);
    CHECK(TravadaEm(kTravadas[1] + 0.1f) == 1);
    CHECK(kTravadas[1] > kTravadas[0] + kEspera);
    CHECK(kTravadas[1] + kEspera < kDuracoes[0]);
    CHECK(FatorDaVolta(Escolha::Esperar) == doctest::Approx(1.0f));
    CHECK(FatorDaVolta(Escolha::Fechar) == doctest::Approx(1.0f));
    CHECK(FatorDaVolta(Escolha::Nenhuma) > 1.3f);
    // O botao troca de lado: a memoria da primeira travada atrapalha na segunda.
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
    // Nunca passa da folga, e desce cada vez que bate.
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
    // Os lugares: a fileira de cima no centro do topo, as colunas espalhadas em volta dele.
    const Vector2 topo(600.0f, 150.0f);
    const Marcha parada;
    CHECK(LugarNaFrota(0, kColunasDaFrota / 2, topo, parada).x == doctest::Approx(600.0f));
    CHECK(LugarNaFrota(0, 0, topo, parada).x == doctest::Approx(600.0f - 4.0f * kEspacoX));
    CHECK(LugarNaFrota(2, 0, topo, parada).y == doctest::Approx(150.0f + 2.0f * kEspacoY));
    const Marcha andou{60.0f, 2};
    CHECK(LugarNaFrota(1, 3, topo, andou).x == doctest::Approx(LugarNaFrota(1, 3, topo, parada).x + 60.0f));
    CHECK(LugarNaFrota(1, 3, topo, andou).y == doctest::Approx(LugarNaFrota(1, 3, topo, parada).y + 2.0f * kDescidaDaFrota));
}

TEST_CASE("Lancamento: cada linha dos creditos tem um vao, fora da faixa do texto") {
    using namespace Lancamento;
    Variacao::Sorteio s(9u);
    const int blocos = 42;
    bool esquerda = false, direita = false;
    for (int k = 0; k < 200; ++k) {
        const int v = VaoDaLinha(blocos, s);
        CHECK(v >= 1);
        CHECK(v + kVaoDosCreditos <= blocos - 1);
        // O vao inteiro fora do meio, onde o texto fica.
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
    // O vao cabe o aluno com folga.
    CHECK(static_cast<float>(kVaoDosCreditos) * kBloco > 100.0f);
    std::set<std::string> linhas(kCreditos.begin(), kCreditos.end());
    CHECK(linhas.size() == kCreditos.size());
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).subida > DegrauDo(Dificuldade::Nivel::Normal).subida);
}
