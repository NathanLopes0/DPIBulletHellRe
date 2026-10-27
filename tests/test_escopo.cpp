//
// O escopo da fase 1 do Luizinho.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <cstdint>
#include <vector>

#include "../Source/Attacks/Escopo.h"

using namespace Escopo;

namespace {
    constexpr Forma kTodas[kFormas] = {Forma::XEmG, Forma::XNoInterno, Forma::YNoInterno,
                                       Forma::YEmH, Forma::FEstatico, Forma::FDinamico};
    constexpr Bloco kTodos[kBlocos] = {Bloco::Global, Bloco::F, Bloco::G, Bloco::Interno, Bloco::H};

    /// Se o aluno parado em (px, py) e alcancado por alguma gota enquanto ela
    /// esta acesa: a gota anda da altura em que nasce ate kQueda * kVivas mais
    /// para baixo, ou ate o fim dela.
    bool Alcancado(const std::vector<Gota>& gotas, const float px, const float py) {
        const float alcance = kRaioDaGota + kRaioDoAluno;
        for (const Gota& g : gotas) {
            const float y0 = g.y;
            const float y1 = std::min(g.y + kQueda * kVivas, g.fim);
            const float yPerto = std::clamp(py, y0, y1);
            const float dx = px - g.x;
            const float dy = py - yPerto;
            if (dx * dx + dy * dy <= alcance * alcance) return true;
        }
        return false;
    }

    /// A distancia de (px, py) a borda mais perto de um retangulo.
    float ABorda(const Retangulo& r, const float px, const float py) {
        if (r.Contem(px, py)) {
            return std::min({px - r.x, r.Direita() - px, py - r.y, r.Baixo() - py});
        }
        const float dx = std::max({r.x - px, 0.0f, px - r.Direita()});
        const float dy = std::max({r.y - py, 0.0f, py - r.Baixo()});
        return std::sqrt(dx * dx + dy * dy);
    }
}

TEST_CASE("Escopo: a busca sobe pelo texto no estatico e pela pilha no dinamico") {
    CHECK(Busca(Forma::XEmG) == std::vector<Bloco>{Bloco::G, Bloco::Global});
    CHECK(Busca(Forma::XNoInterno) == std::vector<Bloco>{Bloco::Interno});
    CHECK(Busca(Forma::YNoInterno) == std::vector<Bloco>{Bloco::Interno, Bloco::G});
    CHECK(Busca(Forma::YEmH) == std::vector<Bloco>{Bloco::H, Bloco::Global});
    CHECK(Busca(Forma::FEstatico) == std::vector<Bloco>{Bloco::F, Bloco::Global});
    CHECK(Busca(Forma::FDinamico) == std::vector<Bloco>{Bloco::F, Bloco::H});

    // As duas chamadas sao o MESMO codigo; so a regra muda.
    CHECK(Placar(Forma::FEstatico).substr(Placar(Forma::FEstatico).find('\n'))
          == Placar(Forma::FDinamico).substr(Placar(Forma::FDinamico).find('\n')));
    CHECK(QuemChamou(Forma::FEstatico) == Bloco::H);
    CHECK(QuemChamou(Forma::FDinamico) == Bloco::H);
    CHECK_FALSE(QuemChamou(Forma::XEmG).has_value());

    for (const Forma f : kTodas) {
        INFO(Placar(f));
        CHECK(Declara(Dono(f), Nome(f)));
        // Ninguem antes do dono declara o nome.
        const auto busca = Busca(f);
        for (size_t i = 0; i + 1 < busca.size(); ++i) CHECK_FALSE(Declara(busca[i], Nome(f)));
        // A busca anda um bloco por salto, e acaba antes de a chuva nascer.
        CHECK(static_cast<float>(busca.size()) * kPorSalto <= kBusca + 1e-4f);
    }
}

TEST_CASE("Escopo: os buracos sao os blocos de dentro que declaram outro com o mesmo nome") {
    CHECK(Ocultam(Bloco::Global, 'x') == std::vector<Bloco>{Bloco::Interno, Bloco::H});
    // O bloco interno esta dentro de g, que ja tem o seu y: so g conta.
    CHECK(Ocultam(Bloco::Global, 'y') == std::vector<Bloco>{Bloco::G});
    CHECK(Ocultam(Bloco::G, 'y').empty());
    CHECK(Ocultam(Bloco::Interno, 'x').empty());
    CHECK(Ocultam(Bloco::H, 'x').empty());
}

TEST_CASE("Escopo: os paineis cabem no campo, um dentro do outro como no texto") {
    const Retangulo campo = Area(Bloco::Global);
    for (const Bloco b : kTodos) {
        const Retangulo r = Area(b);
        CHECK(r.x >= campo.x);
        CHECK(r.Direita() <= campo.Direita());
        CHECK(r.y >= campo.y);
        CHECK(r.Baixo() <= campo.Baixo());
    }
    const Retangulo g = Area(Bloco::G);
    const Retangulo i = Area(Bloco::Interno);
    const Retangulo h = Area(Bloco::H);
    const Retangulo f = Area(Bloco::F);
    CHECK(i.x > g.x);
    CHECK(i.Direita() < g.Direita());
    CHECK(i.y > g.y);
    // Os irmaos nao se encostam.
    CHECK(g.Direita() < h.x);
    CHECK(f.Baixo() < g.y);
    CHECK(f.Baixo() < h.y);

    // OS PAINEIS DE BAIXO VAO ATE O CHAO: o aluno pode ficar dentro deles.
    CHECK(g.Baixo() == doctest::Approx(campo.Baixo()));
    CHECK(i.Baixo() == doctest::Approx(campo.Baixo()));
    CHECK(h.Baixo() == doctest::Approx(campo.Baixo()));

    // E escalam com o campo.
    const Retangulo dobro = Area(Bloco::H, 2.0f * kLargura, 2.0f * kAltura);
    CHECK(dobro.x == doctest::Approx(2.0f * h.x));
    CHECK(dobro.h == doctest::Approx(2.0f * h.h));
}

TEST_CASE("Escopo: o canto de cada painel fica dentro dele, e nenhum embaixo do placar") {
    for (const Bloco b : kTodos) {
        INFO(Declaracoes(b));
        const Retangulo r = Area(b);
        const Ponto c = Canto(b);
        const float w = LarguraDoCanto(Declaracoes(b));
        CHECK(c.x - w / 2.0f >= r.x);
        CHECK(c.x + w / 2.0f <= r.Direita());
        CHECK(c.y - kAlturaDoCanto / 2.0f >= r.y);
        CHECK(c.y + kAlturaDoCanto / 2.0f <= r.Baixo());
        // O placar ocupa o canto de cima a esquerda (Battle: 12 + 320 por 12 + 46).
        CHECK_FALSE((c.x - w / 2.0f < 340.0f && c.y - kAlturaDoCanto / 2.0f < 64.0f));
    }
    // Nenhum canto em cima de outro.
    for (const Bloco a : kTodos) for (const Bloco b : kTodos) {
        if (a == b) continue;
        const Ponto ca = Canto(a);
        const Ponto cb = Canto(b);
        const float wa = LarguraDoCanto(Declaracoes(a));
        const float wb = LarguraDoCanto(Declaracoes(b));
        const bool encostam = std::fabs(ca.x - cb.x) < (wa + wb) / 2.0f
                           && std::fabs(ca.y - cb.y) < kAlturaDoCanto;
        CHECK_FALSE(encostam);
    }
}

TEST_CASE("Escopo: o professor fica no bloco em que o comando roda") {
    for (const Forma f : kTodas) {
        INFO(Placar(f));
        const Ponto p = OndeFicaOProfessor(f);
        if (Onde(f) == Bloco::F) {
            // Embaixo do cartao de f, fora dos paineis de g e de h.
            CHECK(p.y > Area(Bloco::F).Baixo());
            CHECK_FALSE(Area(Bloco::G).Contem(p.x, p.y));
            CHECK_FALSE(Area(Bloco::H).Contem(p.x, p.y));
            continue;
        }
        CHECK(Area(Onde(f)).Contem(p.x, p.y));
        // O CARTAO DELE NAO COBRE A DECLARACAO do bloco, nem passa do chao.
        CHECK(p.y - kMeiaAlturaDoProfessor > Canto(Onde(f)).y + kAlturaDoCanto / 2.0f);
        CHECK(p.y + kMeiaAlturaDoProfessor < Area(Onde(f)).Baixo());
        // No g, fora do bloco interno: o comando e de g.
        if (Onde(f) == Bloco::G) CHECK_FALSE(Area(Bloco::Interno).Contem(p.x, p.y));
    }
}

TEST_CASE("Escopo: a chuva cobre a regiao inteira e nao toca nos buracos") {
    // PONTO A PONTO, a cada 4 px: quem esta parado na regiao em que a variavel
    // e visivel e alcancado por alguma gota; quem esta num buraco, ou fora do
    // painel do dono, nunca. So a faixa rente as bordas fica de fora da
    // conferencia da cobertura (ali a gota so alcanca de raspao). Fora do
    // painel do dono, so o aluno com o corpo em cima da borda e alcancado (a
    // gota encosta nela por dentro); no buraco, que tem a folga, nem ele.
    //
    // O aluno nunca passa de y = 698 (Player: o chao menos a altura do sprite).
    constexpr float kRente = 6.0f;
    constexpr float kChao = 700.0f;

    for (const Forma f : kTodas) {
        INFO(Placar(f));
        const auto gotas = Gotas(f);
        const Retangulo dono = Area(Dono(f));
        std::vector<Retangulo> buracos;
        for (const Bloco b : Ocultam(Dono(f), Nome(f))) buracos.push_back(Area(b));

        int descobertos = 0;
        int invadidos = 0;
        for (float y = 1.0f; y <= kChao; y += 4.0f) {
            for (float x = 1.0f; x < kLargura; x += 4.0f) {
                float borda = ABorda(dono, x, y);
                bool noBuraco = false;
                for (const Retangulo& b : buracos) {
                    borda = std::min(borda, ABorda(b, x, y));
                    if (b.Contem(x, y)) noBuraco = true;
                }
                const float tolerancia = noBuraco ? 1.5f : kRaioDoAluno + 0.5f;
                const bool visivel = dono.Contem(x, y) && !noBuraco;
                CHECK(visivel == Visivel(f, x, y));
                if (visivel && borda > kRente && !Alcancado(gotas, x, y)) ++descobertos;
                if (!visivel && borda > tolerancia && Alcancado(gotas, x, y)) ++invadidos;
            }
        }
        CHECK(descobertos == 0);
        CHECK(invadidos == 0);
    }
}

TEST_CASE("Escopo: as gotas morrem no buraco de baixo ou no fim do painel") {
    for (const Forma f : kTodas) {
        INFO(Placar(f));
        const Retangulo dono = Area(Dono(f));
        const auto gotas = Gotas(f);
        CHECK_FALSE(gotas.empty());
        // O MAIOR DOS CASOS (o global menos dois buracos) cabe no pool sem
        // criar projetil no meio da luta: cada tipo pre-aquece 300
        // (Battle::kProjectilePrewarmCountPerType).
        CHECK(gotas.size() <= 300u);
        for (const Gota& g : gotas) {
            CHECK(g.x >= dono.x + kRaioDaGota - 0.01f);
            CHECK(g.x <= dono.Direita() - kRaioDaGota + 0.01f);
            CHECK(g.fim >= g.y);
            CHECK(g.fim <= dono.Baixo() - kRaioDaGota + 0.01f);
        }
    }

    // Uma gota em cima do bloco interno, no x do global: para antes dele.
    const Retangulo interno = Area(Bloco::Interno);
    bool achou = false;
    for (const Gota& g : Gotas(Forma::XEmG)) {
        if (g.x > interno.x && g.x < interno.Direita() && g.y < interno.y) {
            CHECK(g.fim == doctest::Approx(interno.y - kRaioDaGota - kFolga));
            achou = true;
        }
    }
    CHECK(achou);
}

TEST_CASE("Escopo: quem fica parado no meio de baixo leva chuva em metade dos comandos") {
    // O piloto parado das medicoes fica em (600, 682): entre g e h, no global.
    std::set<Forma> molhado;
    for (const Forma f : kTodas) if (Visivel(f, 600.0f, 682.0f)) molhado.insert(f);
    CHECK(molhado == std::set<Forma>{Forma::XEmG, Forma::YEmH, Forma::FEstatico});

    // E NAO HA LUGAR SEGURO EM TODOS: cada ponto do chao e molhado por algum.
    for (float x = 4.0f; x < kLargura; x += 8.0f) {
        bool algum = false;
        for (const Forma f : kTodas) algum = algum || Visivel(f, x, 682.0f);
        CHECK(algum);
    }
}

TEST_CASE("Escopo: o relogio da fase") {
    CHECK(kPrimeiro + static_cast<float>(kComandos) * kPorComando == doctest::Approx(Fase::kDuracao));
    // A chuva de um comando acaba antes de o seguinte abrir.
    CHECK(kBusca + kAviso + kVivas < kPorComando);
    std::set<Forma> vistas;
    for (int v = 0; v < kComandos; ++v) vistas.insert(Proxima(v));
    CHECK(vistas.size() == static_cast<size_t>(kFormas));
    // A pegadinha - o escopo dinamico - e o ultimo.
    CHECK(Proxima(kComandos - 1) == Forma::FDinamico);
    CHECK(Proxima(kComandos) == Proxima(0));

    std::set<std::string> placares;
    for (const Forma f : kTodas) placares.insert(Placar(f));
    CHECK(placares.size() == static_cast<size_t>(kFormas));

    CHECK(CaraDaGota('x') != CaraDaGota('y'));
    CHECK(CaraDoAviso('x') != CaraDaGota('x'));
}

TEST_CASE("Escopo: o degrau do Normal e o de sempre, e o do Dificil acende antes e cai mais depressa") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.aviso == kAviso);
    CHECK(n.vivas == kVivas);
    CHECK(n.queda == kQueda);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.aviso < n.aviso);
    CHECK(d.queda > n.queda);
    // A COLMEIA CONTINUA FECHADA: cada gota varre pelo menos o que varria.
    CHECK(d.queda * d.vivas >= n.queda * n.vivas);
    // E O AVISO AINDA DA PARA SAIR: atravessar a regiao da gota (o raio dela e
    // o do aluno, mais a folga) a 300 px/s leva bem menos que ele.
    CHECK((kRaioDaGota + kRaioDoAluno + kFolga) * 2.0f / 300.0f < d.aviso);
}

TEST_CASE("Escopo: a legenda diz a regra, onde o nome foi achado e o que a chuva quer dizer") {
    using F = Escopo::Forma;
    for (int k = 0; k < Escopo::kFormas; ++k) {
        const auto f = static_cast<F>(k);
        const std::string l = Escopo::Legenda(f);
        CAPTURE(l);
        CHECK(l.find("chuva") != std::string::npos);
        CHECK(l.find(std::string(1, Escopo::Nome(f))) != std::string::npos);
    }
    CHECK(Escopo::Legenda(F::XNoInterno).find("bloco interno de g()") != std::string::npos);
    CHECK(Escopo::Legenda(F::XEmG).find("global") != std::string::npos);
    // A PEGADINHA: a mesma chamada, duas regras, dois donos.
    CHECK(Escopo::Legenda(F::FDinamico).find("din\xc3\xa2mico") != std::string::npos);
    CHECK(Escopo::Legenda(F::FDinamico).find("de h()") != std::string::npos);
    CHECK(Escopo::Legenda(F::FEstatico).find("global") != std::string::npos);
}

TEST_CASE("Escopo: cada luta sorteia o programa, e os comandos seguem as regras da fase") {
    std::set<std::string> programas;
    std::set<std::string> primeiros;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoEscopo);
        const Plano plano = Sortear(s);
        const Programa& p = plano.programa;
        // O GLOBAL TEM OS DOIS, F NENHUM, H O x, O INTERNO ALGUM.
        CHECK(p.Declara(Bloco::Global, 'x'));
        CHECK(p.Declara(Bloco::Global, 'y'));
        CHECK_FALSE(p.Declara(Bloco::F, 'x'));
        CHECK_FALSE(p.Declara(Bloco::F, 'y'));
        CHECK(p.Declara(Bloco::H, 'x'));
        CHECK((p.Declara(Bloco::Interno, 'x') || p.Declara(Bloco::Interno, 'y')));

        REQUIRE(plano.comandos.size() == static_cast<size_t>(kComandos));
        std::set<std::pair<int, char>> buscas;
        std::set<int> donos;
        for (int k = 0; k < 4; ++k) {
            const Comando& c = plano.comandos[static_cast<size_t>(k)];
            CHECK(c.regra == Regra::Estatico);
            CHECK_FALSE(c.quemChamou.has_value());
            buscas.insert({static_cast<int>(c.onde), c.nome});
            donos.insert(static_cast<int>(Dono(p, c)));
        }
        CHECK(buscas.size() == 4u);
        CHECK(donos.size() >= 2u);

        // A PEGADINHA FECHA, e as duas regras acham donos diferentes.
        const Comando& estatico = plano.comandos[4];
        const Comando& dinamico = plano.comandos[5];
        CHECK(estatico.regra == Regra::Estatico);
        CHECK(dinamico.regra == Regra::Dinamico);
        CHECK(Dono(p, estatico) == Bloco::Global);
        CHECK(Dono(p, dinamico) == Bloco::H);

        // TODA CHUVA DA LUTA CABE NO POOL.
        for (const Comando& c : plano.comandos) CHECK(Gotas(p, c).size() <= kMaisGotas);
        // ALGUEM ALEM DO GLOBAL DECLARA y: o global nunca chove sem buraco.
        CHECK((p.Declara(Bloco::G, 'y') || p.Declara(Bloco::Interno, 'y') || p.Declara(Bloco::H, 'y')));

        programas.insert(Declaracoes(p, Bloco::G) + "|" + Declaracoes(p, Bloco::Interno) + "|" +
                         Declaracoes(p, Bloco::H));
        primeiros.insert(Placar(p, plano.comandos[0]));
    }
    // 4 x 3 x 2 PROGRAMAS, menos os 2 em que so o global declararia y (ali h
    // passa a declarar, e o programa vira um dos outros) - e a primeira busca
    // nao e sempre a mesma.
    CHECK(programas.size() == 22u);

    CHECK(primeiros.size() >= 10u);
}

TEST_CASE("Escopo: o placar e o canto escrevem o programa sorteado") {
    Programa p = Programa::DeSempre();
    const auto i = [](const Bloco b) { return static_cast<size_t>(b); };
    p.y[i(Bloco::G)] = false;                       // g nao declara nada
    p.y[i(Bloco::Interno)] = true;                  // o interno declara os dois
    CHECK(Declaracoes(p, Bloco::G) == "void g() {");
    CHECK(Declaracoes(p, Bloco::Interno) == "{ int x, y;");
    CHECK(Placar(p, Comando{Bloco::G, 'x', Regra::Estatico, std::nullopt}) == "g() { x++; }");
    CHECK(Placar(p, Comando{Bloco::Interno, 'y', Regra::Estatico, std::nullopt}) == "g() { { int x, y;  y++; } }");
    // E A BUSCA SEGUE O PROGRAMA: o y do interno agora e dele mesmo.
    CHECK(Dono(p, Comando{Bloco::Interno, 'y', Regra::Estatico, std::nullopt}) == Bloco::Interno);
    CHECK(Dono(Programa::DeSempre(), Comando{Bloco::Interno, 'y', Regra::Estatico, std::nullopt}) == Bloco::G);
}

TEST_CASE("Escopo: em todo programa sorteado a chuva cobre a regiao, nao toca nos buracos e cabe no pool") {
    constexpr float kRente = 6.0f;
    constexpr float kChao = 700.0f;
    for (std::uint32_t semente = 0; semente < 24; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoEscopo);
        const Plano plano = Sortear(s);
        for (const Comando& c : plano.comandos) {
            const Programa& p = plano.programa;
            const std::string caso = Declaracoes(p, Bloco::G) + " | " + Declaracoes(p, Bloco::Interno) + " | " +
                                     Placar(p, c);
            INFO(caso);

            const auto gotas = Gotas(p, c);
            CHECK_FALSE(gotas.empty());
            CHECK(gotas.size() <= 300u);
            const Retangulo dono = Area(Dono(p, c));
            std::vector<Retangulo> buracos;
            for (const Bloco b : Ocultam(p, Dono(p, c), c.nome)) buracos.push_back(Area(b));
            int descobertos = 0, invadidos = 0;
            for (float y = 3.0f; y <= kChao; y += 8.0f) {
                for (float x = 3.0f; x < kLargura; x += 8.0f) {
                    float borda = ABorda(dono, x, y);
                    bool noBuraco = false;
                    for (const Retangulo& b : buracos) {
                        borda = std::min(borda, ABorda(b, x, y));
                        if (b.Contem(x, y)) noBuraco = true;
                    }
                    const float tolerancia = noBuraco ? 1.5f : kRaioDoAluno + 0.5f;
                    const bool visivel = dono.Contem(x, y) && !noBuraco;
                    CHECK(visivel == Visivel(p, c, x, y));
                    if (visivel && borda > kRente && !Alcancado(gotas, x, y)) ++descobertos;
                    if (!visivel && borda > tolerancia && Alcancado(gotas, x, y)) ++invadidos;
                }
            }
            CHECK(descobertos == 0);
            CHECK(invadidos == 0);
        }
    }
}
