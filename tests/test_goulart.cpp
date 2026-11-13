//
// O Goulart: o escalonador, a paginacao, o braco do disco e a nuvem.
//

#include "doctest.h"

#include <algorithm>
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
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).chuva > DegrauDo(Dificuldade::Nivel::Normal).chuva);
}

TEST_CASE("Escalonador: cada processo chove na faixa dele, e o de tres threads em tres") {
    using namespace Escalonador;
    CHECK(FaixaDo(1) == 0);
    CHECK(FaixaDo(kFaixas) == kFaixas - 1);
    CHECK(FaixaDo(kFaixas + 1) == 0);   // da a volta
    CHECK(FaixasQueRodam(Novo(3, Tipo::Cpu)) == std::vector<int>{2});
    CHECK(FaixasQueRodam(Novo(3, Tipo::Threads)) == std::vector<int>{1, 2, 3});
    CHECK(FaixasQueRodam(Novo(1, Tipo::Threads)) == std::vector<int>{0, 1});   // na borda, so as que existem
    CHECK(InicioDaFaixa(0, 100.0f, 1200.0f) == doctest::Approx(100.0f));
    CHECK(InicioDaFaixa(3, 100.0f, 1200.0f) == doctest::Approx(700.0f));
    CHECK(LarguraDaFaixa(1200.0f) == doctest::Approx(200.0f));
    // A chuva e densa o bastante para a faixa nao ter por onde passar.
    CHECK(kVelocidadeDaChuva * kEntreGotas < 30.0f);
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

TEST_CASE("Paginacao: o chao sao as paginas, e a memoria encolhe a cada etapa") {
    using namespace Paginacao;
    // Oito colunas num chao de 1200: a de cada x, e as pontas presas.
    CHECK(PaginaDoX(10.0f, 0.0f, 1200.0f) == 0);
    CHECK(PaginaDoX(160.0f, 0.0f, 1200.0f) == 1);
    CHECK(PaginaDoX(1199.0f, 0.0f, 1200.0f) == kPaginas - 1);
    CHECK(PaginaDoX(-50.0f, 0.0f, 1200.0f) == 0);
    CHECK(InicioDaPagina(2, 0.0f, 1200.0f) == doctest::Approx(300.0f));
    CHECK(LarguraDaPagina(1200.0f) == doctest::Approx(150.0f));
    for (int e = 0; e + 1 < 3; ++e) {
        CHECK(MoldurasNaEtapa(e + 1, Dificuldade::Nivel::Normal) <= MoldurasNaEtapa(e, Dificuldade::Nivel::Normal));
        CHECK(MoldurasNaEtapa(e, Dificuldade::Nivel::Dificil) <= MoldurasNaEtapa(e, Dificuldade::Nivel::Normal));
    }
    CHECK(MoldurasNaEtapa(2, Dificuldade::Nivel::Normal) < kPaginas);
    // Encolher despeja as paginas das molduras que sobram.
    Relogio r(4);
    for (const int p : {1, 2, 3, 4}) r.Referenciar(p);
    const std::vector<int> fora = r.Encolher(2);
    CHECK(fora == std::vector<int>{4, 3});
    CHECK(r.Molduras().size() == 2u);
    CHECK(r.MolduraDa(1) == 0);
    CHECK(r.MolduraDa(4) == -1);
    // Quem anda por mais paginas do que molduras faz o thrashing: em laco, toda leitura e falta.
    Relogio pequeno(3);
    int faltas = 0;
    for (int volta = 0; volta < 5; ++volta) {
        for (int p = 0; p < 5; ++p) if (!pequeno.Referenciar(p).acerto) ++faltas;
    }
    CHECK(faltas == 25);
    // E quem fica em poucas so falta na primeira vez.
    Relogio local(3);
    faltas = 0;
    for (int k = 0; k < 30; ++k) if (!local.Referenciar(k % 3).acerto) ++faltas;
    CHECK(faltas == 3);
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(19.9f) == 2);
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(kPassoDoPonteiro * 4.0f < kEntreLeituras + kDespejo);
}

TEST_CASE("Paginacao: o professor le as paginas dele, e a do aluno, com o R aceso, quase sempre escapa") {
    using namespace Paginacao;
    // Ele le qualquer pagina menos a do aluno, e cada vez mais depressa.
    for (int aluno = 0; aluno < kPaginas; ++aluno) {
        std::vector<bool> lidas(kPaginas, false);
        for (int s = 0; s <= kPaginas - 2; ++s) lidas[static_cast<size_t>(PaginaDoProfessor(s, aluno))] = true;
        CHECK_FALSE(lidas[static_cast<size_t>(aluno)]);
        CHECK(std::count(lidas.begin(), lidas.end(), true) == kPaginas - 1);
    }
    const Degrau normal = DegrauDo(Dificuldade::Nivel::Normal), dificil = DegrauDo(Dificuldade::Nivel::Dificil);
    for (int e = 0; e + 1 < 3; ++e) {
        CHECK(EntreLeiturasDoProfessor(normal, e + 1) < EntreLeiturasDoProfessor(normal, e));
        CHECK(EntreLeiturasDoProfessor(dificil, e) < EntreLeiturasDoProfessor(normal, e));
    }
    // O aluno le a pagina 0 sem parar, e o professor le as outras: elas se despejam entre si, e a do
    // aluno - que o relogio sempre acha com o R aceso - so cai uma vez.
    Relogio r(3);
    int despejos = 0, doAluno = 0;
    for (int k = 0; k < 70; ++k) {
        for (int i = 0; i < 3; ++i) r.Referenciar(0);
        const Referencia ref = r.Referenciar(PaginaDoProfessor((k * 3) % (kPaginas - 1), 0));
        if (ref.vitima >= 0) {
            ++despejos;
            if (ref.vitima == 0) ++doAluno;
        }
    }
    CHECK(despejos > 50);
    CHECK(doAluno <= 1);
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
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).queda > DegrauDo(Dificuldade::Nivel::Normal).queda);
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
    // O tiro do kernel quica dentro da caixa: volta para dentro, e a velocidade vira.
    Vector2 pos(200.0f, 50.0f), vel(100.0f, -40.0f);
    Quicar(pos, vel, Vector2(100.0f, 50.0f), 170.0f, 104.0f, 8.0f);
    CHECK(pos.x == doctest::Approx(100.0f + 85.0f - 8.0f));
    CHECK(vel.x < 0.0f);
    CHECK(vel.y == doctest::Approx(-40.0f));   // no outro eixo, nada muda
    Vector2 dentro(110.0f, 60.0f), v2(10.0f, 10.0f);
    Quicar(dentro, v2, Vector2(100.0f, 50.0f), 170.0f, 104.0f, 8.0f);
    CHECK(dentro.x == doctest::Approx(110.0f));
    CHECK(v2.x == doctest::Approx(10.0f));
    // As caixas das duas fileiras nao se encostam.
    CHECK(Host(3, 0.0f, 0.0f, 1200.0f, 746.0f).y - Host(0, 0.0f, 0.0f, 1200.0f, 746.0f).y > kAlturaDaVm);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).maisVms > DegrauDo(Dificuldade::Nivel::Normal).maisVms);
}
