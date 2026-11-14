//
// A Sabrina (INF 491 - Topicos Especiais: Bioinformatica): a camada pura das quatro provas.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

#include "../Source/Attacks/Dobramento.h"
#include "../Source/Attacks/Fase.h"
#include "../Source/Attacks/Helice.h"
#include "../Source/Attacks/Pareamento.h"
#include "../Source/Attacks/Sequenciamento.h"

namespace {
    Vector2 V(const float x, const float y) { return Vector2(x, y); }

    bool Perto(const Vector2 a, const Vector2 b, const float tolerancia = 1e-3f) {
        return (a - b).Length() < tolerancia;
    }

    using Pareamento::Base;

    std::vector<Base> Dna(const char* s) {
        std::vector<Base> v;
        for (; *s; ++s) {
            switch (*s) {
                case 'A': v.push_back(Base::A); break;
                case 'C': v.push_back(Base::C); break;
                case 'G': v.push_back(Base::G); break;
                case 'T': v.push_back(Base::T); break;
                default: v.push_back(Base::U); break;
            }
        }
        return v;
    }

    char Traduzir(const char* codon) {
        const auto b = Dna(codon);
        return Pareamento::Traduzir(b[0], b[1], b[2]);
    }
}

// ---- O pareamento ----

TEST_CASE("Pareamento: A com T (no RNA, com U), C com G - e o aluno absorve so o tiro que pareia com a base dele") {
    using namespace Pareamento;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(kDuracoes[0] + 0.1f) == 1);
    CHECK(EtapaNoTempo(19.9f) == 2);

    CHECK(Complementar(Base::A, false) == Base::T);
    CHECK(Complementar(Base::A, true) == Base::U);
    CHECK(Complementar(Base::T, false) == Base::A);
    CHECK(Complementar(Base::U, true) == Base::A);
    CHECK(Complementar(Base::C, false) == Base::G);
    CHECK(Complementar(Base::G, true) == Base::C);
    CHECK(Pontes(Base::A) == 2);
    CHECK(Pontes(Base::U) == 2);
    CHECK(Pontes(Base::C) == 3);
    CHECK(Pontes(Base::G) == 3);

    // T e U sao a mesma classe: o selo do aluno troca de letra no RNA, nao de par.
    CHECK(Classe(Base::T) == Classe(Base::U));
    CHECK(DaClasse(Classe(Base::T), true) == Base::U);
    CHECK(DaClasse(Classe(Base::U), false) == Base::T);
    CHECK(NoAlfabeto(Base::T, true) == Base::U);
    CHECK(NoAlfabeto(Base::G, true) == Base::G);

    // O tiro da classe do aluno passa por ele (Colisoes): a classe do tiro e a da base que pareia.
    for (const Base aluno : {Base::A, Base::C, Base::G, Base::T, Base::U}) {
        for (const Base tiro : {Base::A, Base::C, Base::G, Base::T, Base::U}) {
            CAPTURE(Letra(aluno));
            CAPTURE(Letra(tiro));
            CHECK((ClasseDoTiro(tiro) == Classe(aluno)) == Pareiam(aluno, tiro));
            CHECK(Pareiam(aluno, tiro) == Pareiam(tiro, aluno));
        }
    }
    CHECK(Pareiam(Base::A, Base::U));
    CHECK(Pareiam(Base::G, Base::C));
    CHECK_FALSE(Pareiam(Base::A, Base::G));
    CHECK_FALSE(Pareiam(Base::T, Base::U));
    CHECK(std::string(Cara(Base::U)) == "U");
}

TEST_CASE("Pareamento: a transcricao le o molde, e o codigo genetico da o aminoacido e a familia dele") {
    using namespace Pareamento;
    CHECK(Transcrever(Base::A) == Base::U);
    CHECK(Transcrever(Base::T) == Base::A);
    CHECK(Transcrever(Base::C) == Base::G);
    CHECK(Transcrever(Base::G) == Base::C);
    // A fita molde que se transcreve no RNA, base a base - e a base do molde pareia com a do RNA.
    const auto rna = Dna("AUGGCUUAA");
    const auto molde = Molde(rna);
    REQUIRE(molde.size() == rna.size());
    for (size_t k = 0; k < rna.size(); ++k) {
        CHECK(Transcrever(molde[k]) == rna[k]);
        CHECK(Pareiam(molde[k], rna[k]));
        CHECK(molde[k] != Base::U);
    }

    CHECK(Traduzir("AUG") == 'M');
    CHECK(Traduzir("ATG") == 'M');
    CHECK(Traduzir("UUU") == 'F');
    CHECK(Traduzir("UGG") == 'W');
    CHECK(Traduzir("UAA") == '*');
    CHECK(Traduzir("UAG") == '*');
    CHECK(Traduzir("UGA") == '*');
    CHECK(Traduzir("GGG") == 'G');
    CHECK(Traduzir("CAU") == 'H');
    CHECK(Traduzir("GAU") == 'D');
    CHECK(Traduzir("GAG") == 'E');
    CHECK(Traduzir("AAA") == 'K');
    CHECK(Traduzir("AGA") == 'R');
    CHECK(Traduzir("UCA") == 'S');
    CHECK(Traduzir("AUA") == 'I');
    // Os 64 codons: 3 paradas, 61 aminoacidos, as 20 letras.
    std::set<char> letras;
    int paradas = 0;
    for (const Base a : kRna) for (const Base b : kRna) for (const Base c : kRna) {
        const char aa = Pareamento::Traduzir(a, b, c);
        if (aa == '*') ++paradas;
        else letras.insert(aa);
    }
    CHECK(paradas == 3);
    CHECK(letras.size() == 20u);

    CHECK(FamiliaDe('L') == Familia::Hidrofobico);
    CHECK(FamiliaDe('S') == Familia::Polar);
    CHECK(FamiliaDe('K') == Familia::Positivo);
    CHECK(FamiliaDe('E') == Familia::Negativo);
    CHECK(FamiliaDe('*') == Familia::Parada);
    CHECK(std::string(NomeDo('M')) == "Met");
    CHECK(std::string(NomeDo('*')) == "Pare");

    // O gene comeca no AUG, acaba numa parada, e nao tem parada no meio.
    Variacao::Sorteio s(7u);
    for (int vez = 0; vez < 20; ++vez) {
        const auto gene = Gene(s, 6);
        REQUIRE(gene.size() == 18u);
        CHECK(Pareamento::Traduzir(gene[0], gene[1], gene[2]) == 'M');
        CHECK(Pareamento::Traduzir(gene[15], gene[16], gene[17]) == '*');
        for (size_t c = 3; c + 3 < gene.size(); c += 3) CHECK(Pareamento::Traduzir(gene[c], gene[c + 1], gene[c + 2]) != '*');
        for (const Base b : gene) CHECK(b != Base::T);
    }
}

TEST_CASE("Pareamento: a onda e quase toda da base dominante, as sequencias repetem, e as capsulas giram espacadas") {
    using namespace Pareamento;
    Variacao::Sorteio s(11u);
    const auto onda = Onda(s, Base::T, 24, 0.75f, true);
    REQUIRE(onda.size() == 24u);
    // No RNA, a dominante T vira U; nenhuma base e T.
    CHECK(std::count(onda.begin(), onda.end(), Base::U) == 18);
    CHECK(std::count(onda.begin(), onda.end(), Base::T) == 0);
    for (int vez = 0; vez < 50; ++vez) {
        const Base b = kDna[static_cast<size_t>(vez % 4)];
        CHECK(Outra(s, b, false) != b);
    }
    // Repetir tudo da a mesma base; nunca repetir, nenhuma vizinha igual.
    const auto iguais = Sequencia(s, 12, 1.0f, false);
    CHECK(std::count(iguais.begin(), iguais.end(), iguais.front()) == 12);
    const auto trocadas = Sequencia(s, 30, 0.0f, true);
    for (size_t k = 1; k < trocadas.size(); ++k) {
        CHECK(trocadas[k] != trocadas[k - 1]);
        CHECK(trocadas[k] != Base::T);
    }

    // As quatro capsulas: na elipse, a um quarto de volta uma da outra.
    const Vector2 c = V(600.0f, 500.0f), raios = V(400.0f, 100.0f);
    const Vector2 p0 = NaOrbita(0, 4, 0.0f, c, raios, 0.5f);
    const Vector2 p1 = NaOrbita(1, 4, 0.0f, c, raios, 0.5f);
    CHECK(Perto(p0, V(1000.0f, 500.0f)));
    CHECK(Perto(p1, V(600.0f, 600.0f)));
    CHECK(Perto(NaOrbita(0, 4, Math::Pi, c, raios, 0.5f), p1, 1e-2f));
}

// ---- A dupla helice ----

TEST_CASE("Helice: o modelo gira sem deformar, e a perspectiva aumenta o que vem para a frente") {
    using namespace Helice;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    const Ponto3 p{30.0f, 40.0f, 0.0f};
    const Ponto3 y = GirarEmY(p, 0.7f);
    const Ponto3 x = GirarEmX(p, 1.3f);
    CHECK(std::hypot(y.x, y.z) == doctest::Approx(30.0f));
    CHECK(y.y == doctest::Approx(40.0f));
    CHECK(std::hypot(x.y, x.z) == doctest::Approx(40.0f));
    CHECK(x.x == doctest::Approx(30.0f));
    // Um quarto de volta em y leva o x para o z (para a frente).
    CHECK(GirarEmY(Ponto3{50.0f, 0.0f, 0.0f}, Math::Pi / 2.0f).z == doctest::Approx(50.0f));

    const Vector2 centro = V(600.0f, 400.0f);
    const NaTela meio = Projetar(Ponto3{50.0f, 0.0f, 0.0f}, centro, 600.0f);
    const NaTela frente = Projetar(Ponto3{50.0f, 0.0f, 80.0f}, centro, 600.0f);
    const NaTela tras = Projetar(Ponto3{50.0f, 0.0f, -80.0f}, centro, 600.0f);
    CHECK(meio.escala == doctest::Approx(1.0f));
    CHECK(Perto(meio.onde, V(650.0f, 400.0f)));
    CHECK(frente.escala > 1.0f);
    CHECK(tras.escala < 1.0f);
    CHECK(frente.onde.x > meio.onde.x);
    CHECK(tras.onde.x < meio.onde.x);

    CHECK(NaFrente(30.0f, 100.0f, 0.2f));
    CHECK_FALSE(NaFrente(10.0f, 100.0f, 0.2f));
    CHECK_FALSE(NaFrente(-50.0f, 100.0f, 0.2f));
}

TEST_CASE("Helice: as duas fitas, a borda que solta a base e a rosca que anda de lado") {
    using namespace Helice;
    // A fita 1 vem 'sulco' depois da 0; cada par desce um passo.
    CHECK(Angulo(1, 2.0f, 0.3f, 0.5f, 2.5f) - Angulo(0, 2.0f, 0.3f, 0.5f, 2.5f) == doctest::Approx(2.5f));
    const Ponto3 a = DePe(3.0f, 80.0f, 40.0f, 0.0f);
    CHECK(a.x == doctest::Approx(80.0f));
    CHECK(a.y == doctest::Approx(120.0f));
    CHECK(a.z == doctest::Approx(0.0f));
    const Ponto3 b = Deitada(3.0f, 80.0f, 40.0f, Math::Pi / 2.0f);
    CHECK(b.x == doctest::Approx(120.0f));
    CHECK(b.y == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(b.z == doctest::Approx(80.0f));

    // Passar pelo angulo 0 (x maximo) e a borda direita; por pi, a esquerda.
    CHECK(Borda(-0.1f, 0.1f) == 1);
    CHECK(Borda(Math::Pi - 0.1f, Math::Pi + 0.1f) == -1);
    CHECK(Borda(2.0f * Math::Pi - 0.05f, 2.0f * Math::Pi + 0.05f) == 1);
    CHECK(Borda(0.5f, 1.0f) == 0);
    CHECK(Borda(0.1f, -0.1f) == 1);

    CHECK(AndarDaRosca(2.4f, 0.012f) == doctest::Approx(200.0f));
    CHECK(AndarDaRosca(2.4f, 0.0f) == doctest::Approx(0.0f));
}

TEST_CASE("Helice: A-T solta antes de G-C, o vizinho segura, e o par solto incha") {
    using namespace Helice;
    // A corrida de A-T solta primeiro, depois o A-T entre G-C, o G-C entre A-T e, por ultimo, a
    // corrida de G-C.
    CHECK(Fusao(Dna("AAA"), 1) < Fusao(Dna("GAG"), 1));
    CHECK(Fusao(Dna("GAG"), 1) < Fusao(Dna("ACA"), 1));
    CHECK(Fusao(Dna("ACA"), 1) < Fusao(Dna("GCG"), 1));
    // A e T seguram igual.
    CHECK(Fusao(Dna("AAT"), 1) == doctest::Approx(Fusao(Dna("TTT"), 1)));
    CHECK(Fusao(Dna("GAG"), 99) == doctest::Approx(0.0f));
    // Na ponta, so um vizinho conta.
    CHECK(Fusao(Dna("AG"), 0) == doctest::Approx(Fusao(Dna("GAG"), 1)));

    // A temperatura sobe em reta, e o calor solta A-T no comeco e G-C no fim da etapa.
    const Degrau d = DegrauDo(Dificuldade::Nivel::Normal);
    const float dur = kDuracoes[2];
    CHECK(Temperatura(0.0f, dur, d.temperaturaInicial, d.temperaturaFinal) == doctest::Approx(d.temperaturaInicial));
    CHECK(Temperatura(dur, dur, d.temperaturaInicial, d.temperaturaFinal) == doctest::Approx(d.temperaturaFinal));
    CHECK(Fusao(Dna("AAA"), 1) > d.temperaturaInicial);
    CHECK(Fusao(Dna("GCG"), 1) < d.temperaturaFinal);

    CHECK(RaioDoPar(-1.0f, 80.0f, 1.0f, 0.5f) == doctest::Approx(80.0f));
    CHECK(RaioDoPar(0.25f, 80.0f, 1.0f, 0.5f) == doctest::Approx(120.0f));
    CHECK(RaioDoPar(5.0f, 80.0f, 1.0f, 0.5f) == doctest::Approx(160.0f));

    // A fita sorteada vem em blocos de 2 a 4 de cada tipo, alternados.
    Variacao::Sorteio s(3u);
    const auto fita = Sortear(s, 30);
    REQUIRE(fita.size() == 30u);
    int corrida = 1;
    for (size_t k = 1; k < fita.size(); ++k) {
        const bool mesmo = Pareamento::Pontes(fita[k]) == Pareamento::Pontes(fita[k - 1]);
        corrida = mesmo ? corrida + 1 : 1;
        CHECK(corrida <= 4);
    }
}

// ---- A proteina ----

TEST_CASE("Dobramento: a cadeia segue a cabeca no espacamento, e o corte separa a ponta") {
    using namespace Dobramento;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    std::vector<Vector2> contas = {V(0, 0), V(0, 10), V(0, 20), V(0, 30), V(0, 40)};
    Arrastar(contas, V(30.0f, 0.0f), 15.0f);
    CHECK(Perto(contas[0], V(30.0f, 0.0f)));
    for (size_t i = 1; i < contas.size(); ++i) CHECK((contas[i] - contas[i - 1]).Length() == doctest::Approx(15.0f));
    // Duas contas no mesmo lugar nao viram NaN.
    std::vector<Vector2> juntas = {V(5, 5), V(5, 5)};
    Arrastar(juntas, V(5.0f, 5.0f), 10.0f);
    CHECK(Perto(juntas[1], V(5.0f, 15.0f)));

    const auto soltos = Cortar(contas, 2);
    CHECK(contas.size() == 2u);
    CHECK(soltos.size() == 3u);
    CHECK(Cortar(contas, 9).empty());
    CHECK(contas.size() == 2u);
}

TEST_CASE("Dobramento: o globulo esconde os hidrofobicos no nucleo, e o que rola quica na parede") {
    using namespace Dobramento;
    const std::vector<bool> h = {true, false, false, true, false, true, false, false, false, true};
    const auto g = Globulo(h, 0.35f);
    REQUIRE(g.size() == h.size());
    for (size_t i = 0; i < h.size(); ++i) {
        const float r = std::sqrt(g[i].x * g[i].x + g[i].y * g[i].y + g[i].z * g[i].z);
        CAPTURE(i);
        if (h[i]) CHECK(r <= 0.35f + 1e-4f);
        else CHECK(r == doctest::Approx(1.0f).epsilon(1e-3));
    }
    // Os da superficie nao caem no mesmo lugar.
    for (size_t i = 0; i < g.size(); ++i) {
        for (size_t j = i + 1; j < g.size(); ++j) {
            if (h[i] || h[j]) continue;
            CHECK(std::hypot(g[i].x - g[j].x, g[i].y - g[j].y, g[i].z - g[j].z) > 0.3f);
        }
    }

    Vector2 v = V(200.0f, -100.0f);
    const Vector2 p = Quicar(V(95.0f, 35.0f), v, 0.1f, 30.0f, V(0.0f, 0.0f), V(100.0f, 400.0f));
    CHECK(p.x == doctest::Approx(70.0f));
    CHECK(p.y == doctest::Approx(30.0f));
    CHECK(v.x < 0.0f);
    CHECK(v.y > 0.0f);
}

TEST_CASE("Dobramento: o sitio deixa a boca aberta, o encaixe e la dentro, e o Delta G cai com o tempo") {
    using namespace Dobramento;
    const Vector2 c = V(500.0f, 400.0f);
    const float boca = Math::Pi / 2.0f, abertura = 1.0f;
    const auto contas = Sitio(c, 80.0f, 20, boca, abertura);
    REQUIRE(contas.size() == 20u);
    for (const Vector2& p : contas) {
        CHECK((p - c).Length() == doctest::Approx(80.0f));
        // Nenhuma conta dentro da boca (embaixo do centro).
        float a = Math::Atan2(p.y - c.y, p.x - c.x) - boca;
        while (a > Math::Pi) a -= Math::TwoPi;
        while (a < -Math::Pi) a += Math::TwoPi;
        CHECK(std::fabs(a) >= 0.5f * abertura - 1e-3f);
    }
    CHECK(Encaixado(V(510.0f, 410.0f), c, 80.0f, 12.0f));
    CHECK_FALSE(Encaixado(V(575.0f, 400.0f), c, 80.0f, 12.0f));

    CHECK(DeltaG(0.0f) < 0.0f);
    CHECK(DeltaG(3.0f) < DeltaG(1.0f));
    CHECK(DeltaG(100.0f) == doctest::Approx(-15.0f));

    // Sem chute, o browniano volta para a media.
    Vector2 v = V(300.0f, 0.0f);
    for (int k = 0; k < 200; ++k) v = Browniano(v, V(0.0f, 100.0f), V(0.0f, 0.0f), 0.016f, 3.0f);
    CHECK(Perto(v, V(0.0f, 100.0f), 1.0f));
}

// ---- O sequenciamento ----

TEST_CASE("Sequenciamento: o ciclo da PCR passa por 95, 55 e 72 graus, e as copias dobram ate o teto") {
    using namespace Sequenciamento;
    CHECK(kDuracoes[0] + kDuracoes[1] + kDuracoes[2] == doctest::Approx(Fase::kDuracao));
    CHECK(CicloNoTempo(0.1f, 1.0f).passo == Passo::Desnatura);
    CHECK(CicloNoTempo(0.1f, 1.0f).temperatura == doctest::Approx(95.0f));
    CHECK(CicloNoTempo(0.4f, 1.0f).passo == Passo::Anela);
    CHECK(CicloNoTempo(0.4f, 1.0f).temperatura == doctest::Approx(55.0f));
    CHECK(CicloNoTempo(0.8f, 1.0f).passo == Passo::Estende);
    CHECK(CicloNoTempo(0.8f, 1.0f).temperatura == doctest::Approx(72.0f));
    CHECK(CicloNoTempo(3.2f, 1.0f).numero == 3);
    CHECK(CicloNoTempo(3.2f, 1.0f).passo == Passo::Desnatura);

    CHECK(Copias(2, 0, 40) == 2);
    CHECK(Copias(2, 3, 40) == 16);
    CHECK(Copias(2, 6, 40) == 40);
    CHECK(Copias(1, 30, 1000000) == 1000000);
}

TEST_CASE("Sequenciamento: o contig e um ponteiro de contas, e o read encosta na ponta mais perto") {
    using namespace Sequenciamento;
    const Vector2 c = V(600.0f, 300.0f);
    const auto contas = Contig(c, 2, 3, 20.0f, 0.0f);
    REQUIRE(contas.size() == 6u);
    CHECK(Perto(contas.front(), V(560.0f, 300.0f)));
    CHECK(Perto(contas.back(), V(660.0f, 300.0f)));
    CHECK(Perto(Contig(c, 0, 1, 20.0f, Math::Pi / 2.0f).back(), V(600.0f, 320.0f), 1e-2f));

    CHECK(DistanciaAoSegmento(V(5.0f, 7.0f), V(0.0f, 0.0f), V(10.0f, 0.0f)) == doctest::Approx(7.0f));
    CHECK(DistanciaAoSegmento(V(-3.0f, 4.0f), V(0.0f, 0.0f), V(10.0f, 0.0f)) == doctest::Approx(5.0f));
    CHECK(DistanciaAoSegmento(V(3.0f, 4.0f), V(0.0f, 0.0f), V(0.0f, 0.0f)) == doctest::Approx(5.0f));

    CHECK(Encosta(V(650.0f, 305.0f), c, 2, 3, 20.0f, 0.0f, 12.0f) == 1);
    CHECK(Encosta(V(570.0f, 295.0f), c, 2, 3, 20.0f, 0.0f, 12.0f) == -1);
    CHECK(Encosta(V(650.0f, 330.0f), c, 2, 3, 20.0f, 0.0f, 12.0f) == 0);
    CHECK(Encosta(V(700.0f, 300.0f), c, 2, 3, 20.0f, 0.0f, 12.0f) == 0);
}

TEST_CASE("Sequenciamento: o genoma tem o guia + PAM num lugar so, e as iscas erram uma base") {
    using namespace Sequenciamento;
    CHECK(Pam(Dna("AGG"), 0));
    CHECK(Pam(Dna("TCGG"), 1));
    CHECK_FALSE(Pam(Dna("AGC"), 0));
    CHECK_FALSE(Pam(Dna("AG"), 0));
    int quantas = 0;
    CHECK(Procurar(Dna("TTGACTAGGCC"), Dna("GACT"), &quantas) == 2);
    CHECK(quantas == 1);
    CHECK(Procurar(Dna("TTGACTACCCC"), Dna("GACT")) == -1);

    Variacao::Sorteio s(19u);
    for (int vez = 0; vez < 40; ++vez) {
        const auto guia = Guia(s, 4);
        REQUIRE(guia.size() == 4u);
        CHECK_FALSE(std::all_of(guia.begin(), guia.end(), [](const Base b) { return b == Base::G; }));
        const Genoma g = Montar(s, 26, guia, 3);
        REQUIRE(g.fita.size() == 26u);
        CAPTURE(vez);
        int n = 0;
        CHECK(Procurar(g.fita, guia, &n) == g.alvo);
        CHECK(n == 1);
        CHECK(Pam(g.fita, g.alvo + 4));
        for (const int i : g.iscas) {
            CHECK(Pam(g.fita, i + 4));
            int erros = 0;
            for (int k = 0; k < 4; ++k) erros += g.fita[static_cast<size_t>(i + k)] != guia[static_cast<size_t>(k)] ? 1 : 0;
            CHECK(erros == 1);
            CHECK(std::abs(i - g.alvo) >= 8);
        }
    }
}

TEST_CASE("Sabrina: o Dificil aperta cada prova") {
    const auto n = Dificuldade::Nivel::Normal, d = Dificuldade::Nivel::Dificil;
    CHECK(Helice::DegrauDo(d).giro > Helice::DegrauDo(n).giro);
    CHECK(Helice::DegrauDo(d).limiar < Helice::DegrauDo(n).limiar);
    CHECK(Helice::DegrauDo(d).periodoDaDescida < Helice::DegrauDo(n).periodoDaDescida);
    CHECK(Pareamento::DegrauDo(d).entreOndas < Pareamento::DegrauDo(n).entreOndas);
    CHECK(Pareamento::DegrauDo(d).porCodon < Pareamento::DegrauDo(n).porCodon);
    CHECK(Pareamento::DegrauDo(d).fracaoDominante < Pareamento::DegrauDo(n).fracaoDominante);
    CHECK(Dobramento::DegrauDo(d).rapidez > Dobramento::DegrauDo(n).rapidez);
    CHECK(Dobramento::DegrauDo(d).resistencia > Dobramento::DegrauDo(n).resistencia);
    CHECK(Dobramento::DegrauDo(d).premioPorSegundo < Dobramento::DegrauDo(n).premioPorSegundo);
    CHECK(Sequenciamento::DegrauDo(d).periodo < Sequenciamento::DegrauDo(n).periodo);
    CHECK(Sequenciamento::DegrauDo(d).tamanhoDoGuia > Sequenciamento::DegrauDo(n).tamanhoDoGuia);
    CHECK(Sequenciamento::DegrauDo(d).giro > Sequenciamento::DegrauDo(n).giro);
}
