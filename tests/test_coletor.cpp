//
// O coletor de lixo: quem esta vivo, e quando o mundo para.
//

#include "doctest.h"

#include <algorithm>
#include <vector>

#include "../Source/Attacks/Coletor.h"

namespace {
    Vector2 V(const float x, const float y) { return Vector2(x, y); }

    bool TemAresta(const Coletor::Grafo& g, const int a, const int b) {
        return std::find(g.arestas.begin(), g.arestas.end(), std::make_pair(a, b)) != g.arestas.end();
    }
}

TEST_CASE("Coletor: vivo e quem a raiz alcanca, por qualquer caminho") {
    // A raiz no zero, e uma corrente de blocos a 100 px um do outro, que se
    // afasta dela; e um bloco solto, longe de todos.
    const std::vector<Vector2> blocos = {
        V(100.0f, 0.0f), V(200.0f, 0.0f), V(300.0f, 0.0f), V(300.0f, 100.0f), V(900.0f, 900.0f)
    };
    const auto g = Coletor::Referencias(blocos, Vector2(0.0f, 0.0f), 110.0f);
    REQUIRE(g.vivo.size() == 5);
    CHECK(g.vivo[0]);
    CHECK(g.vivo[1]);
    CHECK(g.vivo[2]);
    CHECK(g.vivo[3]);          // tres saltos da raiz, e vivo do mesmo jeito
    CHECK_FALSE(g.vivo[4]);
}

TEST_CASE("Coletor: blocos que so apontam uns para os outros sao lixo") {
    // O CICLO SEM RAIZ. Os dois se referenciam, e ninguem da raiz chega neles:
    // uma contagem de referencias os deixaria vivos para sempre, e e aqui que o
    // marcar-e-varrer mostra para que serve.
    const std::vector<Vector2> blocos = {
        V(50.0f, 0.0f), V(600.0f, 600.0f), V(650.0f, 600.0f)
    };
    const auto g = Coletor::Referencias(blocos, Vector2(0.0f, 0.0f), 110.0f);
    CHECK(g.vivo[0]);
    CHECK_FALSE(g.vivo[1]);
    CHECK_FALSE(g.vivo[2]);
    CHECK(TemAresta(g, 1, 2));    // a referencia existe, e mesmo assim e lixo
}

TEST_CASE("Coletor: as arestas, uma por par, da raiz primeiro") {
    const std::vector<Vector2> blocos = { V(100.0f, 0.0f), V(150.0f, 0.0f), V(1000.0f, 0.0f) };
    const auto g = Coletor::Referencias(blocos, Vector2(0.0f, 0.0f), 110.0f);
    REQUIRE(g.arestas.size() == 2);
    CHECK(g.arestas[0] == std::make_pair(Coletor::kRaiz, 0));
    CHECK(g.arestas[1] == std::make_pair(0, 1));
    CHECK_FALSE(TemAresta(g, 1, 0));   // o par sai uma vez so
}

TEST_CASE("Coletor: o alcance e inclusivo, e sem blocos nao ha grafo") {
    const auto justo = Coletor::Referencias({ V(110.0f, 0.0f) }, Vector2(0.0f, 0.0f), 110.0f);
    CHECK(justo.vivo[0]);
    const auto vazio = Coletor::Referencias({}, Vector2(0.0f, 0.0f), 110.0f);
    CHECK(vazio.vivo.empty());
    CHECK(vazio.arestas.empty());
}

TEST_CASE("Coletor: o mundo para nos ultimos segundos do ciclo, e a varredura e o fim da pausa") {
    Coletor::Relogio r{4.0f, 1.0f, 0.0f};
    REQUIRE(Coletor::Valido(r));

    const float dt = 1.0f / 64.0f;   // exato em float: o teste conta quadros
    int para = -1, varre = -1;
    for (int q = 0; q < 400; ++q) {
        const auto e = Coletor::Avancar(r, dt);
        if (e == Coletor::Evento::Para && para < 0) para = q;
        if (e == Coletor::Evento::Varre && varre < 0) varre = q;
        if (para >= 0 && varre < 0) CHECK(Coletor::Estado(r) == Coletor::Mundo::Parado);
    }
    // Para aos 3 s (o quadro 192, contando do zero, fecha 3 s) e varre aos 4 s.
    CHECK(para == 191);
    CHECK(varre == 255);
    CHECK(Coletor::Estado(r) == Coletor::Mundo::Rodando);
}

TEST_CASE("Coletor: o ciclo se repete") {
    Coletor::Relogio r{2.0f, 0.5f, 0.0f};
    int paradas = 0, varridas = 0;
    for (int q = 0; q < 64 * 10; ++q) {                 // 10 s
        const auto e = Coletor::Avancar(r, 1.0f / 64.0f);
        if (e == Coletor::Evento::Para) ++paradas;
        if (e == Coletor::Evento::Varre) ++varridas;
    }
    CHECK(paradas == 5);
    CHECK(varridas == 5);
}

TEST_CASE("Coletor: um passo grande nao encurta a pausa") {
    // Um quadro longo (o jogo engasgou) que passa do fim do ciclo ainda para o
    // mundo - e a pausa vem INTEIRA depois dele. Sem isso, o quadro seguinte ja
    // varreria, e o lixo sumiria sem o aluno ter visto a marca.
    Coletor::Relogio r{4.0f, 1.0f, 2.9f};
    CHECK(Coletor::Avancar(r, 2.0f) == Coletor::Evento::Para);   // pulou o fim do ciclo
    CHECK(Coletor::Estado(r) == Coletor::Mundo::Parado);

    int parado = 0;
    while (Coletor::Avancar(r, 1.0f / 64.0f) != Coletor::Evento::Varre) {
        ++parado;
        REQUIRE(parado < 1000);
        CHECK(Coletor::Estado(r) == Coletor::Mundo::Parado);
    }
    CHECK(parado == 63);   // 1 s em passos de 1/64: a 64a chamada varre
    CHECK(Coletor::Estado(r) == Coletor::Mundo::Rodando);
}

TEST_CASE("Coletor: um relogio sem ciclo, ou com a pausa do tamanho do ciclo, nao anda") {
    for (const Coletor::Relogio r : { Coletor::Relogio{0.0f, 0.0f, 0.0f},
                                      Coletor::Relogio{4.0f, 4.0f, 0.0f},
                                      Coletor::Relogio{4.0f, -1.0f, 0.0f} }) {
        Coletor::Relogio copia = r;
        CHECK_FALSE(Coletor::Valido(copia));
        for (int q = 0; q < 1000; ++q) CHECK(Coletor::Avancar(copia, 0.1f) == Coletor::Evento::Nenhum);
    }
}
