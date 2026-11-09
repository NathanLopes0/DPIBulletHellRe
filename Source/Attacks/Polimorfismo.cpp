//
// A fase 1 do Luizinho: o polimorfismo.
//

#include "Polimorfismo.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace Polimorfismo {

namespace {
    float Uniforme(std::mt19937& g) { return static_cast<float>(g() % 100000u) / 100000.0f; }
    int Ate(std::mt19937& g, const int n) { return static_cast<int>(g() % static_cast<std::uint32_t>(std::max(1, n))); }

    Vector2 Polar(const float graus, const float raio) {
        const float a = Math::ToRadians(graus);
        return Vector2(Math::Cos(a) * raio, Math::Sin(a) * raio);
    }
}

const char* Nome(const Classe c) {
    switch (c) {
        case Classe::Circulo: return "Circulo";
        case Classe::Quadrado: return "Quadrado";
        case Classe::Triangulo: return "Triangulo";
        case Classe::Estrela: return "Estrela";
    }
    return "Forma";
}

std::vector<Vector2> Vertices(const Classe c) {
    std::vector<Vector2> v;
    switch (c) {
        case Classe::Circulo:
            break;
        case Classe::Quadrado:
            for (int k = 0; k < 4; ++k) v.push_back(Polar(-135.0f + 90.0f * static_cast<float>(k), 1.0f));
            break;
        case Classe::Triangulo:
            for (int k = 0; k < 3; ++k) v.push_back(Polar(-90.0f + 120.0f * static_cast<float>(k), 1.0f));
            break;
        case Classe::Estrela:
            for (int k = 0; k < 10; ++k) v.push_back(Polar(-90.0f + 36.0f * static_cast<float>(k), k % 2 == 0 ? 1.0f : 0.45f));
            break;
    }
    return v;
}

std::vector<Vector2> Contorno(const Classe c, const int pontos) {
    std::vector<Vector2> saida;
    if (pontos <= 0) return saida;
    if (c == Classe::Circulo) {
        for (int i = 0; i < pontos; ++i) saida.push_back(Polar(-90.0f + 360.0f * static_cast<float>(i) / static_cast<float>(pontos), 1.0f));
        return saida;
    }
    // Os pontos por igual pelo perimetro: cada um a perimetro/pontos do anterior.
    const auto v = Vertices(c);
    float perimetro = 0.0f;
    for (size_t k = 0; k < v.size(); ++k) perimetro += (v[(k + 1) % v.size()] - v[k]).Length();
    for (int i = 0; i < pontos; ++i) {
        float falta = perimetro * static_cast<float>(i) / static_cast<float>(pontos);
        for (size_t k = 0; k < v.size(); ++k) {
            const Vector2 a = v[k], b = v[(k + 1) % v.size()];
            const float lado = (b - a).Length();
            if (falta <= lado || k + 1 == v.size()) {
                const float u = lado > 0.0f ? std::clamp(falta / lado, 0.0f, 1.0f) : 0.0f;
                saida.push_back(a + (b - a) * u);
                break;
            }
            falta -= lado;
        }
    }
    return saida;
}

Plano Sortear(Variacao::Sorteio& s) {
    Plano p;
    p.pesoDaInstancia = s.Real(0.2f, 0.35f);
    p.pesoDoLaco = s.Real(0.3f, 0.45f);
    p.pesoDoDinamico = s.Real(0.3f, 0.45f);
    return p;
}

Chamada Proxima(const std::uint32_t sorteio, const int noLaco, const Plano& p) {
    std::mt19937 g(sorteio);
    Chamada c;
    const float total = p.pesoDaInstancia + p.pesoDoLaco + p.pesoDoDinamico;
    const float sorte = Uniforme(g) * total;
    c.comando = sorte < p.pesoDaInstancia ? Comando::Instancia
              : sorte < p.pesoDaInstancia + p.pesoDoLaco ? Comando::Laco : Comando::TipoDinamico;
    const int quantos = c.comando == Comando::Laco ? std::max(2, noLaco) : 1;
    for (int k = 0; k < quantos; ++k) {
        int classe = Ate(g, kClasses);
        if (!c.objetos.empty() && classe == static_cast<int>(c.objetos.back())) classe = (classe + 1 + Ate(g, kClasses - 1)) % kClasses;
        c.objetos.push_back(static_cast<Classe>(classe));
    }
    return c;
}

std::string Placar(const Chamada& c, const bool revelado) {
    switch (c.comando) {
        case Comando::Instancia:
            return std::string("new ") + Nome(c.objetos.front()) + "().desenhar();";
        case Comando::Laco:
            return "for (Forma f : formas) f.desenhar();";
        case Comando::TipoDinamico:
            return std::string("Forma f = sortear();\nf.desenhar();") + (revelado ? std::string(" // ") + Nome(c.objetos.front()) : "");
    }
    return "";
}

std::string Rotulo(const Chamada& c, const int k, const bool revelado) {
    const Classe classe = c.objetos[static_cast<size_t>(std::clamp(k, 0, static_cast<int>(c.objetos.size()) - 1))];
    switch (c.comando) {
        case Comando::Instancia: return std::string("new ") + Nome(classe) + "()";
        case Comando::Laco: return Nome(classe);
        case Comando::TipoDinamico: return revelado ? std::string("Forma f = ") + Nome(classe) : "Forma f = ?";
    }
    return "";
}

std::vector<Vector2> Lugares(const int n, const std::uint32_t sorteio, const float largura, const float topo, const float fundo) {
    std::mt19937 g(sorteio);
    std::vector<Vector2> lugares;
    for (int k = 0; k < n; ++k) {
        Vector2 melhor;
        float folga = -1.0f;
        // O que fica mais longe dos outros entre alguns sorteados: espalhados, e nunca dois juntos.
        for (int tentativa = 0; tentativa < 24; ++tentativa) {
            const Vector2 p(kMargem + Uniforme(g) * std::max(0.0f, largura - 2.0f * kMargem),
                            topo + Uniforme(g) * std::max(0.0f, fundo - topo));
            float perto = 1e9f;
            for (const Vector2& q : lugares) perto = std::min(perto, (p - q).Length());
            if (perto > folga) { folga = perto; melhor = p; }
            if (folga >= kDistancia) break;
        }
        lugares.push_back(melhor);
    }
    return lugares;
}

float Duracao(const Chamada& c, const float entreChamadas) {
    const float revelar = c.comando == Comando::TipoDinamico ? kRevelacao : 0.0f;
    const auto chamadas = static_cast<float>(std::max<size_t>(1, c.objetos.size()));
    return kVooDoNew + kAntesDaChamada + revelar + entreChamadas * (chamadas - 1.0f) + kTraco;
}

Degrau DegrauDo(const Dificuldade::Nivel n) {
    Degrau d;
    if (n != Dificuldade::Nivel::Dificil) return d;
    d.menorPasso = 1.7f;
    d.maiorPasso = 2.3f;
    d.noLaco = 4;
    d.pontos = 28;
    d.velocidade = 210.0f;
    d.entreChamadas = 0.5f;
    d.giro = 30.0f;
    return d;
}

}
