//
// O padrao Observer: quando o sujeito notifica, quando consulta, e para onde
// o olho olha.
//

#include "doctest.h"

#include "../Source/Attacks/Observer.h"

namespace {
    const Vector2 kDireita(300.0f, 0.0f);
    const Vector2 kEsquerda(-300.0f, 0.0f);
    const Vector2 kParado(0.0f, 0.0f);
    constexpr float kQuadro = 1.0f / 60.0f;

    /// Anda 'segundos' na mesma velocidade e conta os avisos de cada tipo.
    void Anda(Observer::Sujeito& s, const Vector2 v, const float segundos, int& notificou, int& consultou) {
        for (float t = 0.0f; t < segundos - 1.0e-4f; t += kQuadro) {
            const auto aviso = Observer::Avancar(s, v, kQuadro);
            if (aviso == Observer::Aviso::Notifica) ++notificou;
            if (aviso == Observer::Aviso::Consulta) ++consultou;
        }
    }
}

TEST_CASE("Observer: o rumo tem zona morta, e o estado guarda o ultimo rumo de cada eixo") {
    CHECK(Observer::Rumo(300.0f) == 1);
    CHECK(Observer::Rumo(-300.0f) == -1);
    CHECK(Observer::Rumo(10.0f) == 0);
    CHECK(Observer::Rumo(-10.0f) == 0);

    Observer::Estado e;
    e = Observer::Proximo(e, kDireita);
    CHECK(e.x == 1);
    CHECK(e.y == 0);

    // PARAR NAO MUDA O ESTADO: o eixo em que ele nao anda guarda o rumo.
    e = Observer::Proximo(e, kParado);
    CHECK(e.x == 1);

    e = Observer::Proximo(e, Vector2(0.0f, -200.0f));
    CHECK(e.x == 1);
    CHECK(e.y == -1);

    e = Observer::Proximo(e, kEsquerda);
    CHECK(e.x == -1);
    CHECK(e.y == -1);
}

TEST_CASE("Observer: o primeiro quadro so le o estado, e virar notifica") {
    Observer::Sujeito s;
    CHECK(Observer::Avancar(s, kDireita, kQuadro) == Observer::Aviso::Nenhum);

    int n = 0, c = 0;
    Anda(s, kDireita, 1.0f, n, c);
    CHECK(n == 0);

    // Virou: notifica uma vez, e nao a cada quadro.
    Anda(s, kEsquerda, 1.0f, n, c);
    CHECK(n == 1);
    CHECK(c == 0);
}

TEST_CASE("Observer: parar nao notifica - so virar") {
    Observer::Sujeito s;
    int n = 0, c = 0;
    Anda(s, kDireita, 1.0f, n, c);
    Anda(s, kParado, 1.0f, n, c);
    Anda(s, kDireita, 1.0f, n, c);
    CHECK(n == 0);
}

TEST_CASE("Observer: quem vira e volta antes do intervalo minimo nao notifica nada") {
    Observer::Sujeito s;
    s.intervaloMinimo = 0.5f;
    int n = 0, c = 0;
    Anda(s, kDireita, 0.2f, n, c);

    // O aviso anterior foi ha 0,2 s: a virada fica pendente...
    Anda(s, kEsquerda, 0.1f, n, c);
    // ...e volta ao estado notificado antes de o intervalo passar.
    Anda(s, kDireita, 1.0f, n, c);
    CHECK(n == 0);

    // A mesma virada, mantida, sai assim que o intervalo passa.
    Observer::Sujeito t;
    t.intervaloMinimo = 0.5f;
    int m = 0, d = 0;
    Anda(t, kDireita, 0.2f, m, d);
    Anda(t, kEsquerda, 0.25f, m, d);
    CHECK(m == 0);
    Anda(t, kEsquerda, 0.1f, m, d);
    CHECK(m == 1);
}

TEST_CASE("Observer: sem aviso, a professora consulta - o polling") {
    Observer::Sujeito s;
    s.consulta = 2.0f;
    int n = 0, c = 0;
    Anda(s, kParado, 7.0f, n, c);
    CHECK(n == 0);
    CHECK(c == 3);   // aos 2, 4 e 6 s

    // UMA NOTIFICACAO ADIA A CONSULTA: o relogio da consulta conta do ultimo
    // aviso, de qualquer tipo.
    Observer::Sujeito t;
    t.consulta = 2.0f;
    int m = 0, d = 0;
    Anda(t, kDireita, 1.5f, m, d);
    Anda(t, kEsquerda, 1.5f, m, d);    // notifica aos 1,5 s
    CHECK(m == 1);
    CHECK(d == 0);                      // a consulta dos 2 s nao veio
    Anda(t, kEsquerda, 1.0f, m, d);    // ...e vem aos 3,5 s
    CHECK(d == 1);
}

TEST_CASE("Observer: o olho olha para o lado em que o aluno esta, com zona morta") {
    const Vector2 olho(500.0f, 300.0f);
    CHECK(Observer::Olhar(olho, Vector2(700.0f, 650.0f)) == 1);
    CHECK(Observer::Olhar(olho, Vector2(300.0f, 650.0f)) == -1);
    CHECK(Observer::Olhar(olho, Vector2(520.0f, 650.0f)) == 0);
}
