//
// A fase 3 do Vitor: o fork e a troca de mensagens.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <utility>
#include <vector>

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
            if (e.o == Acontece::Envia || e.o == Acontece::Responde) CHECK(e.quando + e.voo < kPorRodada);
        }
    }
}

TEST_CASE("Mensagens: na assincrona a caixa do cliente enche e esvazia, e ele nunca recebe do vazio") {
    const auto r = Roteiro(Rodada::Assincrona);
    int maior = 0;
    for (const Evento& e : r) {
        if (e.o != Acontece::Recebe) continue;
        // Antes de receber ha pelo menos um na caixa.
        CHECK(NaCaixa(r, kCliente, e.quando - 0.001f) >= 1);
        maior = std::max(maior, NaCaixa(r, kCliente, e.quando - 0.001f));
    }
    CHECK(maior >= 3);                                 // ela enche: o servidor nao espera
    CHECK(NaCaixa(r, kCliente, kPorRodada) == 0);          // e esvazia
    CHECK(Placar(Rodada::Assincrona, 3, 0, false, false) == "send sem esperar \xc2\xb7 caixa do cliente: 3");
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
    CHECK(Placar(Rodada::Sincrona, 0, 0, true, false) == "send s\xc3\xadncrono \xc2\xb7 cliente bloqueado");
}

TEST_CASE("Mensagens: na rpc o pedido vai, a resposta volta, e o resultado e a soma") {
    const auto r = Roteiro(Rodada::Rpc);
    int chamadas = 0;
    float ultimoResultado = -1.0f;
    for (const Evento& e : r) {
        if (e.o == Acontece::Envia) {
            CHECK(e.de == kCliente);
            CHECK(e.quando > ultimoResultado);   // a rpc bloqueia: so chama de novo depois do resultado
        }
        if (e.o == Acontece::Responde) CHECK(e.de == kServidor);
        if (e.o == Acontece::Resultado) {
            CHECK(e.valor == Soma(ChamadaDaVez(chamadas)));
            ++chamadas;
            ultimoResultado = e.quando;
        }
    }
    CHECK(chamadas == kChamadas);
    CHECK(Soma(ChamadaDaVez(0)) == 5);
    CHECK(Placar(Rodada::Rpc, 0, 0, true, false) == "r = soma(2, 3) \xc2\xb7 rpc no servidor");
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

TEST_CASE("Mensagens: os dois processos no alto, o cliente a esquerda, e o arco desce ate o fundo") {
    for (int k = 0; k < kRodadas; ++k) {
        CAPTURE(k);
        const Ponto cliente = Lugar(kCliente, k, 1200.0f);
        const Ponto servidor = Lugar(kServidor, k, 1200.0f);
        CHECK(cliente.x < servidor.x);
        for (const Ponto& p : {cliente, servidor}) {
            CHECK(p.y >= 120.0f - 0.01f);   // abaixo do placar, com o cartao de 128
            CHECK(p.y < 260.0f);
            CHECK(p.x > 64.0f);
            CHECK(p.x < 1200.0f - 64.0f);
        }
        const float fundo = Fundo(RodadaDaVez(k));
        const Ponto meio = NoArco(cliente, servidor, 0.5f, fundo);
        CHECK(meio.y == doctest::Approx(fundo));
        // Nas rodadas de muitos envelopes o canal vai ate o chao.
        const Rodada r = RodadaDaVez(k);
        if (r == Rodada::Assincrona || r == Rodada::Impasse) CHECK(fundo >= 698.0f);
        else CHECK(fundo < 680.0f);
        const Ponto a = NoArco(cliente, servidor, 0.0f, fundo);
        const Ponto b = NoArco(cliente, servidor, 1.0f, fundo);
        CHECK(a.x == doctest::Approx(cliente.x));
        CHECK(a.y == doctest::Approx(cliente.y));
        CHECK(b.x == doctest::Approx(servidor.x));
        CHECK(b.y == doctest::Approx(servidor.y));
    }
    CHECK(Nome(kCliente) == "cliente");
    CHECK(Nome(kServidor) == "servidor");
}

// ---- A variacao e o degrau ----

namespace {
    /// Confere num roteiro as regras de todas as rodadas.
    void ConferirRoteiro(const Rodada r, const std::vector<Evento>& roteiro, const Plano& p, const Degrau& d) {
        CAPTURE(static_cast<int>(r));
        REQUIRE_FALSE(roteiro.empty());
        for (size_t i = 1; i < roteiro.size(); ++i) CHECK(roteiro[i - 1].quando <= roteiro[i].quando);
        float estoura = -1.0f;
        int maior = 0, resultados = 0;
        for (const Evento& e : roteiro) {
            CHECK(e.quando < kPorRodada);
            if (e.o == Acontece::Envia || e.o == Acontece::Responde) CHECK(e.quando + e.voo < kPorRodada);
            if (e.o == Acontece::Recebe) {
                CHECK(NaCaixa(roteiro, e.de, e.quando - 0.001f) >= 1);
                maior = std::max(maior, NaCaixa(roteiro, e.de, e.quando - 0.001f));
            }
            if (e.o == Acontece::Estoura) estoura = e.quando;
            if (e.o == Acontece::Resultado) {
                CHECK(e.valor == Soma(p.chamadas[static_cast<size_t>(resultados)]));
                CHECK(e.valor % 2 == 1);   // o leque tem um tiro no meio
                ++resultados;
            }
        }
        // Quem recebe com receive() esvazia a caixa: o cliente na assincrona, os dois depois do timeout.
        if (r == Rodada::Assincrona) CHECK(NaCaixa(roteiro, kCliente, kPorRodada) == 0);
        if (r == Rodada::Impasse) {
            CHECK(NaCaixa(roteiro, kCliente, kPorRodada) == 0);
            CHECK(NaCaixa(roteiro, kServidor, kPorRodada) == 0);
        }
        if (r == Rodada::Assincrona) CHECK(maior >= 3);
        if (r == Rodada::Rpc) CHECK(resultados == kChamadas);
        if (r == Rodada::Sincrona) {
            // Cada envio tem o seu rendezvous, quando o envelope chega.
            int envios = 0, encontros = 0;
            for (const Evento& e : roteiro) {
                if (e.o == Acontece::Encontro) ++encontros;
                if (e.o != Acontece::Envia) continue;
                ++envios;
                bool achou = false;
                for (const Evento& f : roteiro) {
                    if (f.o == Acontece::Encontro && std::fabs(f.quando - (e.quando + e.voo)) < 0.001f) achou = true;
                }
                CHECK(achou);
            }
            CHECK(envios == d.sincronos);
            CHECK(encontros == d.sincronos);
            // O cliente bloqueado so manda de novo depois do rendezvous.
            float ultimoEncontro = -1.0f;
            for (const Evento& e : roteiro) {
                if (e.o == Acontece::Envia) CHECK(e.quando > ultimoEncontro);
                if (e.o == Acontece::Encontro) ultimoEncontro = e.quando;
            }
        }
        if (r == Rodada::Rpc) {
            // O servidor responde depois de o pedido chegar, e o resultado e a resposta chegando.
            float chegouOPedido = -1.0f;
            float chegaAResposta = -1.0f;
            float ultimoResultado = -1.0f;
            for (const Evento& e : roteiro) {
                if (e.o == Acontece::Envia) {
                    CHECK(e.quando > ultimoResultado);
                    chegouOPedido = e.quando + e.voo;
                }
                if (e.o == Acontece::Responde) {
                    CHECK(e.quando >= chegouOPedido - 0.001f);
                    chegaAResposta = e.quando + e.voo;
                }
                if (e.o == Acontece::Resultado) {
                    CHECK(e.quando == doctest::Approx(chegaAResposta));
                    ultimoResultado = e.quando;
                }
            }
        }
        if (r == Rodada::Impasse) {
            REQUIRE(estoura > 0.0f);
            for (const Evento& e : roteiro) if (e.o == Acontece::Recebe) CHECK(e.quando > estoura);
            for (const Evento& e : roteiro) {
                if (e.o == Acontece::Envia && e.paraNaPorta) CHECK(e.quando + e.voo * kNaPorta < estoura);
            }
        }
    }

    void ConferirRodada(const Rodada r, const Plano& p, const Degrau& d) {
        ConferirRoteiro(r, Roteiro(r, p, d), p, d);
    }
}

TEST_CASE("Mensagens: o plano de sempre e o roteiro de sempre") {
    const Plano p = PlanoDeSempre();
    const Degrau d = DegrauDo(Dificuldade::Nivel::Normal);
    for (int k = 0; k < kRodadas; ++k) {
        CHECK(p.rodadas[static_cast<size_t>(k)] == RodadaDaVez(k));
        const auto velho = Roteiro(RodadaDaVez(k));
        const auto novo = Roteiro(RodadaDaVez(k), p, d);
        REQUIRE(velho.size() == novo.size());
        for (size_t i = 0; i < velho.size(); ++i) {
            CHECK(velho[i].quando == doctest::Approx(novo[i].quando));
            CHECK(velho[i].o == novo[i].o);
        }
        for (const int q : {kCliente, kServidor}) {
            CHECK(Lugar(q, k, 1200.0f, p).x == doctest::Approx(Lugar(q, k, 1200.0f).x));
            CHECK(Lugar(q, k, 1200.0f, p).y == doctest::Approx(Lugar(q, k, 1200.0f).y));
        }
    }
    CHECK(d.anelDoEncontro == kAnelDoEncontro);
    CHECK(d.rapidez == doctest::Approx(1.0f));
}

TEST_CASE("Mensagens: em toda luta a assincrona abre, as contas sao outras, e as regras valem nos dois modos") {
    std::set<std::vector<int>> ordens;
    std::set<std::pair<int, int>> contas;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        CAPTURE(semente);
        Variacao::Sorteio s(semente);
        const Plano p = Sortear(s);
        CHECK(p.rodadas[0] == Rodada::Assincrona);
        std::vector<int> tipos;
        for (const Rodada r : p.rodadas) tipos.push_back(static_cast<int>(r));
        ordens.insert(tipos);
        std::vector<int> ordenados = tipos;
        std::sort(ordenados.begin(), ordenados.end());
        CHECK(ordenados == std::vector<int>{0, 1, 2, 3});

        CHECK(Soma(p.chamadas[0]) != Soma(p.chamadas[1]));
        for (const Chamada& c : p.chamadas) {
            CHECK(c.a >= 1);
            CHECK(c.b >= 1);
            contas.insert({c.a, c.b});
        }

        for (int k = 0; k < kRodadas; ++k) {
            const Ponto cliente = Lugar(kCliente, k, 1200.0f, p);
            const Ponto servidor = Lugar(kServidor, k, 1200.0f, p);
            CHECK(cliente.x < servidor.x);
            for (const Ponto& q : {cliente, servidor}) {
                CHECK(q.y >= 120.0f - 0.01f);
                CHECK(q.y < 260.0f);
                CHECK(q.x > 64.0f);
                CHECK(q.x < 1200.0f - 64.0f);
            }
        }
        for (const Dificuldade::Nivel n : Dificuldade::kNiveis) {
            for (const Rodada r : p.rodadas) ConferirRodada(r, p, DegrauDo(n));
        }
    }
    CHECK(ordens.size() == 6u);
    CHECK(contas.size() > 10u);
    // O placar da rpc fala a conta desta luta.
    Plano p = PlanoDeSempre();
    p.chamadas = {Chamada{4, 5}, Chamada{1, 6}};
    CHECK(Placar(Rodada::Rpc, 0, 0, true, false, p) == "r = soma(4, 5) \xc2\xb7 rpc no servidor");
    CHECK(Placar(Rodada::Rpc, 0, 1, false, false, p) == "return 7");
}

TEST_CASE("Mensagens: com a latencia sorteada as regras valem, e os envelopes chegam fora de ordem") {
    bool foraDeOrdem = false;
    bool encolheu = false;
    std::set<int> encontros;
    std::set<int> respostas;
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        CAPTURE(semente);
        Variacao::Sorteio sorteioDoPlano(semente);
        const Plano p = Sortear(sorteioDoPlano);
        for (const Dificuldade::Nivel n : Dificuldade::kNiveis) {
            const Degrau d = DegrauDo(n);
            Variacao::Sorteio s(semente * 7u + 1u);
            for (const Rodada r : p.rodadas) {
                const auto roteiro = Roteiro(r, p, d, &s);
                ConferirRoteiro(r, roteiro, p, d);
                float ultimaChegada = -1.0f;
                for (const Evento& e : roteiro) {
                    if (r == Rodada::Assincrona && e.o == Acontece::Envia) {
                        foraDeOrdem = foraDeOrdem || e.quando + e.voo < ultimaChegada;
                        ultimaChegada = std::max(ultimaChegada, e.quando + e.voo);
                    }
                    if (e.o == Acontece::Encontro) encontros.insert(static_cast<int>(e.quando * 1000.0f));
                    if (e.o == Acontece::Responde) respostas.insert(static_cast<int>(e.quando * 1000.0f));
                }
                const auto sempre = Roteiro(r, p, d);
                if (r == Rodada::Assincrona && roteiro.front().quando < sempre.front().quando - 0.001f) encolheu = true;
            }
        }
    }
    CHECK(foraDeOrdem);              // a assincrona entrega fora de ordem
    CHECK(encontros.size() > 50u);   // o rendezvous nao tem hora marcada
    CHECK(respostas.size() > 50u);   // nem o servidor da rpc
    CHECK_FALSE(encolheu);           // a assincrona cabe sem encolher: os envios saem na hora
    // Sem sorteio, o roteiro e o de sempre.
    const auto a = Roteiro(Rodada::Rpc);
    const auto b = Roteiro(Rodada::Rpc, PlanoDeSempre(), Degrau{}, nullptr);
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) CHECK(a[i].quando == doctest::Approx(b[i].quando));
}

TEST_CASE("Mensagens: o degrau - o Dificil enche mais a caixa e atira mais") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.assincronos > n.assincronos);
    CHECK(d.sincronos > n.sincronos);
    CHECK(n.lequeDoRecebido == 1);
    CHECK(d.lequeDoRecebido > 1);
    CHECK(d.lequeDoRecebido % 2 == 1);   // o do meio vai no aluno
    CHECK(d.depoisDoTimeout > n.depoisDoTimeout);
    CHECK(d.anelDoEncontro > n.anelDoEncontro);
    CHECK(d.rapidez > n.rapidez);
    // A caixa do cliente enche mais no Dificil.
    auto maiorCaixa = [](const Degrau& g) {
        const auto r = Roteiro(Rodada::Assincrona, PlanoDeSempre(), g);
        int maior = 0;
        for (float u = 0.0f; u < kPorRodada; u += 0.01f) maior = std::max(maior, NaCaixa(r, kCliente, u));
        return maior;
    };
    CHECK(maiorCaixa(d) > maiorCaixa(n));
}
