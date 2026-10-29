//
// O TCC do Hugo: o tema do orientador, a banca e as perguntas dela.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "../Source/Attacks/Hugo/Tcc.h"

using namespace Tcc;
using Dificuldade::Nivel;

namespace {
    int Finais(const Golpe::Tiro& t) {
        return t.divide > 0.0f ? (2 << t.geracoes) : 1;
    }
    int Total(const Golpe::Rodada& r) {
        int n = 0;
        for (const Golpe::Tiro& t : r.tiros) n += Finais(t);
        return n;
    }
}

TEST_CASE("Tcc: a banca sao dois professores, nenhum deles o orientador") {
    std::set<int> vistos;
    for (const Materia orientador : kMaterias) {
        for (std::uint32_t semente = 0; semente < 60; ++semente) {
            Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDaDefesa);
            const auto banca = Banca(orientador, s);
            CHECK(banca[0] != banca[1]);
            CHECK(banca[0] != orientador);
            CHECK(banca[1] != orientador);
            CHECK(banca[0] != Materia::Generico);
            CHECK(banca[1] != Materia::Generico);
            vistos.insert(static_cast<int>(banca[0]));
            vistos.insert(static_cast<int>(banca[1]));
        }
    }
    CHECK(vistos.size() == kMaterias.size());   // todo professor ja foi banca de alguem
    Variacao::Sorteio s = Variacao::SorteioDa(1u, kFaseDaDefesa);
    const auto semOrientador = Banca(Materia::Generico, s);
    CHECK(semOrientador[0] != semOrientador[1]);
}

TEST_CASE("Tcc: todo padrao sai em ordem, cabe na rodada, e o Dificil e mais") {
    std::vector<Materia> todas(kMaterias.begin(), kMaterias.end());
    todas.push_back(Materia::Generico);
    for (const Materia m : todas) {
        CAPTURE(static_cast<int>(m));
        for (int k = 0; k < 6; ++k) {
            Variacao::Sorteio sn = Variacao::SorteioDa(5u, kFaseDoTema);
            Variacao::Sorteio sd = Variacao::SorteioDa(5u, kFaseDoTema);
            const Golpe::Rodada normal = Padrao(m, k, sn, Nivel::Normal);
            const Golpe::Rodada dificil = Padrao(m, k, sd, Nivel::Dificil);
            REQUIRE_FALSE(normal.tiros.empty());
            for (const Golpe::Rodada* r : {&normal, &dificil}) {
                for (size_t i = 1; i < r->tiros.size(); ++i) CHECK(r->tiros[i].quando >= r->tiros[i - 1].quando);
                for (const Golpe::Tiro& t : r->tiros) {
                    CHECK(t.quando + t.espera < r->duracao);
                    CHECK(t.velocidade >= 200.0f);
                    CHECK(t.velocidade <= 320.0f);   // e um bullet hell, nao um teste de reflexo
                }
                // CURTO: um padrao se le de uma vez, e o seguinte pode ate
                // comecar antes de ele acabar, mas nao muito antes.
                CHECK(r->duracao <= 1.6f);
            }
            CHECK(Total(dificil) >= Total(normal));
            CHECK(dificil.tiros.front().velocidade >= normal.tiros.front().velocidade);
            CHECK(Total(normal) >= 6);
        }
    }
    const Degrau n = DegrauDo(Nivel::Normal), d = DegrauDo(Nivel::Dificil);
    CHECK(d.entreTemas < n.entreTemas);
    CHECK(d.entrePerguntas < n.entrePerguntas);
    CHECK(d.entreCorrecoes < n.entreCorrecoes);
}

TEST_CASE("Tcc: cada padrao tem o desenho da materia") {
    Variacao::Sorteio s = Variacao::SorteioDa(9u, kFaseDoTema);
    const auto P = [&](const Materia m) { return Padrao(m, 0, s, Nivel::Normal); };

    // A PILHA: o nivel de baixo (o ultimo empilhado) sai primeiro.
    const auto pilha = P(Materia::Salles);
    std::map<float, float> partidaPorNivel;
    for (const auto& t : pilha.tiros) partidaPorNivel[t.dy] = t.quando + t.espera;
    float anterior = 1e9f;
    for (const auto& [dy, partida] : partidaPorNivel) {   // dy crescente: empilhados depois
        (void)dy;
        CHECK(partida < anterior + 1e-4f);
        anterior = partida;
    }

    // A TRANSACAO: todas partem juntas, no COMMIT, a linha andando junta, e
    // o ROLLBACK desfez tres escritas - as frestas.
    const auto transacao = P(Materia::Thiago);
    CHECK(transacao.tiros.size() == 6u);
    for (const auto& t : transacao.tiros) {
        CHECK(t.paralelo);
        CHECK(t.quando + t.espera == doctest::Approx(transacao.tiros[0].quando + transacao.tiros[0].espera));
    }

    // O BARRAMENTO: paredes paralelas de 4 bits em 8, com frestas.
    const auto bus = P(Materia::Ricardo);
    std::map<float, int> porByte;
    for (const auto& t : bus.tiros) { CHECK(t.paralelo); ++porByte[t.quando]; }
    CHECK(porByte.size() == 3u);
    for (const auto& [quando, uns] : porByte) { (void)quando; CHECK(uns == 4); }

    // OS OBSERVADORES: aparecem juntos e atiram um depois do outro.
    const auto obs = P(Materia::MariaLucia);
    std::map<float, float> partidaPorOlho;
    for (const auto& t : obs.tiros) { CHECK(t.quando == 0.0f); partidaPorOlho[t.dx] = t.espera; }
    CHECK(partidaPorOlho.size() == 4u);
    float antes = -1.0f;
    for (const auto& [dx, espera] : partidaPorOlho) { (void)dx; CHECK(espera > antes); antes = espera; }

    for (const auto& t : P(Materia::Vladimir).tiros) { CHECK(t.divide > 0.0f); CHECK(t.herdaRumo); CHECK(t.geracoes == 2); }
    for (const auto& t : P(Materia::Lucas).tiros) { CHECK(t.reMira > 0.0f); CHECK(std::fabs(t.desvio) >= 40.0f); }
    for (const auto& t : P(Materia::Luizinho).tiros) { CHECK(t.curva > 0.0f); CHECK(std::fabs(t.desvio) >= 40.0f); }
    const auto gradiente = P(Materia::Julio);
    for (size_t i = 1; i < gradiente.tiros.size(); ++i) CHECK(gradiente.tiros[i].onda * gradiente.tiros[i - 1].onda < 0.0f);

    // A JANELA: rajadas de 1, 2, 4 e 8.
    std::map<int, int> rajadas;
    for (const auto& t : P(Materia::Vitor).tiros) ++rajadas[static_cast<int>(std::floor(t.quando / 0.3f + 1e-3f))];
    CHECK(rajadas[0] == 1);
    CHECK(rajadas[1] == 2);
    CHECK(rajadas[2] == 4);
    CHECK(rajadas[3] == 8);

    // O FOR: o arco varre de um lado ao outro, e a volta seguinte, ao contrario.
    const auto laco = P(Materia::Andre);
    CHECK(laco.tiros.front().desvio < laco.tiros.back().desvio);
    const auto volta = Padrao(Materia::Andre, 1, s, Nivel::Normal);
    CHECK(volta.tiros.front().desvio > volta.tiros.back().desvio);
}

TEST_CASE("Tcc: cada professor tem a area, o capitulo, a pergunta e o projetil dele") {
    std::set<std::string> areas, capitulos, perguntas, animacoes;
    for (const Materia m : kMaterias) {
        areas.insert(Area(m));
        capitulos.insert(Capitulo(m));
        perguntas.insert(Pergunta(m));
        animacoes.insert(Animacao(m));
    }
    CHECK(areas.size() == kMaterias.size());
    CHECK(capitulos.size() == kMaterias.size());
    CHECK(perguntas.size() == kMaterias.size());
    CHECK(animacoes.size() == kMaterias.size());
    CHECK(Area(Materia::Salles) == "Estruturas de Dados");
    // O NOME VOLTA PELO GOLPE: e o mesmo de materias.json.
    for (const Materia m : kMaterias) CHECK(Golpe::DoProfessor(Professor(m)) == m);
    CHECK(Pergunta(Materia::Vitor) == "E se der deadlock?");
    CHECK(Animacao(Materia::Generico) == "Folha");
}

TEST_CASE("Golpe: o voo espera junto de quem atirou, parte mirado, e a arvore abre") {
    Golpe::Tiro t;
    t.dx = 10.0f;
    t.dy = 30.0f;
    t.espera = 0.5f;
    t.velocidade = 200.0f;
    Golpe::Voo v = Golpe::Comecar(t, Vector2(0.0f, 1.0f));
    const float dt = 0.1f;
    // ESPERANDO, acompanha quem atirou.
    Golpe::Passo p = Golpe::Avancar(v, Vector2(0.0f, 0.0f), Vector2::Zero, Vector2(100.0f, 100.0f), Vector2(110.0f, 500.0f), dt);
    CHECK(p.posicao.x == doctest::Approx(110.0f));
    CHECK(p.posicao.y == doctest::Approx(130.0f));
    CHECK(p.velocidade.Length() == doctest::Approx(0.0f));
    for (int i = 0; i < 3; ++i) p = Golpe::Avancar(v, p.posicao, p.velocidade, Vector2(100.0f, 100.0f), Vector2(110.0f, 500.0f), dt);
    CHECK(p.velocidade.Length() == doctest::Approx(0.0f));
    // PARTE mirado no alvo, que esta embaixo.
    for (int i = 0; i < 2; ++i) p = Golpe::Avancar(v, p.posicao, p.velocidade, Vector2(100.0f, 100.0f), Vector2(110.0f, 500.0f), dt);
    CHECK(p.velocidade.y == doctest::Approx(200.0f));
    CHECK(p.velocidade.x == doctest::Approx(0.0f).epsilon(0.01));

    // A ARVORE: os filhos abrem do rumo do pai, sem mirar de novo.
    Golpe::Tiro semente;
    semente.velocidade = 200.0f;
    semente.divide = 0.2f;
    semente.geracoes = 1;
    semente.abertura = 20.0f;
    semente.herdaRumo = true;
    Golpe::Voo pai = Golpe::Comecar(semente, Vector2(1.0f, 0.0f));
    Golpe::Passo q;
    bool dividiu = false;
    Vector2 pos(0.0f, 0.0f), vel(200.0f, 0.0f);
    for (int i = 0; i < 5 && !dividiu; ++i) {
        q = Golpe::Avancar(pai, pos, vel, Vector2::Zero, Vector2(0.0f, 900.0f), 0.05f);
        dividiu = q.divide;
    }
    REQUIRE(dividiu);
    const auto filhos = Golpe::Filhos(pai, q.posicao, q.velocidade, Vector2(0.0f, 900.0f));
    const float angulo = Math::ToDegrees(std::acos(std::clamp(Vector2::Dot(filhos[0].direcao, filhos[1].direcao), -1.0f, 1.0f)));
    CHECK(angulo == doctest::Approx(20.0f).epsilon(0.01));
    CHECK(filhos[0].direcao.x > 0.9f);   // o rumo do pai, e nao o do alvo (embaixo)
    CHECK(filhos[0].tiro.geracoes == 0);
    CHECK(filhos[0].tiro.divide > 0.0f);

    // O PARALELO: os fios ficam de lado em relacao ao rumo.
    Golpe::Tiro fio;
    fio.dx = 44.0f;
    fio.paralelo = true;
    const Golpe::Saida saida = Golpe::Sair(fio, Vector2(0.0f, 0.0f), Vector2(500.0f, 0.0f));
    CHECK(saida.direcao.x == doctest::Approx(1.0f));
    CHECK(std::fabs(saida.de.y) == doctest::Approx(44.0f));
    CHECK(saida.de.x == doctest::Approx(0.0f));
}

TEST_CASE("Tcc: a mira adianta onde o aluno vai estar, ate um limite") {
    const Vector2 de(600.0f, 100.0f), aluno(600.0f, 700.0f), andando(100.0f, 0.0f);
    // PARADO, ou sem adiantar: no aluno.
    CHECK(Mira(de, 300.0f, aluno, Vector2::Zero, 1.0f).x == doctest::Approx(600.0f));
    CHECK(Mira(de, 300.0f, aluno, andando, 0.0f).x == doctest::Approx(600.0f));
    // ANDANDO: 600 px a 300 px/s sao 2 s de voo, mas a mira so adianta kAdiantaAte.
    CHECK(Mira(de, 300.0f, aluno, andando, 1.0f).x == doctest::Approx(600.0f + 100.0f * kAdiantaAte));
    CHECK(Mira(de, 300.0f, aluno, andando, 0.5f).x == doctest::Approx(600.0f + 50.0f * kAdiantaAte));
    // PERTO: o voo e curto, e adianta pouco.
    CHECK(Mira(Vector2(600.0f, 600.0f), 300.0f, aluno, andando, 1.0f).x == doctest::Approx(600.0f + 100.0f / 3.0f));
    CHECK(kAdiantamentoDaBanca < 1.0f);
}
