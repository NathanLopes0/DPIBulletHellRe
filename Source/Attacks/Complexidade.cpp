//
// A fase 3 do Luizinho: a complexidade dos algoritmos.
//

#include "Complexidade.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace Complexidade {

namespace {
    int TetoLog2(const int n) {
        int k = 0;
        while ((1 << k) < n) ++k;
        return k;
    }
    float Uniforme(std::mt19937& g) { return static_cast<float>(g() % 100000u) / 100000.0f; }
}

std::string Notacao(const Ordem o) {
    switch (o) {
        case Ordem::Constante: return "O(1)";
        case Ordem::Logaritmica: return "O(log n)";
        case Ordem::Linear: return "O(n)";
        case Ordem::Linearitmica: return "O(n log n)";
        case Ordem::Quadratica: return "O(n\xC2\xB2)";
        case Ordem::Exponencial: return "O(2\xE2\x81\xBF)";
    }
    return "";
}

std::string Algoritmo(const Ordem o) {
    switch (o) {
        case Ordem::Constante: return "acesso v[i]";
        case Ordem::Logaritmica: return "busca bin\xC3\xA1ria";
        case Ordem::Linear: return "busca linear";
        case Ordem::Linearitmica: return "merge sort";
        case Ordem::Quadratica: return "dois la\xC3\xA7os";
        case Ordem::Exponencial: return "subconjuntos";
    }
    return "";
}

int Operacoes(const Ordem o, const int n) {
    const int m = std::max(1, n);
    switch (o) {
        case Ordem::Constante: return 1;
        case Ordem::Logaritmica: return TetoLog2(m + 1);
        case Ordem::Linear: return m;
        case Ordem::Linearitmica: return m * std::max(1, TetoLog2(m));
        case Ordem::Quadratica: return m * m;
        case Ordem::Exponencial: return 1 << std::min(m, 12);
    }
    return 0;
}

int Passadas(const Ordem o, const int n) {
    switch (o) {
        case Ordem::Linear: return 1;
        case Ordem::Linearitmica: return std::max(1, TetoLog2(std::max(1, n)));
        case Ordem::Quadratica: return std::max(1, n);
        default: return 0;
    }
}

std::string CaraDaOperacao(const Ordem o) {
    switch (o) {
        case Ordem::Constante:
        case Ordem::Logaritmica: return "Barata";
        case Ordem::Linear:
        case Ordem::Linearitmica: return "Media";
        case Ordem::Quadratica:
        case Ordem::Exponencial: return "Cara";
    }
    return "Media";
}

int MenorN(const Ordem o) {
    switch (o) {
        case Ordem::Constante: return 8;
        case Ordem::Logaritmica: return 16;
        case Ordem::Linear: return 12;
        case Ordem::Linearitmica: return 6;
        case Ordem::Quadratica: return 5;
        case Ordem::Exponencial: return 4;
    }
    return 4;
}

int MaiorN(const Ordem o) {
    switch (o) {
        case Ordem::Constante: return 64;
        case Ordem::Logaritmica: return 64;
        case Ordem::Linear: return 20;
        case Ordem::Linearitmica: return 9;
        case Ordem::Quadratica: return 6;
        case Ordem::Exponencial: return 5;
    }
    return 8;
}

Plano Sortear(Variacao::Sorteio& s) {
    Plano p;
    p.pesoDasBaratas = s.Real(0.2f, 0.35f);
    p.pesoDasMedias = s.Real(0.3f, 0.45f);
    p.pesoDasCaras = s.Real(0.25f, 0.4f);
    return p;
}

Rodada Proxima(const int vez, const std::uint32_t sorteio, const Plano& p, const int anterior, const int nAMais) {
    std::mt19937 g(sorteio);
    Rodada r;
    // O grupo pelo peso, e a ordem dentro do grupo - sem repetir a anterior.
    for (int tentativa = 0; tentativa < 8; ++tentativa) {
        const float total = p.pesoDasBaratas + p.pesoDasMedias + p.pesoDasCaras;
        const float sorte = Uniforme(g) * total;
        const int grupo = sorte < p.pesoDasBaratas ? 0 : sorte < p.pesoDasBaratas + p.pesoDasMedias ? 1 : 2;
        r.ordem = static_cast<Ordem>(grupo * 2 + static_cast<int>(g() % 2u));
        if (static_cast<int>(r.ordem) != anterior) break;
    }
    // O n sobe com a vez: o comeco usa a parte de baixo da faixa, o fim a de cima.
    const int menor = MenorN(r.ordem), maior = MaiorN(r.ordem);
    const float subida = std::clamp(static_cast<float>(vez) / 6.0f, 0.0f, 1.0f);
    const float meio = static_cast<float>(menor) + static_cast<float>(maior - menor) * subida;
    const float folga = static_cast<float>(maior - menor) * 0.35f;
    const float n = meio + (Uniforme(g) * 2.0f - 1.0f) * folga;
    r.n = std::clamp(static_cast<int>(std::lround(n)), menor, maior);
    if (r.ordem == Ordem::Logaritmica || r.ordem == Ordem::Constante) {
        // Potencias de dois: numeros que se reconhecem.
        int pot = 1;
        while ((pot << 1) <= r.n) pot <<= 1;
        r.n = std::max(menor, pot);
    }
    if (r.ordem == Ordem::Linear || r.ordem == Ordem::Linearitmica || r.ordem == Ordem::Quadratica) r.n += std::max(0, nAMais);
    return r;
}

std::string Placar(const Rodada& r) {
    const int ops = Operacoes(r.ordem, r.n);
    return Algoritmo(r.ordem) + ": " + Notacao(r.ordem) + "\nn = " + std::to_string(r.n) + " \xE2\x86\x92 " +
           std::to_string(ops) + (ops == 1 ? " opera\xC3\xA7\xC3\xA3o" : " opera\xC3\xA7\xC3\xB5" "es");
}

float AnguloDaCasa(const int i, const int n) {
    const int m = std::max(1, n);
    const float passo = kAbertura / static_cast<float>(m);
    return 90.0f - kAbertura / 2.0f + passo * (static_cast<float>(std::clamp(i, 0, m - 1)) + 0.5f);
}

int CasaDoAngulo(const float graus, const int n) {
    const int m = std::max(1, n);
    const float passo = kAbertura / static_cast<float>(m);
    const int i = static_cast<int>(std::floor((graus - (90.0f - kAbertura / 2.0f)) / passo));
    return std::clamp(i, 0, m - 1);
}

float AnguloNaCasa(const int i, const int n, const float sorte) {
    const float passo = kAbertura / static_cast<float>(std::max(1, n));
    return AnguloDaCasa(i, n) + (std::clamp(sorte, 0.0f, 1.0f) - 0.5f) * kEspalho * passo;
}

float PresoNaCasa(const float graus, const int i, const int n) {
    const float meiaCasa = kAbertura / static_cast<float>(std::max(1, n)) / 2.0f;
    const float centro = AnguloDaCasa(i, n);
    return std::clamp(graus, centro - meiaCasa, centro + meiaCasa);
}

int Meio(const Intervalo& i) {
    return i.lo + (i.hi - i.lo) / 2;
}

Intervalo Descartar(const Intervalo& i, const int alvo) {
    if (i.Vazio()) return i;
    const int meio = Meio(i);
    if (alvo == meio) return Intervalo{meio + 1, meio};
    if (alvo < meio) return Intervalo{i.lo, meio - 1};
    return Intervalo{meio + 1, i.hi};
}

float NaCurva(const Ordem o, const float n) {
    const float m = std::max(1.0f, n);
    float y = 0.0f;
    switch (o) {
        case Ordem::Constante: y = 1.0f; break;
        case Ordem::Logaritmica: y = std::log2(m + 1.0f); break;
        case Ordem::Linear: y = m; break;
        case Ordem::Linearitmica: y = m * std::max(1.0f, std::log2(m)); break;
        case Ordem::Quadratica: y = m * m; break;
        case Ordem::Exponencial: y = std::pow(2.0f, m); break;
    }
    return std::min(y, kTetoDoGrafico);
}

float Duracao(const Rodada& r) {
    const int n = std::max(1, r.n);
    switch (r.ordem) {
        case Ordem::Constante: return 0.0f;
        case Ordem::Logaritmica: return kEntreLeituras * static_cast<float>(Operacoes(r.ordem, n) - 1);
        case Ordem::Linear: return kNaPassada * static_cast<float>(n - 1);
        case Ordem::Linearitmica: {
            const int passadas = Passadas(r.ordem, n);
            return kNaPassada * static_cast<float>(n - 1) * static_cast<float>(passadas) +
                   kEntrePassadas * static_cast<float>(passadas - 1);
        }
        case Ordem::Quadratica: {
            const int passadas = Passadas(r.ordem, n);
            return kNaPassadaDosLacos * static_cast<float>(n - 1) * static_cast<float>(passadas) +
                   kEntrePassadas * 0.6f * static_cast<float>(passadas - 1);
        }
        case Ordem::Exponencial: return kEntreDobras * static_cast<float>(n - 1);
    }
    return 0.0f;
}

Degrau DegrauDo(const Dificuldade::Nivel n) {
    Degrau d;
    if (n != Dificuldade::Nivel::Dificil) return d;
    d.rapidez = 1.3f;
    d.ritmo = 0.8f;
    d.tamanho = 1.25f;
    d.depois = 0.25f;
    d.nAMais = 2;
    return d;
}

}
