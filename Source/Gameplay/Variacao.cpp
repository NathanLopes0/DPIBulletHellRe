//
// O sorteio de cada batalha: o que muda de uma luta para a outra.
//

#include "Variacao.h"

#include <algorithm>
#include <cmath>

namespace Variacao {

std::uint32_t Misturar(const std::uint32_t semente, const std::uint32_t rotulo) {
    std::uint64_t z = (static_cast<std::uint64_t>(semente) << 32) | rotulo;
    z += 0x9e3779b97f4a7c15ull;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    z ^= z >> 31;
    return static_cast<std::uint32_t>(z ^ (z >> 32));
}

std::uint32_t Rotulo(const std::string& nome) {
    std::uint32_t h = 2166136261u;
    for (const char c : nome) {
        h ^= static_cast<std::uint8_t>(c);
        h *= 16777619u;
    }
    return h;
}

int Sorteio::Inteiro(const int de, const int ate) {
    if (ate <= de) return de;
    const std::uint64_t n = static_cast<std::uint64_t>(static_cast<std::int64_t>(ate) - de) + 1u;
    if (n > 0xffffffffull) return static_cast<int>(static_cast<std::int64_t>(de) + Bruto());   // o int inteiro
    // O MAIOR MULTIPLO de n que cabe em 2^32: o que passa dele e sorteado de
    // novo, senao os primeiros restos sairiam uma vez a mais.
    const std::uint64_t limite = (0x100000000ull / n) * n;
    std::uint64_t x;
    do { x = Bruto(); } while (x >= limite);
    return static_cast<int>(static_cast<std::int64_t>(de) + static_cast<std::int64_t>(x % n));
}

float Sorteio::Real(const float de, const float ate) {
    if (!(ate > de)) return de;
    const float u = static_cast<float>(Bruto() >> 8) * (1.0f / 16777216.0f);   // [0, 1)
    const float r = de + u * (ate - de);
    return r < ate ? r : std::nextafter(ate, de);
}

std::vector<int> Ordem(Sorteio& s, const int n, const int fixas) {
    std::vector<int> ordem;
    for (int i = 0; i < n; ++i) ordem.push_back(i);
    if (fixas < n) {
        std::vector<int> resto(ordem.begin() + std::max(0, fixas), ordem.end());
        s.Embaralhar(resto);
        std::copy(resto.begin(), resto.end(), ordem.begin() + std::max(0, fixas));
    }
    return ordem;
}

int NaOrdem(const std::vector<int>& ordem, const int vez) {
    const int n = static_cast<int>(ordem.size());
    if (n == 0 || vez < 0) return vez;
    return ordem[static_cast<size_t>(vez % n)] + (vez / n) * n;
}

bool Sorteio::Chance(const float p) {
    if (p <= 0.0f) return false;
    if (p >= 1.0f) return true;
    return Real(0.0f, 1.0f) < p;
}

}
