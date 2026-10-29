//
// O clock da fase 2 do Ricardo.
//

#include "Relogio.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

namespace Relogio {

    std::optional<float> LerFrequencia(const std::string& placar) {
        const std::size_t hz = placar.find("Hz");
        if (hz == std::string::npos) return std::nullopt;

        // O NUMERO LOGO ANTES: pula os espacos e volta enquanto for digito,
        // ponto ou virgula.
        std::size_t fim = hz;
        while (fim > 0 && placar[fim - 1] == ' ') --fim;
        std::size_t inicio = fim;
        while (inicio > 0) {
            const char c = placar[inicio - 1];
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == ',') --inicio;
            else break;
        }
        if (inicio == fim) return std::nullopt;

        std::string numero = placar.substr(inicio, fim - inicio);
        std::replace(numero.begin(), numero.end(), ',', '.');
        if (std::count(numero.begin(), numero.end(), '.') > 1) return std::nullopt;
        char* resto = nullptr;
        const float f = std::strtof(numero.c_str(), &resto);
        if (!resto || *resto != '\0' || !(f > 0.0f)) return std::nullopt;
        return f;
    }

    bool Subiu(const float antes, const float agora, const float frequencia) {
        if (!(frequencia > 0.0f) || agora <= antes) return false;
        return std::floor(agora * frequencia) > std::floor(std::max(0.0f, antes) * frequencia);
    }

    bool Alto(const float t, const float frequencia) {
        if (!(frequencia > 0.0f)) return true;
        const float ciclos = std::max(0.0f, t) * frequencia;
        return ciclos - std::floor(ciclos) < 0.5f;
    }

    float DuracaoDoPasso(const float frequencia) {
        if (!(frequencia > 0.0f)) return 0.08f;
        return std::min(0.08f, 0.4f / frequencia);
    }

    std::uint32_t Johnson(const int k, const int bits) {
        if (bits <= 0 || bits > 31) return 0u;
        const int volta = 2 * bits;
        const int m = ((k % volta) + volta) % volta;
        if (m <= bits) {
            // ENCHENDO: m uns, entrando pela esquerda.
            return ((1u << m) - 1u) << (bits - m);
        }
        // ESVAZIANDO: sobram 2*bits - m uns, encostados na direita.
        return (1u << (volta - m)) - 1u;
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        // QUATRO INTEIROS DE 4 A 6 QUE SOMAM 20: comeca em 5 cada, e troca um
        // segundo de uma etapa para outra uma ou duas vezes.
        const int trocas = s.Inteiro(0, 2);
        for (int k = 0; k < trocas; ++k) {
            const int de = s.Inteiro(0, 3);
            const int para = s.Inteiro(0, 3);
            if (de == para || p.duracoes[static_cast<std::size_t>(de)] <= 4 ||
                p.duracoes[static_cast<std::size_t>(para)] >= 6) continue;
            --p.duracoes[static_cast<std::size_t>(de)];
            ++p.duracoes[static_cast<std::size_t>(para)];
        }
        p.inicioDoContador = s.Inteiro(0, kMaiorInicioDoContador);
        return p;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.passo = Dificuldade::Escolher(n, kPasso, 84.0f);
        return d;
    }
}
