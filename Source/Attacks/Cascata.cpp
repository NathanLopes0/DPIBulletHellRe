//
// O modelo cascata da fase 1 da Maria Lucia.
//

#include "Cascata.h"

#include <algorithm>
#include <random>

namespace Cascata {

    int EtapaNoTempo(const float t) {
        if (t < 0.0f) return 0;
        const int k = static_cast<int>(t / kDuracaoDaEtapa);
        return std::min(k, static_cast<int>(kEtapas.size()));
    }

    const char* CaraDaEtapa(const int etapa) {
        static constexpr std::array<const char*, 5> kCaras = {"Pedido", "Classe", "Codigo", "Bug", "Debito"};
        return kCaras[static_cast<size_t>(std::clamp(etapa, 0, 4))];
    }

    float EntreQuedas(const int etapa, const Dificuldade::Nivel n) {
        static constexpr std::array<float, 5> kNormal = {0.40f, 0.34f, 0.28f, 0.24f, 0.21f};
        const float normal = kNormal[static_cast<size_t>(std::clamp(etapa, 0, 4))];
        return Dificuldade::Escolher(n, normal, normal * 0.85f);
    }

    float VelocidadeDaQueda(const int etapa) {
        return 140.0f + 25.0f * static_cast<float>(std::clamp(etapa, 0, 4));
    }

    int Custo(const int etapa) {
        static constexpr std::array<int, 5> kCustos = {1, 2, 3, 5, 8};
        return kCustos[static_cast<size_t>(std::clamp(etapa, 0, 4))];
    }

    int TirosDoCusto(const int etapa) {
        return std::min(Custo(etapa), kMaisNoAnel);
    }

    std::string PlacarDaMudanca(const int etapa) {
        // Os nomes com acento: \xc3\xa7 e o c cedilha, \xc3\xa3 o a til, \xc3\xa7\xc3\xa3 "ca".
        static constexpr std::array<const char*, 5> kNomes = {
            "nos Requisitos", "no Projeto", "na Implementa\xc3\xa7\xc3\xa3o", "nos Testes", "na Manuten\xc3\xa7\xc3\xa3o"};
        const int k = std::clamp(etapa, 0, 4);
        // \xc3\xa7 e o c cedilha; \xc3\x97 o sinal de vezes.
        return std::string("Mudan\xc3\xa7" "a ") + kNomes[static_cast<size_t>(k)] + ": custo \xc3\x97" +
               std::to_string(Custo(k));
    }

    Vector2 OrigemDoPedido(const std::uint32_t sorteio, const float largura, const float altura) {
        std::mt19937 g(sorteio);
        const bool esquerda = (g() % 2u) == 0u;
        const float de = altura * 0.45f;
        const float ate = altura - 60.0f;
        const float y = de + static_cast<float>(g() % 1000u) / 1000.0f * (ate - de);
        return Vector2(esquerda ? -20.0f : largura + 20.0f, y);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreMudancas = 1.9f;
        d.quedaMaisRapida = 1.15f;
        return d;
    }
}
