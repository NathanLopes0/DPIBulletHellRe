//
// O escopo da fase 1 do Luizinho.
//

#include "Escopo.h"

#include <algorithm>
#include <cmath>

namespace Escopo {

    namespace {
        /// Os paineis no campo de referencia (1200 x 746). O global e o campo;
        /// os de g, do bloco interno e de h descem ate o chao, para o aluno
        /// poder ficar dentro deles.
        Retangulo Referencia(const Bloco b) {
            switch (b) {
                case Bloco::Global:  return {0.0f, 0.0f, kLargura, kAltura};
                case Bloco::F:       return {500.0f, 70.0f, 240.0f, 44.0f};
                case Bloco::G:       return {30.0f, 250.0f, 540.0f, 496.0f};
                case Bloco::Interno: return {290.0f, 420.0f, 250.0f, 326.0f};
                case Bloco::H:       return {690.0f, 300.0f, 480.0f, 446.0f};
            }
            return {};
        }

        /// A largura do canto com a declaracao: um tanto por letra, mais a
        /// margem. O Luizinho desenha a caixa com esta mesma conta.
        constexpr float kPorLetra = 9.5f;
        constexpr float kMargemDoCanto = 18.0f;
        constexpr float kRecuoDoCanto = 8.0f;

        bool Dentro(const Bloco b, const Bloco dono) {
            // Se 'b' esta (estritamente) dentro de 'dono' no texto.
            Bloco atual = b;
            while (atual != Bloco::Global) {
                atual = Pai(atual);
                if (atual == dono) return true;
            }
            return false;
        }

        constexpr Bloco kTodos[kBlocos] = {Bloco::Global, Bloco::F, Bloco::G, Bloco::Interno, Bloco::H};
    }

    bool Retangulo::Contem(const float px, const float py) const {
        return px >= x && px <= Direita() && py >= y && py <= Baixo();
    }

    Bloco Pai(const Bloco b) {
        switch (b) {
            case Bloco::Interno: return Bloco::G;
            default: return Bloco::Global;
        }
    }

    bool Declara(const Bloco b, const char nome) {
        return Programa::DeSempre().Declara(b, nome);
    }

    std::string Declaracoes(const Bloco b) {
        return Declaracoes(Programa::DeSempre(), b);
    }

    Retangulo Area(const Bloco b, const float largura, const float altura) {
        const Retangulo r = Referencia(b);
        const float ex = largura / kLargura;
        const float ey = altura / kAltura;
        return {r.x * ex, r.y * ey, r.w * ex, r.h * ey};
    }

    Bloco Onde(const Forma f) {
        switch (f) {
            case Forma::XEmG:       return Bloco::G;
            case Forma::XNoInterno: return Bloco::Interno;
            case Forma::YNoInterno: return Bloco::Interno;
            case Forma::YEmH:       return Bloco::H;
            case Forma::FEstatico:  return Bloco::F;
            case Forma::FDinamico:  return Bloco::F;
        }
        return Bloco::Global;
    }

    std::optional<Bloco> QuemChamou(const Forma f) {
        if (f == Forma::FEstatico || f == Forma::FDinamico) return Bloco::H;
        return std::nullopt;
    }

    char Nome(const Forma f) {
        return (f == Forma::YNoInterno || f == Forma::YEmH) ? 'y' : 'x';
    }

    Regra RegraDe(const Forma f) {
        return f == Forma::FDinamico ? Regra::Dinamico : Regra::Estatico;
    }

    std::vector<Bloco> Busca(const Forma f) {
        return Busca(Programa::DeSempre(), DaForma(f));
    }

    Bloco Dono(const Forma f) {
        return Dono(Programa::DeSempre(), DaForma(f));
    }

    std::vector<Bloco> Ocultam(const Bloco dono, const char nome) {
        return Ocultam(Programa::DeSempre(), dono, nome);
    }

    bool Visivel(const Forma f, const float px, const float py, const float largura, const float altura) {
        return Visivel(Programa::DeSempre(), DaForma(f), px, py, largura, altura);
    }

    Forma Proxima(const int vez) {
        const int k = ((vez % kFormas) + kFormas) % kFormas;
        return static_cast<Forma>(k);
    }

    std::string Placar(const Forma f) {
        return Placar(Programa::DeSempre(), DaForma(f));
    }

    Ponto OndeFicaOProfessor(const Forma f, const float largura, const float altura) {
        return OndeFicaOProfessor(DaForma(f), largura, altura);
    }

    float LarguraDoCanto(const std::string& texto) {
        return static_cast<float>(texto.size()) * kPorLetra + kMargemDoCanto;
    }

    Ponto Canto(const Bloco b, const float largura, const float altura) {
        return Canto(Programa::DeSempre(), b, largura, altura);
    }

    std::vector<Gota> Gotas(const Forma f, const float largura, const float altura) {
        return Gotas(Programa::DeSempre(), DaForma(f), largura, altura);
    }

    std::string CaraDoAviso(const char nome) {
        return nome == 'y' ? "AvisoY" : "AvisoX";
    }

    std::string CaraDaGota(const char nome) {
        return nome == 'y' ? "Y" : "X";
    }


    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.aviso = Dificuldade::Escolher(n, kAviso, 0.72f);
        d.queda = Dificuldade::Escolher(n, kQueda, 150.0f);
        return d;
    }
    std::string Legenda(const Forma f) {
        return Legenda(Programa::DeSempre(), DaForma(f));
    }
    // ---- O PROGRAMA E OS COMANDOS DE CADA LUTA ------------------------------

    bool Programa::Declara(const Bloco b, const char nome) const {
        const auto i = static_cast<size_t>(b);
        return nome == 'x' ? x[i] : (nome == 'y' ? y[i] : false);
    }

    Programa Programa::DeSempre() {
        Programa p;
        auto i = [](const Bloco b) { return static_cast<size_t>(b); };
        p.x[i(Bloco::Global)] = p.y[i(Bloco::Global)] = true;
        p.y[i(Bloco::G)] = true;
        p.x[i(Bloco::Interno)] = true;
        p.x[i(Bloco::H)] = true;
        return p;
    }

    Comando DaForma(const Forma f) {
        return Comando{Onde(f), Nome(f), RegraDe(f), QuemChamou(f)};
    }

    std::string DeclaracaoDe(const Programa& p, const Bloco b) {
        const bool x = p.Declara(b, 'x');
        const bool y = p.Declara(b, 'y');
        if (x && y) return "int x, y;";
        if (x) return "int x;";
        if (y) return "int y;";
        return "";
    }

    std::string Declaracoes(const Programa& p, const Bloco b) {
        const std::string d = DeclaracaoDe(p, b);
        const std::string depois = d.empty() ? "" : " " + d;
        switch (b) {
            case Bloco::Global:  return d;
            case Bloco::F:       return "void f() { x++; }";
            case Bloco::G:       return "void g() {" + depois;
            case Bloco::Interno: return "{" + depois;
            case Bloco::H:       return "void h() {" + depois;
        }
        return "";
    }

    std::vector<Bloco> Busca(const Programa& p, const Comando& c) {
        std::vector<Bloco> olhados;
        if (c.regra == Regra::Dinamico) {
            // NO DINAMICO A BUSCA SEGUE A PILHA: o bloco que roda, quem o
            // chamou, e por fim o global (quem chamou h).
            std::vector<Bloco> pilha = {c.onde};
            if (c.quemChamou) pilha.push_back(*c.quemChamou);
            pilha.push_back(Bloco::Global);
            for (const Bloco b : pilha) {
                olhados.push_back(b);
                if (p.Declara(b, c.nome)) break;
            }
            return olhados;
        }
        // NO ESTATICO, O TEXTO: de dentro para fora, ate o global.
        Bloco b = c.onde;
        while (true) {
            olhados.push_back(b);
            if (p.Declara(b, c.nome) || b == Bloco::Global) break;
            b = Pai(b);
        }
        return olhados;
    }

    Bloco Dono(const Programa& p, const Comando& c) {
        return Busca(p, c).back();
    }

    std::vector<Bloco> Ocultam(const Programa& p, const Bloco dono, const char nome) {
        std::vector<Bloco> buracos;
        for (const Bloco b : kTodos) {
            if (b == dono || !Dentro(b, dono) || !p.Declara(b, nome)) continue;
            // SO OS DE FORA: se um bloco entre ele e o dono ja declara o nome,
            // ele esta dentro desse buraco.
            bool porFora = true;
            for (Bloco meio = Pai(b); meio != dono && meio != Bloco::Global; meio = Pai(meio)) {
                if (p.Declara(meio, nome)) { porFora = false; break; }
            }
            if (porFora) buracos.push_back(b);
        }
        return buracos;
    }

    bool Visivel(const Programa& p, const Comando& c, const float px, const float py,
                 const float largura, const float altura) {
        const Bloco dono = Dono(p, c);
        if (!Area(dono, largura, altura).Contem(px, py)) return false;
        for (const Bloco b : Ocultam(p, dono, c.nome)) {
            if (Area(b, largura, altura).Contem(px, py)) return false;
        }
        return true;
    }

    std::string Placar(const Programa& p, const Comando& c) {
        auto comDeclaracao = [&](const Bloco b, const std::string& resto) {
            const std::string d = DeclaracaoDe(p, b);
            return d.empty() ? resto : d + "  " + resto;
        };
        const std::string uso = std::string(1, c.nome) + "++;";
        if (c.quemChamou) {
            // \xc3\xa1 e o a agudo, e \xc3\xa2 o a circunflexo: o MSVC nao le
            // acento em literal.
            const std::string regra = (c.regra == Regra::Dinamico) ? "escopo din\xc3\xa2" "mico"
                                                                   : "escopo est\xc3\xa1" "tico";
            return regra + "\nh() { " + comDeclaracao(Bloco::H, "f();") + " }";
        }
        switch (c.onde) {
            case Bloco::G:       return "g() { " + comDeclaracao(Bloco::G, uso) + " }";
            case Bloco::Interno: return "g() { { " + comDeclaracao(Bloco::Interno, uso) + " } }";
            case Bloco::H:       return "h() { " + comDeclaracao(Bloco::H, uso) + " }";
            case Bloco::F:       return "f() { " + uso + " }";
            case Bloco::Global:  break;
        }
        return uso;
    }

    Ponto OndeFicaOProfessor(const Comando& c, const float largura, const float altura) {
        const Bloco b = c.onde;
        const Retangulo r = Area(b, largura, altura);
        const float ey = altura / kAltura;
        const float ex = largura / kLargura;
        switch (b) {
            case Bloco::G:       return {r.x + 130.0f * ex, r.y + 110.0f * ey};
            case Bloco::Interno: return {r.x + r.w / 2.0f, r.y + 105.0f * ey};
            case Bloco::H:       return {r.x + r.w / 2.0f, r.y + 105.0f * ey};
            case Bloco::F:       return {r.x + r.w / 2.0f, r.Baixo() + 70.0f * ey};
            case Bloco::Global:  break;
        }
        return {largura / 2.0f, 150.0f * ey};
    }

    Ponto Canto(const Programa& p, const Bloco b, const float largura, const float altura) {
        const Retangulo r = Area(b, largura, altura);
        const float w = LarguraDoCanto(Declaracoes(p, b));
        if (b == Bloco::F) return {r.x + r.w / 2.0f, r.y + r.h / 2.0f};
        if (b == Bloco::Global) {
            // O GLOBAL ESCREVE NO CANTO DE CIMA A DIREITA: o da esquerda e do
            // placar.
            return {largura - kRecuoDoCanto - w / 2.0f, kRecuoDoCanto + kAlturaDoCanto / 2.0f};
        }
        return {r.x + kRecuoDoCanto + w / 2.0f, r.y + kRecuoDoCanto + kAlturaDoCanto / 2.0f};
    }

    std::vector<Gota> Gotas(const Programa& p, const Comando& c, const float largura, const float altura) {
        const Bloco dono = Dono(p, c);
        const Retangulo area = Area(dono, largura, altura);
        const float r = kRaioDaGota;
        const float folga = kRaioDaGota + kFolga;

        // OS BURACOS CRESCEM uma gota e a folga: nenhuma gota nasce, nem cai,
        // perto o bastante para alcancar quem esta la dentro.
        std::vector<Retangulo> buracos;
        for (const Bloco b : Ocultam(p, dono, c.nome)) {
            const Retangulo h = Area(b, largura, altura);
            buracos.push_back({h.x - folga, h.y - folga, h.w + 2.0f * folga, h.h + 2.0f * folga});
        }

        const float esquerda = area.x + r;
        const float direita = area.Direita() - r;
        const float baixo = area.Baixo() - r;

        // AS COLUNAS RENTES: a cada borda da regiao e a cada lado de buraco.
        // Sem elas, a colmeia deixaria uma faixa sem chuva ao lado de cada
        // borda, da largura de ate meio passo.
        std::vector<float> rentes = {esquerda, direita};
        for (const Retangulo& b : buracos) {
            if (b.x >= esquerda && b.x <= direita) rentes.push_back(b.x);
            if (b.Direita() >= esquerda && b.Direita() <= direita) rentes.push_back(b.Direita());
        }

        std::vector<Gota> gotas;
        int fileira = 0;
        for (float y = area.y + r; y <= baixo + 0.01f; y += kEntreFileiras, ++fileira) {
            std::vector<float> xs;
            // A PRIMEIRA FILEIRA E CHEIA, com as colunas das duas: a gota so
            // varre para baixo, e a coluna da fileira impar deixaria seca a
            // faixa de cima ate a primeira gota dela.
            const float passo = (fileira == 0) ? kEntreGotas / 2.0f : kEntreGotas;
            const float inicio = esquerda + ((fileira % 2 == 1) ? kEntreGotas / 2.0f : 0.0f);
            for (float x = inicio; x <= direita + 0.01f; x += passo) xs.push_back(x);
            for (const float x : rentes) {
                const bool perto = std::any_of(xs.begin(), xs.end(),
                                               [x](const float o) { return std::fabs(o - x) < 4.0f; });
                if (!perto) xs.push_back(x);
            }
            std::sort(xs.begin(), xs.end());
            for (const float x : xs) {
                bool noBuraco = false;
                float fim = baixo;
                for (const Retangulo& b : buracos) {
                    const bool naColuna = x > b.x && x < b.Direita();
                    if (naColuna && y > b.y && y < b.Baixo()) { noBuraco = true; break; }
                    if (naColuna && b.y >= y) fim = std::min(fim, b.y);
                }
                if (!noBuraco) gotas.push_back({x, y, fim});
            }
        }
        return gotas;
    }

    std::string Legenda(const Programa& p, const Comando& c) {
        // \xc3\xa1 a agudo, \xc3\xa2 a circunflexo, \xc3\xa9 e agudo, \xc3\xad i agudo.
        auto doBloco = [](const Bloco b, const char nome) {
            const std::string n(1, nome);
            switch (b) {
                case Bloco::Global:  return "o " + n + " global";
                case Bloco::F:       return "o " + n + " de f()";
                case Bloco::G:       return "o " + n + " de g()";
                case Bloco::Interno: return "o " + n + " do bloco interno de g()";
                case Bloco::H:       return "o " + n + " de h()";
            }
            return n;
        };
        const std::string n(1, c.nome);
        const std::string achou = doBloco(Dono(p, c), c.nome);
        const std::string chuva = " A chuva cai onde esse " + n + " \xc3\xa9 vis\xc3\xadvel.";
        if (c.regra == Regra::Dinamico) {
            return "Escopo din\xc3\xa2mico: f() procura o " + n + " em quem a chamou, h(), e usa " +
                   achou + "." + chuva;
        }
        if (c.quemChamou) {
            return "Escopo est\xc3\xa1tico: f() procura o " + n + " no texto, fora dela, e usa " + achou +
                   " - mesmo chamada de h()." + chuva;
        }
        return "Escopo est\xc3\xa1tico: " + n + "++ procura de dentro para fora e acha " + achou + "." +
               chuva;
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano plano;
        Programa& p = plano.programa;
        p = Programa{};
        auto i = [](const Bloco b) { return static_cast<size_t>(b); };
        p.x[i(Bloco::Global)] = p.y[i(Bloco::Global)] = true;
        p.x[i(Bloco::H)] = true;
        p.y[i(Bloco::H)] = s.Chance(0.5f);
        const int g = s.Inteiro(0, 3);              // nenhum, x, y, os dois
        p.x[i(Bloco::G)] = (g & 1) != 0;
        p.y[i(Bloco::G)] = (g & 2) != 0;
        const int interno = s.Inteiro(1, 3);        // x, y, os dois
        p.x[i(Bloco::Interno)] = (interno & 1) != 0;
        p.y[i(Bloco::Interno)] = (interno & 2) != 0;
        if (!p.y[i(Bloco::G)] && !p.y[i(Bloco::Interno)] && !p.y[i(Bloco::H)]) p.y[i(Bloco::H)] = true;

        // AS QUATRO BUSCAS, sem repetir. Dois donos, pelo menos, sempre ha: no
        // maximo tres das seis buscas acabam no global.
        std::vector<Comando> candidatos;
        for (const Bloco onde : {Bloco::G, Bloco::Interno, Bloco::H}) {
            for (const char nome : {'x', 'y'}) {
                const Comando c{onde, nome, Regra::Estatico, std::nullopt};
                if (Gotas(p, c).size() <= kMaisGotas) candidatos.push_back(c);
            }
        }

        s.Embaralhar(candidatos);

        plano.comandos.assign(candidatos.begin(), candidatos.begin() + std::min<std::ptrdiff_t>(4, static_cast<std::ptrdiff_t>(candidatos.size())));


        // A PEGADINHA FECHA: f chamada de h, no estatico e logo depois no dinamico.
        plano.comandos.push_back(DaForma(Forma::FEstatico));
        plano.comandos.push_back(DaForma(Forma::FDinamico));
        return plano;
    }
}
