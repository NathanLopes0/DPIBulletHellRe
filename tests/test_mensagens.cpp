//
// A fase 3 do Vitor: o fork e a troca de mensagens.
//

#include "doctest.h"

#include <cmath>

#include "../Source/Attacks/Mensagens.h"

using namespace Mensagens;

TEST_CASE("Mensagens: as rodadas cabem na fase, e comecam depois do fork") {
    CHECK(kComeca + static_cast<float>(kRodadas) * kPorRodada == doctest::Approx(Fase::kDuracao));
    CHECK(kComeca >= 1.6f);   // Nuvem::Fork acaba em 1,6 s
    CHECK(RodadaDaVez(kRodadas) == RodadaDaVez(0));
    for (int k = 0; k < kRodadas; ++k) {
        CAPTURE(k);
        for (const Evento& e : Roteiro(RodadaDaVez(k))) {
            CHECK(e.quando < kPorRodada);
            // TODO ENVELOPE CHEGA ANTES DE A RODADA ACABAR.
            if (e.o == Acontece::Envia || e.o == Acontece::Responde) CHECK(e.quando + e.voo < kPorRodada);
        }
    }
}

TEST_CASE("Mensagens: na assincrona a caixa do pai enche e esvazia, e ele nunca recebe do vazio") {
    const auto r = Roteiro(Rodada::Assincrona);
    int maior = 0;
    for (const Evento& e : r) {
        if (e.o != Acontece::Recebe) continue;
        // Antes de receber ha pelo menos um na caixa.
        CHECK(NaCaixa(r, kPai, e.quando - 0.001f) >= 1);
        maior = std::max(maior, NaCaixa(r, kPai, e.quando - 0.001f));
    }
    CHECK(maior >= 3);                                 // ela enche: o filho nao espera
    CHECK(NaCaixa(r, kPai, kPorRodada) == 0);          // e esvazia
    CHECK(Placar(Rodada::Assincrona, 3, 0, false, false) == "send sem esperar \xc2\xb7 caixa do pai: 3");
}

TEST_CASE("Mensagens: na sincrona cada envio tem o seu rendezvous, quando o envelope chega") {
    const auto r = Roteiro(Rodada::Sincrona);
    int envios = 0, encontros = 0;
    for (const Evento& e : r) {
        if (e.o == Acontece::Envia) {
            ++envios;
            bool achou = false;
            for (const Evento& f : r) {
                if (f.o == Acontece::Encontro && std::fabs(f.quando - (e.quando + e.voo)) < 0.001f) achou = true;
            }
            CHECK(achou);
        }
        if (e.o == Acontece::Encontro) ++encontros;
    }
    CHECK(envios == 3);
    CHECK(encontros == 3);
    CHECK(Placar(Rodada::Sincrona, 0, 0, true, false) == "send s\xc3\xadncrono \xc2\xb7 pai bloqueado");
}

TEST_CASE("Mensagens: na rpc o pedido vai, a resposta volta, e o resultado e a soma") {
    const auto r = Roteiro(Rodada::Rpc);
    int chamadas = 0;
    float ultimoResultado = -1.0f;
    for (const Evento& e : r) {
        if (e.o == Acontece::Envia) {
            CHECK(e.de == kPai);
            CHECK(e.quando > ultimoResultado);   // a rpc bloqueia: so chama de novo depois do resultado
        }
        if (e.o == Acontece::Responde) CHECK(e.de == kFilho);
        if (e.o == Acontece::Resultado) {
            CHECK(e.valor == Soma(ChamadaDaVez(chamadas)));
            ++chamadas;
            ultimoResultado = e.quando;
        }
    }
    CHECK(chamadas == kChamadas);
    CHECK(Soma(ChamadaDaVez(0)) == 5);
    CHECK(Placar(Rodada::Rpc, 0, 0, true, false) == "r = soma(2, 3) \xc2\xb7 rpc no filho");
    CHECK(Placar(Rodada::Rpc, 0, 1, false, false) == "return 7");
}

TEST_CASE("Mensagens: no impasse os dois mandam e ninguem recebe, ate o timeout") {
    const auto r = Roteiro(Rodada::Impasse);
    int naPorta = 0;
    float estoura = -1.0f;
    for (const Evento& e : r) {
        if (e.o == Acontece::Envia && e.paraNaPorta) {
            ++naPorta;
            CHECK(e.quando + e.voo * kNaPorta < 2.3f);   // chegam a porta antes do timeout
        }
        if (e.o == Acontece::Estoura) estoura = e.quando;
    }
    CHECK(naPorta == 2);
    REQUIRE(estoura > 0.0f);
    // Nenhum receive antes do timeout.
    for (const Evento& e : r) if (e.o == Acontece::Recebe) CHECK(e.quando > estoura);
    CHECK(Placar(Rodada::Impasse, 0, 0, true, false) == "send() e send() \xc2\xb7 deadlock");
    CHECK(Placar(Rodada::Impasse, 0, 0, false, true) == "timeout \xc2\xb7 send sem esperar");
}

TEST_CASE("Mensagens: os dois processos no alto, o pai a esquerda, e o arco desce ate o fundo") {
    for (int k = 0; k < kRodadas; ++k) {
        CAPTURE(k);
        const Ponto pai = Lugar(kPai, k, 1200.0f);
        const Ponto filho = Lugar(kFilho, k, 1200.0f);
        CHECK(pai.x < filho.x);
        for (const Ponto& p : {pai, filho}) {
            CHECK(p.y >= 120.0f - 0.01f);   // abaixo do placar, com o cartao de 128
            CHECK(p.y < 260.0f);
            CHECK(p.x > 64.0f);
            CHECK(p.x < 1200.0f - 64.0f);
        }
        const float fundo = Fundo(RodadaDaVez(k));
        const Ponto meio = NoArco(pai, filho, 0.5f, fundo);
        CHECK(meio.y == doctest::Approx(fundo));
        // NAS RODADAS DE MUITOS ENVELOPES o canal vai ate o chao; nas outras o
        // aluno embaixo passa por baixo dele.
        const Rodada r = RodadaDaVez(k);
        if (r == Rodada::Assincrona || r == Rodada::Impasse) CHECK(fundo >= 698.0f);
        else CHECK(fundo < 680.0f);
        const Ponto a = NoArco(pai, filho, 0.0f, fundo);
        const Ponto b = NoArco(pai, filho, 1.0f, fundo);
        CHECK(a.x == doctest::Approx(pai.x));
        CHECK(a.y == doctest::Approx(pai.y));
        CHECK(b.x == doctest::Approx(filho.x));
        CHECK(b.y == doctest::Approx(filho.y));
    }
    CHECK(Nome(kPai) == "pai");
    CHECK(Nome(kFilho) == "filho");
}
