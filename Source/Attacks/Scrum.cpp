//
// O Scrum da fase 2 da Maria Lucia.
//

#include "Scrum.h"

#include <algorithm>

namespace Scrum {

    int SprintNoTempo(const float t) {
        if (t < kBacklog) return -1;
        return std::min(kSprints, static_cast<int>((t - kBacklog) / kSprint));
    }

    float InicioDaSprint(const int sprint) {
        return kBacklog + kSprint * static_cast<float>(sprint);
    }

    int Dailies(const float dentro) {
        int n = 0;
        for (const float d : kDailies) if (dentro >= d) ++n;
        return n;
    }

    int Historias(const int sprint, const Dificuldade::Nivel n) {
        const int base = 2 + std::clamp(sprint, 0, kSprints - 1);
        return base + Dificuldade::Escolher(n, 0, 1);
    }

    std::vector<int> Pontos(Variacao::Sorteio& s, const int n) {
        std::vector<int> p;
        for (int i = 0; i < n; ++i) p.push_back(kBaralho[static_cast<size_t>(s.Inteiro(0, 4))]);
        return p;
    }

    Vector2 Vaga(const int coluna, const int posicao, const float esquerda, const float largura, const float topo) {
        const float c = static_cast<float>(std::clamp(coluna, 0, kColunas - 1));
        return Vector2(esquerda + largura * (c + 0.5f) / static_cast<float>(kColunas),
                       topo + kTopoDasVagas + kEntreHistorias * static_cast<float>(std::max(0, posicao)));
    }

    int TirosDaHistoria(const int pontos) {
        return 1 + std::max(0, pontos) / 3;
    }

    std::string Placar(const int sprint, const int dailies, const int faltam) {
        // \xc2\xb7 e o ponto do meio.
        std::string s = "Sprint " + std::to_string(sprint + 1);
        if (dailies > 0) s += "  \xc2\xb7  daily " + std::to_string(dailies);
        return s + "  \xc2\xb7  faltam " + std::to_string(faltam) + (faltam == 1 ? " ponto" : " pontos");
    }

    std::string Cara(const int pontos) {
        return "P" + std::to_string(pontos);
    }
}
