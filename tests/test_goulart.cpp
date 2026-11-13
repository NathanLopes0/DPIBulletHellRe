//
// O Goulart: o escalonador, a paginacao, o braco do disco e a nuvem.
//

#include "doctest.h"

#include <set>
#include <string>
#include <vector>

#include "../Source/Attacks/Disco.h"
#include "../Source/Attacks/Escalonador.h"
#include "../Source/Attacks/Paginacao.h"
#include "../Source/Attacks/Virtualizacao.h"

// ---- O escalonador ----

namespace {
    /// Anda o escalonador de 'de' ate 'ate' em passos de quadro, juntando os avisos.
    std::vector<Escalonador::Aviso> Rodar(Escalonador::RoundRobin& rr, float de, const float ate) {
        std::vector<Escalonador::Aviso> todos;
        for (float t = de; t <= ate + 1e-4f; t += 0.016f) {
            for (const auto& a : rr.Avancar(t)) todos.push_back(a);
        }
        return todos;
    }
}

TEST_CASE("Escalonador: o Round Robin reveza pelo quantum, e o fim da fila volta a CPU") {
    using namespace Escalonador;
    RoundRobin rr(1.0f, 0.1f);
    rr.Chegar(Novo(1, Tipo::Cpu));
    rr.Chegar(Novo(2, Tipo::Cpu));
    rr.Chegar(Novo(3, Tipo::Cpu));
    const auto avisos = Rodar(rr, 0.016f, 3.5f);
    std::vector<int> despachos;
    for (const auto& a : avisos) if (a.evento == Evento::Despachou) despachos.push_back(a.pid);
    REQUIRE(despachos.size() >= 4u);
    CHECK(despachos[0] == 1);
    CHECK(despachos[1] == 2);
    CHECK(despachos[2] == 3);
    CHECK(despachos[3] == 1);   // o primeiro voltou para o fim e chegou a vez dele de novo
    int preempcoes = 0;
    for (const auto& a : avisos) if (a.evento == Evento::Preemptou) ++preempcoes;
    CHECK(preempcoes >= 3);
}

TEST_CASE("Escalonador: o processo de E/S roda pouco, espera o disco e volta pela interrupcao") {
    using namespace Escalonador;
    RoundRobin rr(1.0f, 0.1f);
    rr.Chegar(Novo(1, Tipo::EntradaSaida));
    rr.Chegar(Novo(2, Tipo::Cpu));
    const auto avisos = Rodar(rr, 0.016f, 0.6f);
    bool pediu = false;
    for (const auto& a : avisos) if (a.evento == Evento::PediuES && a.pid == 1) pediu = true;
    CHECK(pediu);
    REQUIRE(rr.Rodando().has_value());
    CHECK(*rr.Rodando() == 2);   // a CPU nao ficou parada esperando o disco
    CHECK(rr.Bloqueados() == std::vector<int>{1});
    const auto depois = Rodar(rr, 0.62f, 0.6f + kTempoDeES + 0.2f);
    bool acordou = false;
    for (const auto& a : depois) if (a.evento == Evento::Acordou && a.pid == 1) acordou = true;
    CHECK(acordou);
    CHECK(rr.Bloqueados().empty());
}

TEST_CASE("Escalonador: quem acaba a CPU termina, kill -9 tira de onde estiver, e o placar conta") {
    using namespace Escalonador;
    RoundRobin rr(5.0f, 0.0f);
    Processo curto = Novo(7, Tipo::Cpu);
    curto.falta = 0.3f;
    rr.Chegar(curto);
    rr.Chegar(Novo(8, Tipo::Cpu));
    bool terminou = false;
    for (const auto& a : Rodar(rr, 0.016f, 0.5f)) if (a.evento == Evento::Terminou && a.pid == 7) terminou = true;
    CHECK(terminou);
    CHECK(rr.Achar(7) == nullptr);
    CHECK(rr.Quantos() == 1);
    CHECK(rr.Matar(8));
    CHECK_FALSE(rr.Matar(8));
    CHECK(rr.Quantos() == 0);
    CHECK_FALSE(rr.Rodando().has_value());

    RoundRobin outro(1.2f);
    outro.Chegar(Novo(1, Tipo::Cpu));
    outro.Chegar(Novo(2, Tipo::Threads));
    outro.Avancar(0.5f);
    CHECK(Placar(outro) == "Round Robin, q = 1,2 s  \xc2\xb7  CPU: P1  \xc2\xb7  prontos: P2  \xc2\xb7  E/S: -");
    CHECK(Cara(10) == "P1");
    CHECK(TipoDoFork(0) == Tipo::Cpu);
    CHECK(TipoDoFork(4) == Tipo::EntradaSaida);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).quantum < DegrauDo(Dificuldade::Nivel::Normal).quantum);
}

// ---- A paginacao ----

TEST_CASE("Paginacao: a MMU parte o endereco em pagina e deslocamento") {
    using namespace Paginacao;
    const Endereco e = Traduzir(0x3A7F);
    CHECK(e.pagina == 3);
    CHECK(e.deslocamento == 0xA7F);
    CHECK(Montar(5, 0xA07) == 0x5A07u);
    CHECK(Hexa(0x3A7F, 4) == "0x3A7F");
    CHECK(PlacarDoAcerto(0x3A7F, 2) == "0x3A7F = p\xc3\xa1gina 3 + 0xA7F  \xc2\xb7  acerto na moldura 2");
    CHECK(PlacarDaFalta(0x5A07, 3, 1) == "0x5A07 = p\xc3\xa1gina 5 + 0xA07  \xc2\xb7  falta! sai a p\xc3\xa1gina 3");
    CHECK(PlacarDaFalta(0x5A07, -1, 1).find("vai para a moldura 1") != std::string::npos);
    CHECK(PlacarDoThrashing(8, 10) == "Thrashing: 8 faltas nas \xc3\xbaltimas 10 refer\xc3\xaancias");
    // O maior endereco ainda cabe em quatro algarismos.
    CHECK(Montar(kPaginas - 1, (1 << kBitsDoDeslocamento) - 1) <= 0xFFFFu);
}

TEST_CASE("Paginacao: o relogio da a segunda chance a quem tem R, e despeja o primeiro sem") {
    using namespace Paginacao;
    Relogio r(3);
    for (const int p : {1, 2, 3}) {
        const Referencia ref = r.Referenciar(p);
        CHECK_FALSE(ref.acerto);
        CHECK(ref.vitima == -1);   // ainda ha moldura livre
    }
    // Todas com R ligado: o ponteiro poupa as tres (apaga os R) e despeja a primeira.
    Referencia ref = r.Referenciar(4);
    CHECK(ref.segundaChance == std::vector<int>{0, 1, 2});
    CHECK(ref.vitima == 1);
    CHECK(ref.moldura == 0);
    // A 2 e lida de novo: acerto, e o R dela acende.
    ref = r.Referenciar(2);
    CHECK(ref.acerto);
    CHECK(ref.moldura == 1);
    // A 5: o ponteiro (na moldura 1) poupa a 2 e despeja a 3, que tinha o R apagado.
    ref = r.Referenciar(5);
    CHECK(ref.segundaChance == std::vector<int>{1});
    CHECK(ref.vitima == 3);
    CHECK(r.MolduraDa(5) == 2);
    CHECK(r.MolduraDa(3) == -1);
}

TEST_CASE("Paginacao: a localidade cabe na memoria, e o laco grande demais e o thrashing") {
    using namespace Paginacao;
    const int molduras = 5;
    const auto local = ConjuntoDeTrabalho(0, molduras, 2);
    const auto mudanca = ConjuntoDeTrabalho(1, molduras, 2);
    const auto thrashing = ConjuntoDeTrabalho(2, molduras, 2);
    CHECK(static_cast<int>(local.size()) < molduras);
    CHECK(local != mudanca);
    CHECK(static_cast<int>(thrashing.size()) > molduras);
    for (const int p : thrashing) CHECK((p >= 0 && p < kPaginas));

    // No laco que nao cabe, o relogio erra toda vez; na localidade, quase nunca.
    Variacao::Sorteio s(7u);
    Relogio r(molduras);
    int cursor = -1, faltasNoLaco = 0;
    for (int i = 0; i < 40; ++i) {
        const int p = ProximaReferencia(s, thrashing, 0.0f, cursor, true);
        if (!r.Referenciar(p).acerto && i >= molduras) ++faltasNoLaco;
    }
    CHECK(faltasNoLaco >= 30);
    Relogio r2(molduras);
    int faltasNaLocalidade = 0;
    for (int i = 0; i < 40; ++i) if (!r2.Referenciar(ProximaReferencia(s, local, 0.0f, cursor, false)).acerto) ++faltasNaLocalidade;
    CHECK(faltasNaLocalidade <= static_cast<int>(local.size()));
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(19.9f) == 2);
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(EntreReferencias(2, Dificuldade::Nivel::Normal) < EntreReferencias(0, Dificuldade::Nivel::Normal));
}

// ---- O braco do disco ----

TEST_CASE("Disco: os tres algoritmos no exemplo do livro (cabeca em 53)") {
    using namespace Disco;
    const std::vector<int> pedidos = {98, 183, 37, 122, 14, 124, 65, 67};
    CHECK(Movimento(53, pedidos, Algoritmo::FCFS) == 640);
    CHECK(Movimento(53, pedidos, Algoritmo::SSTF) == 236);
    CHECK(Movimento(53, pedidos, Algoritmo::SCAN, -1) == 236);   // para o 0, e volta ate o 183
    int sentido = -1;
    CHECK(Alvo(53, pedidos, Algoritmo::FCFS, sentido) == 98);
    CHECK(Alvo(53, pedidos, Algoritmo::SSTF, sentido) == 65);
    CHECK(Alvo(53, pedidos, Algoritmo::SCAN, sentido) == 37);
    // Sem nada adiante, o elevador vai ate a ponta, e la vira.
    sentido = -1;
    CHECK(Alvo(10, {60, 90}, Algoritmo::SCAN, sentido) == 0);
    CHECK(Alvo(0, {60, 90}, Algoritmo::SCAN, sentido) == 60);
    CHECK(sentido == 1);
    CHECK(Alvo(5, {}, Algoritmo::SSTF, sentido) == -1);
}

TEST_CASE("Disco: a trilha, as etapas e o placar") {
    using namespace Disco;
    CHECK(XDoCilindro(0, 100.0f, 990.0f) == doctest::Approx(100.0f));
    CHECK(XDoCilindro(kCilindros - 1, 100.0f, 990.0f) == doctest::Approx(1090.0f));
    CHECK(CilindroDoX(XDoCilindro(73, 100.0f, 990.0f), 100.0f, 990.0f) == 73);
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(kDuracoes[0] + 0.1f) == 1);
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(Placar(Algoritmo::FCFS, 640, 8, 640) == "FCFS: o bra\xc3\xa7o andou 640 cilindros em 8 pedidos");
    CHECK(Placar(Algoritmo::SCAN, 236, 8, 640) == "SCAN: 236 cilindros em 8 pedidos  \xc2\xb7  em FCFS seriam 640");
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).entrePedidos < DegrauDo(Dificuldade::Nivel::Normal).entrePedidos);
}

// ---- A nuvem ----

TEST_CASE("Virtualizacao: os hosts, a elasticidade e a migracao") {
    using namespace Virtualizacao;
    std::set<std::pair<int, int>> lugares;
    for (int k = 0; k < kHosts; ++k) {
        const Vector2 h = Host(k, 0.0f, 0.0f, 1200.0f, 746.0f);
        lugares.insert({static_cast<int>(h.x), static_cast<int>(h.y)});
        // As caixas cabem no campo.
        CHECK(h.x - kLarguraDaVm / 2.0f > 0.0f);
        CHECK(h.x + kLarguraDaVm / 2.0f < 1200.0f);
    }
    CHECK(lugares.size() == static_cast<size_t>(kHosts));
    CHECK(NomeDoHost(0) == 'A');
    CHECK(NomeDoHost(5) == 'F');
    CHECK(VmsNoTempo(1.0f, 5) == 2);
    CHECK(VmsNoTempo(kDuracoes[0] + 1.0f, 5) == 3);
    CHECK(VmsNoTempo(19.9f, 5) == 5);
    CHECK(VmsNoTempo(19.9f, 9) <= kHosts);
    CHECK(HostLivre({true, false, true}, 0) == 1);
    CHECK(HostLivre({true, true, true}, 0) == -1);
    const Vector2 a(0.0f, 0.0f), b(100.0f, 0.0f);
    CHECK(NaMigracao(a, b, 0.0f).x == doctest::Approx(0.0f));
    CHECK(NaMigracao(a, b, 0.5f).x == doctest::Approx(50.0f));
    CHECK(NaMigracao(a, b, 2.0f).x == doctest::Approx(100.0f));
    CHECK(NaMigracao(a, b, 0.1f).x < 10.0f);   // devagar no comeco
    CHECK(PlacarDaMigracao(2, 0, 4) == "Migra\xc3\xa7\xc3\xa3o ao vivo: VM 2, do host A para o E");
    CHECK(PlacarDaEscala(3, 4) == "Carga subindo: 3 \xe2\x86\x92 4 VMs");
    CHECK(Placar(3) == "Hipervisor: 3 VMs em 6 hosts");
    CHECK(kVooDoTrap < kEntreTraps);
    CHECK(kMigracao < kEntreMigracoes);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).maisVms > DegrauDo(Dificuldade::Nivel::Normal).maisVms);
}
