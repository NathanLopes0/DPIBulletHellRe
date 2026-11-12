//
// A fase 1 do Luizinho: as amarracoes.
//

#include "doctest.h"

#include <set>

#include "../Source/Attacks/Amarracao.h"

using namespace Amarracao;

namespace {
    Programa ExemploDeAula() {
        // int x = 2;  void f() { { int x = 6; } }  void g() { int x = 5; }
        Programa p;
        p.x = {2, 0, 6, 5};
        return p;
    }
}

TEST_CASE("Amarracao: o bloco de dentro esta em f, e f e g estao no global") {
    CHECK(Pai(Bloco::Interno) == Bloco::F);
    CHECK(Pai(Bloco::F) == Bloco::Global);
    CHECK(Pai(Bloco::G) == Bloco::Global);
    CHECK(Pai(Bloco::Global) == Bloco::Global);
}

TEST_CASE("Amarracao: a busca olha do bloco de dentro para fora e para em quem declara") {
    const Programa p = ExemploDeAula();
    Comando c;
    c.onde = Bloco::Interno;
    CHECK(Busca(p, c) == std::vector<Bloco>{Bloco::Interno});
    c.onde = Bloco::F;
    CHECK(Busca(p, c) == std::vector<Bloco>{Bloco::F, Bloco::Global});
    c.onde = Bloco::G;
    CHECK(Busca(p, c) == std::vector<Bloco>{Bloco::G});
    c.onde = Bloco::Global;
    CHECK(Busca(p, c) == std::vector<Bloco>{Bloco::Global});

    // Sem o x no bloco de dentro, a busca sai dele, passa por f e chega ao global.
    Programa semInterno = p;
    semInterno.x[static_cast<size_t>(Bloco::Interno)] = 0;
    c.onde = Bloco::Interno;
    CHECK(Busca(semInterno, c) == std::vector<Bloco>{Bloco::Interno, Bloco::F, Bloco::Global});
    CHECK(Dono(semInterno, c) == Bloco::Global);
}

TEST_CASE("Amarracao: a mesma chamada de f por g acha o x do global no estatico e o de g no dinamico") {
    const Programa p = ExemploDeAula();
    const Comando estatico = Chamada(Regra::Estatico);
    const Comando dinamico = Chamada(Regra::Dinamico);
    CHECK(estatico.onde == Bloco::F);
    CHECK(Busca(p, estatico) == std::vector<Bloco>{Bloco::F, Bloco::Global});
    CHECK(Busca(p, dinamico) == std::vector<Bloco>{Bloco::F, Bloco::G});
    CHECK(p.Valor(Dono(p, estatico)) == 2);
    CHECK(p.Valor(Dono(p, dinamico)) == 5);
    // Fora da chamada, a regra nao muda nada: quem roda em f sempre sobe pelo texto.
    Comando emF;
    emF.onde = Bloco::F;
    emF.regra = Regra::Dinamico;
    CHECK(Dono(p, emF) == Bloco::Global);
}

TEST_CASE("Amarracao: o programa de cada luta tem o global e g declarando, f nunca, e valores diferentes") {
    std::set<int> interno;
    for (std::uint32_t s = 0; s < 200; ++s) {
        Variacao::Sorteio sorteio(s);
        const Programa p = SortearPrograma(sorteio);
        CHECK(p.Declara(Bloco::Global));
        CHECK(p.Declara(Bloco::G));
        CHECK_FALSE(p.Declara(Bloco::F));
        interno.insert(p.Declara(Bloco::Interno) ? 1 : 0);
        std::set<int> valores;
        int declarados = 0;
        for (int b = 0; b < kBlocos; ++b) {
            const int v = p.Valor(static_cast<Bloco>(b));
            if (v == 0) continue;
            ++declarados;
            valores.insert(v);
            CHECK(v >= kMenorValor);
            CHECK(v <= kMaiorValor);
        }
        CHECK(valores.size() == static_cast<size_t>(declarados));
        // Assim a chamada de f sempre muda de resposta com a regra.
        CHECK(Dono(p, Chamada(Regra::Estatico)) != Dono(p, Chamada(Regra::Dinamico)));
    }
    // As vezes o bloco de dentro declara, as vezes nao.
    CHECK(interno.size() == 2);
}

TEST_CASE("Amarracao: os comandos comuns passam por todos os blocos e nunca repetem o anterior") {
    std::set<int> vistos;
    for (std::uint32_t s = 0; s < 200; ++s) {
        const Bloco anterior = static_cast<Bloco>(s % kBlocos);
        const Comando c = Proximo(s, anterior);
        CHECK(c.onde != anterior);
        CHECK_FALSE(c.chamada);
        CHECK(c.regra == Regra::Estatico);
        vistos.insert(static_cast<int>(c.onde));
    }
    CHECK(vistos.size() == static_cast<size_t>(kBlocos));
}

TEST_CASE("Amarracao: o placar diz onde roda e, achado o x, de onde ele veio") {
    const Programa p = ExemploDeAula();
    Comando c;
    c.onde = Bloco::Interno;
    CHECK(Placar(p, c, false) == "void f() { { atira(x); } }");
    CHECK(Placar(p, c, true) == "void f() { { atira(x); } }\nx = 6, do bloco de dentro de f");
    c.onde = Bloco::F;
    CHECK(Placar(p, c, true) == "void f() { atira(x); }\nx = 2, do global");
    CHECK(Placar(p, Chamada(Regra::Estatico), false) == "g() chama f(): atira(x);\nescopo est\xC3\xA1tico");
    CHECK(Placar(p, Chamada(Regra::Dinamico), true) == "g() chama f(): atira(x);\ndin\xC3\xA2mico: x = 5, de quem chamou");
    CHECK(Declaracao(p, Bloco::Global) == "int x = 2;  // global");
    CHECK(Declaracao(p, Bloco::F) == "void f() {");
    CHECK(Declaracao(p, Bloco::Interno) == "{ int x = 6;");
    CHECK(Declaracao(p, Bloco::G) == "void g() { int x = 5;");
    Programa semInterno = p;
    semInterno.x[static_cast<size_t>(Bloco::Interno)] = 0;
    CHECK(Declaracao(semInterno, Bloco::Interno) == "{");
}

TEST_CASE("Amarracao: os paineis ficam no campo, o bloco de dentro em f, e o professor no bloco dele") {
    for (const bool fAEsquerda : {true, false}) {
        CAPTURE(fAEsquerda);
        const float w = 1200.0f, h = 746.0f;
        const Retangulo f = Area(Bloco::F, fAEsquerda, w, h);
        const Retangulo interno = Area(Bloco::Interno, fAEsquerda, w, h);
        const Retangulo g = Area(Bloco::G, fAEsquerda, w, h);
        CHECK(f.Contem(interno.x, interno.y));
        CHECK(f.Contem(interno.x + interno.w, interno.y + interno.h));
        // f e g nao se tocam: g nao esta dentro de f.
        CHECK((g.x >= f.x + f.w || g.x + g.w <= f.x));
        for (const Retangulo& r : {f, interno, g}) {
            CHECK(r.x >= 0.0f);
            CHECK(r.x + r.w <= w);
            CHECK(r.y + r.h <= h - 100.0f);   // o fundo do campo fica livre para o aluno
        }
        const Ponto emInterno = OndeFicaOProfessor(Bloco::Interno, fAEsquerda, w, h);
        const Ponto emF = OndeFicaOProfessor(Bloco::F, fAEsquerda, w, h);
        const Ponto emG = OndeFicaOProfessor(Bloco::G, fAEsquerda, w, h);
        CHECK(interno.Contem(emInterno.x, emInterno.y));
        CHECK(f.Contem(emF.x, emF.y));
        CHECK_FALSE(interno.Contem(emF.x, emF.y));
        CHECK(g.Contem(emG.x, emG.y));
        // O canto do global fica no alto, do lado direito (o placar e a esquerda).
        const Ponto global = OndeFicaOCanto(Bloco::Global, fAEsquerda, w, h, 200.0f);
        CHECK(global.x > w / 2.0f);
        CHECK(global.y < 60.0f);
    }
}

TEST_CASE("Amarracao: o leque tem um tiro por unidade de x, e a duracao conta cada bloco olhado") {
    CHECK(AberturaDoLeque(1) == doctest::Approx(0.0f));
    CHECK(AberturaDoLeque(5) == doctest::Approx(4.0f * kEntreTiros));
    const Programa p = ExemploDeAula();
    Comando emG;
    emG.onde = Bloco::G;
    Comando emF;
    emF.onde = Bloco::F;
    CHECK(Duracao(p, emF) == doctest::Approx(Duracao(p, emG) + kPorSalto));
    // Nenhum comando passa de tres segundos, nem o que olha os tres blocos.
    Programa semInterno = p;
    semInterno.x[static_cast<size_t>(Bloco::Interno)] = 0;
    Comando emInterno;
    emInterno.onde = Bloco::Interno;
    CHECK(Duracao(semInterno, emInterno) + kMaiorPausa < 3.0f);
}

TEST_CASE("Amarracao: o degrau do Normal e o de sempre, e o do Dificil aperta") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.rajadas == kRajadas);
    CHECK(n.velocidade == doctest::Approx(kVelocidade));
    CHECK(n.porSalto == doctest::Approx(kPorSalto));
    CHECK(d.rajadas >= n.rajadas);
    CHECK(d.velocidade > n.velocidade);
    CHECK(d.porSalto < n.porSalto);
}

TEST_CASE("Amarracao: a rajada mira no aluno com um desvio de ate meio tiro") {
    CHECK(DesvioDaRajada(0.5f) == doctest::Approx(0.0f));
    CHECK(DesvioDaRajada(0.0f) == doctest::Approx(-kEntreTiros / 2.0f));
    CHECK(DesvioDaRajada(1.0f) == doctest::Approx(kEntreTiros / 2.0f));
    CHECK(DesvioDaRajada(7.0f) == doctest::Approx(kEntreTiros / 2.0f));
}
