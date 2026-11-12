//
// A fase 1 do Luizinho: as amarracoes.
//

#include "Amarracao.h"

#include <algorithm>
#include <random>

namespace Amarracao {

namespace {
    size_t Indice(const Bloco b) { return static_cast<size_t>(b); }

    /// Os paineis num campo de referencia de 1200 x 746, com f a esquerda.
    constexpr float kLarguraDeReferencia = 1200.0f;
    constexpr float kAlturaDeReferencia = 746.0f;

    Retangulo Referencia(const Bloco b) {
        switch (b) {
            case Bloco::Global:  return {0.0f, 0.0f, kLarguraDeReferencia, kAlturaDeReferencia};
            case Bloco::F:       return {40.0f, 150.0f, 520.0f, 440.0f};
            case Bloco::Interno: return {90.0f, 330.0f, 330.0f, 220.0f};
            case Bloco::G:       return {660.0f, 150.0f, 500.0f, 300.0f};
        }
        return {};
    }

    /// Onde o professor fica em cada bloco, no campo de referencia.
    Ponto ProfessorDeReferencia(const Bloco b) {
        switch (b) {
            case Bloco::Global:  return {600.0f, 95.0f};
            case Bloco::F:       return {490.0f, 262.0f};
            case Bloco::Interno: return {255.0f, 450.0f};
            case Bloco::G:       return {910.0f, 320.0f};
        }
        return {};
    }

    Retangulo Espelhado(Retangulo r, const bool fAEsquerda) {
        if (!fAEsquerda) r.x = kLarguraDeReferencia - r.x - r.w;
        return r;
    }

    Retangulo Escalado(const Retangulo& r, const float largura, const float altura) {
        const float ex = largura / kLarguraDeReferencia;
        const float ey = altura / kAlturaDeReferencia;
        return {r.x * ex, r.y * ey, r.w * ex, r.h * ey};
    }
}

Bloco Pai(const Bloco b) {
    return b == Bloco::Interno ? Bloco::F : Bloco::Global;
}

bool Programa::Declara(const Bloco b) const { return x[Indice(b)] > 0; }

int Programa::Valor(const Bloco b) const { return x[Indice(b)]; }

Programa SortearPrograma(Variacao::Sorteio& s) {
    std::vector<int> valores;
    for (int v = kMenorValor; v <= kMaiorValor; ++v) valores.push_back(v);
    s.Embaralhar(valores);
    Programa p;
    p.x[Indice(Bloco::Global)] = valores[0];
    p.x[Indice(Bloco::G)] = valores[1];
    if (s.Chance(kChanceDoInterno)) p.x[Indice(Bloco::Interno)] = valores[2];
    return p;
}

Comando Proximo(const std::uint32_t sorteio, const Bloco anterior) {
    std::mt19937 g(sorteio);
    std::vector<Bloco> blocos;
    for (int b = 0; b < kBlocos; ++b) if (static_cast<Bloco>(b) != anterior) blocos.push_back(static_cast<Bloco>(b));
    Comando c;
    c.onde = blocos[g() % blocos.size()];
    return c;
}

Comando Chamada(const Regra r) {
    Comando c;
    c.onde = Bloco::F;
    c.regra = r;
    c.chamada = true;
    return c;
}

std::vector<Bloco> Busca(const Programa& p, const Comando& c) {
    std::vector<Bloco> olhados;
    Bloco atual = c.onde;
    for (int passo = 0; passo < kBlocos; ++passo) {
        olhados.push_back(atual);
        if (p.Declara(atual) || atual == Bloco::Global) break;
        // No dinamico, quem esta fora de f e quem a chamou.
        const bool porQuemChamou = c.chamada && c.regra == Regra::Dinamico && atual == Bloco::F;
        atual = porQuemChamou ? Bloco::G : Pai(atual);
    }
    return olhados;
}

Bloco Dono(const Programa& p, const Comando& c) {
    return Busca(p, c).back();
}

std::string Declaracao(const Programa& p, const Bloco b) {
    const std::string x = p.Declara(b) ? "int x = " + std::to_string(p.Valor(b)) + ";" : "";
    switch (b) {
        case Bloco::Global:  return x + "  // global";
        case Bloco::F:       return "void f() {";
        case Bloco::Interno: return x.empty() ? "{" : "{ " + x;
        case Bloco::G:       return "void g() { " + x;
    }
    return "";
}

std::string Placar(const Programa& p, const Comando& c, const bool achou) {
    const Bloco dono = Dono(p, c);
    std::string de;
    switch (dono) {
        case Bloco::Global:  de = "do global"; break;
        case Bloco::F:       de = "de f()"; break;
        case Bloco::Interno: de = "do bloco de dentro de f"; break;
        case Bloco::G:       de = "de g()"; break;
    }
    const std::string valor = "x = " + std::to_string(p.Valor(dono)) + ", ";

    // A chamada de f por g: a regra na linha de baixo, e depois o x que ela acha.
    if (c.chamada) {
        const bool estatico = c.regra == Regra::Estatico;
        const std::string regra = estatico ? "est\xC3\xA1tico" : "din\xC3\xA2mico";
        const std::string linha = "g() chama f(): atira(x);\n";
        if (!achou) return linha + "escopo " + regra;
        return linha + regra + ": " + valor + (estatico ? de : "de quem chamou");
    }

    std::string linha;
    switch (c.onde) {
        case Bloco::Global:  linha = "atira(x);  // no global"; break;
        case Bloco::F:       linha = "void f() { atira(x); }"; break;
        case Bloco::Interno: linha = "void f() { { atira(x); } }"; break;
        case Bloco::G:       linha = "void g() { atira(x); }"; break;
    }
    return achou ? linha + "\n" + valor + de : linha;
}

Retangulo Area(const Bloco b, const bool fAEsquerda, const float largura, const float altura) {
    return Escalado(Espelhado(Referencia(b), fAEsquerda), largura, altura);
}

Ponto OndeFicaOCanto(const Bloco b, const bool fAEsquerda, const float largura, const float altura,
                     const float larguraDoCanto) {
    if (b == Bloco::Global) {
        // No alto, do lado oposto ao placar (que fica a esquerda).
        return {largura - 16.0f - larguraDoCanto / 2.0f, 36.0f};
    }
    const Retangulo r = Area(b, fAEsquerda, largura, altura);
    return {r.x + 10.0f + larguraDoCanto / 2.0f, r.y + 6.0f + kAlturaDoCanto / 2.0f};
}

Ponto OndeFicaOProfessor(const Bloco b, const bool fAEsquerda, const float largura, const float altura) {
    Ponto p = ProfessorDeReferencia(b);
    if (!fAEsquerda) p.x = kLarguraDeReferencia - p.x;
    return {p.x * largura / kLarguraDeReferencia, p.y * altura / kAlturaDeReferencia};
}

float DesvioDaRajada(const float sorte) {
    return (std::clamp(sorte, 0.0f, 1.0f) - 0.5f) * kEntreTiros;
}

float AberturaDoLeque(const int tiros) {
    return kEntreTiros * static_cast<float>(std::max(0, tiros - 1));
}

float Duracao(const Programa& p, const Comando& c, const float porSalto, const int rajadas) {
    const auto saltos = static_cast<float>(Busca(p, c).size());
    return kAntesDaBusca + porSalto * saltos + kAntesDaResposta +
           kEntreRajadas * static_cast<float>(std::max(0, rajadas - 1));
}

Degrau DegrauDo(const Dificuldade::Nivel n) {
    Degrau d;
    if (n != Dificuldade::Nivel::Dificil) return d;
    d.rajadas = kRajadas;
    d.velocidade = 255.0f;
    d.porSalto = 0.22f;
    return d;
}

}
