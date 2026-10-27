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
        switch (b) {
            case Bloco::Global:  return nome == 'x' || nome == 'y';
            case Bloco::G:       return nome == 'y';
            case Bloco::Interno: return nome == 'x';
            case Bloco::H:       return nome == 'x';
            case Bloco::F:       return false;
        }
        return false;
    }

    std::string Declaracoes(const Bloco b) {
        switch (b) {
            case Bloco::Global:  return "int x, y;";
            case Bloco::F:       return "void f() { x++; }";
            case Bloco::G:       return "void g() { int y;";
            case Bloco::Interno: return "{ int x;";
            case Bloco::H:       return "void h() { int x;";
        }
        return "";
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
        const char nome = Nome(f);
        std::vector<Bloco> olhados;

        if (RegraDe(f) == Regra::Dinamico) {
            // NO DINAMICO A BUSCA SEGUE A PILHA: o bloco que roda, quem o
            // chamou, e por fim o global (quem chamou h).
            std::vector<Bloco> pilha = {Onde(f)};
            if (const auto quem = QuemChamou(f)) pilha.push_back(*quem);
            pilha.push_back(Bloco::Global);
            for (const Bloco b : pilha) {
                olhados.push_back(b);
                if (Declara(b, nome)) break;
            }
            return olhados;
        }

        // NO ESTATICO, O TEXTO: de dentro para fora, ate o global.
        Bloco b = Onde(f);
        while (true) {
            olhados.push_back(b);
            if (Declara(b, nome) || b == Bloco::Global) break;
            b = Pai(b);
        }
        return olhados;
    }

    Bloco Dono(const Forma f) {
        return Busca(f).back();
    }

    std::vector<Bloco> Ocultam(const Bloco dono, const char nome) {
        std::vector<Bloco> buracos;
        for (const Bloco b : kTodos) {
            if (b == dono || !Dentro(b, dono) || !Declara(b, nome)) continue;
            // SO OS DE FORA: se um bloco entre ele e o dono ja declara o nome,
            // ele esta dentro desse buraco.
            bool porFora = true;
            for (Bloco meio = Pai(b); meio != dono && meio != Bloco::Global; meio = Pai(meio)) {
                if (Declara(meio, nome)) { porFora = false; break; }
            }
            if (porFora) buracos.push_back(b);
        }
        return buracos;
    }

    bool Visivel(const Forma f, const float px, const float py, const float largura, const float altura) {
        const Bloco dono = Dono(f);
        if (!Area(dono, largura, altura).Contem(px, py)) return false;
        for (const Bloco b : Ocultam(dono, Nome(f))) {
            if (Area(b, largura, altura).Contem(px, py)) return false;
        }
        return true;
    }

    Forma Proxima(const int vez) {
        const int k = ((vez % kFormas) + kFormas) % kFormas;
        return static_cast<Forma>(k);
    }

    std::string Placar(const Forma f) {
        switch (f) {
            case Forma::XEmG:       return "g() { int y;  x++; }";
            case Forma::XNoInterno: return "g() { { int x;  x++; } }";
            case Forma::YNoInterno: return "g() { { int x;  y++; } }";
            case Forma::YEmH:       return "h() { int x;  y++; }";
            // \xc3\xa1 e o a agudo, e \xc3\xa2 o a circunflexo: o MSVC nao
            // le acento em literal.
            case Forma::FEstatico:  return "escopo est\xc3\xa1" "tico\nh() { int x;  f(); }";
            case Forma::FDinamico:  return "escopo din\xc3\xa2" "mico\nh() { int x;  f(); }";
        }
        return "";
    }

    Ponto OndeFicaOProfessor(const Forma f, const float largura, const float altura) {
        const Bloco b = Onde(f);
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

    float LarguraDoCanto(const std::string& texto) {
        return static_cast<float>(texto.size()) * kPorLetra + kMargemDoCanto;
    }

    Ponto Canto(const Bloco b, const float largura, const float altura) {
        const Retangulo r = Area(b, largura, altura);
        const float w = LarguraDoCanto(Declaracoes(b));
        if (b == Bloco::F) return {r.x + r.w / 2.0f, r.y + r.h / 2.0f};
        if (b == Bloco::Global) {
            // O GLOBAL ESCREVE NO CANTO DE CIMA A DIREITA: o da esquerda e do
            // placar.
            return {largura - kRecuoDoCanto - w / 2.0f, kRecuoDoCanto + kAlturaDoCanto / 2.0f};
        }
        return {r.x + kRecuoDoCanto + w / 2.0f, r.y + kRecuoDoCanto + kAlturaDoCanto / 2.0f};
    }

    std::vector<Gota> Gotas(const Forma f, const float largura, const float altura) {
        const Retangulo area = Area(Dono(f), largura, altura);
        const float r = kRaioDaGota;
        const float folga = kRaioDaGota + kFolga;

        // OS BURACOS CRESCEM uma gota e a folga: nenhuma gota nasce, nem cai,
        // perto o bastante para alcancar quem esta la dentro.
        std::vector<Retangulo> buracos;
        for (const Bloco b : Ocultam(Dono(f), Nome(f))) {
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

    std::string CaraDoAviso(const char nome) {
        return nome == 'y' ? "AvisoY" : "AvisoX";
    }

    std::string CaraDaGota(const char nome) {
        return nome == 'y' ? "Y" : "X";
    }

    std::vector<int> OrdemDosComandos(Variacao::Sorteio& s) {
        constexpr int kBuscas = static_cast<int>(Forma::FEstatico);
        std::vector<int> ordem = Variacao::Ordem(s, kBuscas, 0);
        for (int k = kBuscas; k < kFormas; ++k) ordem.push_back(k);
        return ordem;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.aviso = Dificuldade::Escolher(n, kAviso, 0.72f);
        d.queda = Dificuldade::Escolher(n, kQueda, 150.0f);
        return d;
    }
    std::string Legenda(const Forma f) {
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
        const char nome = Nome(f);
        const std::string n(1, nome);
        const std::string achou = doBloco(Dono(f), nome);
        const std::string chuva = " A chuva cai onde esse " + n + " \xc3\xa9 vis\xc3\xadvel.";
        if (RegraDe(f) == Regra::Dinamico) {
            return "Escopo din\xc3\xa2mico: f() procura o " + n + " em quem a chamou, h(), e usa " +
                   achou + "." + chuva;
        }
        if (QuemChamou(f)) {
            return "Escopo est\xc3\xa1tico: f() procura o " + n + " no texto, fora dela, e usa " + achou +
                   " - mesmo chamada de h()." + chuva;
        }
        return "Escopo est\xc3\xa1tico: " + n + "++ procura de dentro para fora e acha " + achou + "." +
               chuva;

    }
}
