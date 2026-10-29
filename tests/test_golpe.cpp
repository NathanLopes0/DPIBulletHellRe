//
// O golpe do orientador no TCC: o da materia que ele da.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <string>

#include "../Source/Gameplay/Orientador/Golpe.h"

using namespace Golpe;

namespace {
    const Materia kTodas[] = {Materia::Generico, Materia::Andre, Materia::Salles, Materia::Ricardo,
                              Materia::Thiago, Materia::Luizinho, Materia::Vladimir, Materia::Julio,
                              Materia::MariaLucia, Materia::Lucas, Materia::Vitor};

    /// Quantos tiros chegam a voar de um: a gramatica divide.
    int Finais(const Tiro& t) {
        return t.divide > 0.0f ? (2 << t.geracoes) : 1;
    }
}

TEST_CASE("Golpe: cada professor de materias.json tem o golpe dele") {
    CHECK(DoProfessor("Andr\xc3\xa9") == Materia::Andre);
    CHECK(DoProfessor("Salles") == Materia::Salles);
    CHECK(DoProfessor("Ricardo") == Materia::Ricardo);
    CHECK(DoProfessor("Thiago") == Materia::Thiago);
    CHECK(DoProfessor("Luizinho") == Materia::Luizinho);
    CHECK(DoProfessor("Vladimir") == Materia::Vladimir);
    CHECK(DoProfessor("J\xc3\xbalio") == Materia::Julio);
    CHECK(DoProfessor("Maria L\xc3\xba" "cia") == Materia::MariaLucia);
    CHECK(DoProfessor("Lucas") == Materia::Lucas);
    CHECK(DoProfessor("Vitor") == Materia::Vitor);
    CHECK(DoProfessor("Hugo") == Materia::Generico);
    CHECK(DoProfessor("") == Materia::Generico);
}

TEST_CASE("Golpe: todos tem o mesmo peso - uns dois tiros por segundo - e cabem na rodada") {
    for (const Materia m : kTodas) {
        if (m == Materia::MariaLucia) continue;   // quem dispara e o aluno
        CAPTURE(static_cast<int>(m));
        Variacao::Sorteio s = Variacao::SorteioDa(7u, "orientador");
        float tempo = 0.0f;
        int tiros = 0;
        for (int k = 0; k < 40; ++k) {
            const Rodada r = DaVez(m, k, s);
            REQUIRE(r.duracao > 0.0f);
            tempo += r.duracao;
            // EM ORDEM DE HORA: o orientador solta o proximo quando a hora dele chega.
            for (size_t i = 1; i < r.tiros.size(); ++i) CHECK(r.tiros[i].quando >= r.tiros[i - 1].quando);
            for (const Tiro& t : r.tiros) {
                tiros += Finais(t);
                // TUDO PARTE ANTES DA PROXIMA RODADA: nada se acumula.
                CHECK(t.quando + t.espera < r.duracao);
                CHECK(t.velocidade > 0.0f);
            }
        }
        const float porSegundo = static_cast<float>(tiros) / tempo;
        CHECK(porSegundo > 1.6f);
        CHECK(porSegundo < 2.4f);
    }
    // O OBSERVER: um a cada kNotificacoes tiros do aluno, que da um a cada
    // 0,12 s - uns dois por segundo tambem.
    CHECK(Notificacoes(3, 0) == 0);
    CHECK(Notificacoes(4, 0) == 1);
    CHECK(Notificacoes(9, 1) == 1);
    CHECK(Notificacoes(9, 2) == 0);
    CHECK(Notificacoes(8, 5) == 0);
    Variacao::Sorteio s = Variacao::SorteioDa(7u, "orientador");
    CHECK(DaVez(Materia::MariaLucia, 0, s).tiros.empty());
}

TEST_CASE("Golpe: a pilha desempilha do ultimo para o primeiro, e o COMMIT manda tudo junto") {
    Variacao::Sorteio s = Variacao::SorteioDa(1u, "orientador");
    const Rodada pilha = DaVez(Materia::Salles, 0, s);
    REQUIRE(pilha.tiros.size() == 4u);
    float ultimoPush = 0.0f;
    for (const Tiro& t : pilha.tiros) ultimoPush = std::max(ultimoPush, t.quando);
    for (size_t i = 0; i < pilha.tiros.size(); ++i) {
        const Tiro& t = pilha.tiros[i];
        CHECK(t.quando + t.espera > ultimoPush);   // o POP vem depois do ultimo PUSH
        if (i > 0) {
            const Tiro& antes = pilha.tiros[i - 1];
            CHECK(t.dy < antes.dy);                                       // em cima do anterior
            CHECK(t.quando + t.espera < antes.quando + antes.espera);     // e sai antes dele
        }
    }

    const Rodada transacao = DaVez(Materia::Thiago, 0, s);
    REQUIRE(transacao.tiros.size() >= 4u);
    const float commit = transacao.tiros.front().quando + transacao.tiros.front().espera;
    for (const Tiro& t : transacao.tiros) {
        CHECK(t.quando + t.espera == doctest::Approx(commit));
        CHECK(t.quando < commit);
    }
}

TEST_CASE("Golpe: o byte do barramento tem tres bits 1, paralelos, e muda a cada rodada") {
    std::set<std::string> bytes;
    for (std::uint32_t semente = 0; semente < 60; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, "orientador");
        const Rodada r = DaVez(Materia::Ricardo, 0, s);
        REQUIRE(r.tiros.size() == static_cast<size_t>(kBitsUm));
        std::string byte(8, '0');
        for (const Tiro& t : r.tiros) {
            CHECK(t.paralelo);
            const int fio = static_cast<int>(std::lround(t.dx / 12.0f + 3.5f));
            REQUIRE(fio >= 0);
            REQUIRE(fio < 8);
            byte[static_cast<size_t>(fio)] = '1';
        }
        CHECK(std::count(byte.begin(), byte.end(), '1') == kBitsUm);
        bytes.insert(byte);
    }
    CHECK(bytes.size() > 25u);
}

TEST_CASE("Golpe: a janela do TCP cresce, cai pela metade na perda, e recomeca") {
    CHECK(Janela(0) == 1);
    CHECK(Janela(1) == 2);
    CHECK(Janela(2) == 4);
    CHECK(Janela(3) == 5);
    CHECK(Janela(4) == 6);
    CHECK(Janela(5) == 3);
    for (int k = 5; k < 40; ++k) {
        CAPTURE(k);
        CHECK(Janela(k) >= 3);
        CHECK(Janela(k) <= 6);
        // O DENTE DE SERRA: sobe de um em um, ou cai do 6 para a metade.
        const int antes = Janela(k - 1);
        CHECK((Janela(k) == antes + 1 || (antes == 6 && Janela(k) == 3)));
    }
}

TEST_CASE("Golpe: o ponteiro curva sem mudar a rapidez, e acha o alvo") {
    Vector2 v(650.0f, 0.0f);
    const Vector2 alvo(0.0f, -1.0f);
    const float dt = 1.0f / 60.0f;
    for (int i = 0; i < 120; ++i) {
        const Vector2 antes = v;
        v = Curvar(v, alvo, 260.0f, dt);
        CHECK(v.Length() == doctest::Approx(650.0f).epsilon(0.001));
        const float giro = std::acos(std::clamp(Vector2::Dot(antes, v) / (650.0f * 650.0f), -1.0f, 1.0f));
        CHECK(giro <= Math::ToRadians(260.0f) * dt + 1e-4f);
    }
    CHECK(v.y / v.Length() < -0.999f);   // apontando para o alvo

    // O RUMO: sem desvio, direto no alvo; com o alvo em cima, para cima.
    const Vector2 r = Rumo(Vector2(100.0f, 500.0f), Vector2(100.0f, 100.0f), 0.0f);
    CHECK(r.x == doctest::Approx(0.0f));
    CHECK(r.y == doctest::Approx(-1.0f));
    const Vector2 emCima = Rumo(Vector2(5.0f, 5.0f), Vector2(5.0f, 5.0f), 0.0f);
    CHECK(emCima.y == doctest::Approx(-1.0f));
    const Vector2 girado = Rumo(Vector2(0.0f, 0.0f), Vector2(0.0f, -10.0f), 90.0f);
    CHECK(std::fabs(girado.x) == doctest::Approx(1.0f));
}

TEST_CASE("Golpe: o passo do gradiente oscila e converge") {
    CHECK(Oscilacao(0.0f, 60.0f) == doctest::Approx(0.0f));
    float maior = 0.0f;
    for (float t = 0.0f; t < 0.4f; t += 0.01f) maior = std::max(maior, std::fabs(Oscilacao(t, 60.0f)));
    CHECK(maior > 20.0f);   // erra de verdade no comeco
    for (float t = 4.0f * kTempoDaOnda; t < 2.0f; t += 0.01f) CHECK(std::fabs(Oscilacao(t, 60.0f)) < 0.05f * 60.0f);
}

TEST_CASE("Golpe: cada golpe tem o desenho da materia") {
    Variacao::Sorteio s = Variacao::SorteioDa(3u, "orientador");
    const auto todos = [&](const Materia m, auto&& tem) {
        const Rodada r = DaVez(m, 0, s);
        if (r.tiros.empty()) return false;
        for (const Tiro& t : r.tiros) if (!tem(t)) return false;
        return true;
    };
    CHECK(todos(Materia::Andre, [](const Tiro& t) { return !t.Guiado() && t.desvio == 0.0f; }));
    CHECK(todos(Materia::Salles, [](const Tiro& t) { return t.espera > 0.0f && t.dx == 0.0f; }));
    CHECK(todos(Materia::Ricardo, [](const Tiro& t) { return t.paralelo; }));
    CHECK(todos(Materia::Thiago, [](const Tiro& t) { return t.espera > 0.0f; }));
    CHECK(todos(Materia::Luizinho, [](const Tiro& t) { return t.curva > 0.0f && std::fabs(t.desvio) >= 45.0f; }));
    CHECK(todos(Materia::Vladimir, [](const Tiro& t) { return t.divide > 0.0f; }));
    CHECK(todos(Materia::Julio, [](const Tiro& t) { return t.onda != 0.0f; }));
    CHECK(todos(Materia::Lucas, [](const Tiro& t) { return t.reMira > 0.0f && std::fabs(t.desvio) >= 30.0f; }));
    CHECK(todos(Materia::Vitor, [](const Tiro& t) { return !t.Guiado(); }));
    // O ganho de um lado e do outro se alterna, no gradiente e no fetch.
    const Rodada julio = DaVez(Materia::Julio, 0, s);
    CHECK(julio.tiros[0].onda * julio.tiros[1].onda < 0.0f);
}
