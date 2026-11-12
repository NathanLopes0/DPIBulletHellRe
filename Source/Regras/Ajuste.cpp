//
// O ajuste automatico de dificuldade.
//

#include "Ajuste.h"

#include <algorithm>

namespace Ajuste {

int NivelPara(const int seguidas, const bool aprovado, const Dificuldade::Nivel nivel) {
    if (aprovado || nivel == Dificuldade::Nivel::Dificil || seguidas <= 0) return 0;
    return std::min(kMaiorDegrau, seguidas / kReprovacoesPorDegrau);
}

int DaDupla(const std::vector<int>& niveis) {
    if (niveis.empty()) return 0;
    return *std::min_element(niveis.begin(), niveis.end());
}

Degrau Do(const int nivel) {
    Degrau d;
    d.nivel = std::clamp(nivel, 0, kMaiorDegrau);
    d.velocidade = 1.0f - kVelocidadePorDegrau * static_cast<float>(d.nivel);
    d.perda = 1.0f - kPerdaPorDegrau * static_cast<float>(d.nivel);
    return d;
}

}
