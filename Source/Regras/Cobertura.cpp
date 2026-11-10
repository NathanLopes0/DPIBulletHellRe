//
// A cobertura, na dupla: derrubar o projetil que ia no parceiro.
//

#include "Cobertura.h"

#include <algorithm>

namespace Cobertura {

    bool IaPassarPerto(const Vector2& pos, const Vector2& vel, const Vector2& alvo, const float raio,
                       const float horizonte) {
        const float v2 = vel.LengthSq();
        // O instante do caminho mais perto do alvo, dentro do horizonte; parado, e agora.
        const float t = v2 > 0.0f ? std::clamp(Vector2::Dot(alvo - pos, vel) / v2, 0.0f, horizonte) : 0.0f;
        const Vector2 maisPerto = pos + vel * t;
        return (alvo - maisPerto).LengthSq() <= raio * raio;
    }

    bool VaiNele(const Vector2& pos, const Vector2& vel, const float raioDoProjetil, const Aluno& aluno,
                 const float horizonte) {
        return aluno.atingivel &&
               IaPassarPerto(pos, vel, aluno.centro, aluno.raio + raioDoProjetil + Raspao::kAlcance, horizonte);
    }

    int AlunoNoCaminho(const Vector2& pos, const Vector2& vel, const float raioDoProjetil,
                       const std::vector<Aluno>& alunos, const float horizonte) {
        if (alunos.size() < 2) return -1;
        int quem = -1;
        float maisPerto = 0.0f;
        for (std::size_t i = 0; i < alunos.size(); ++i) {
            if (!VaiNele(pos, vel, raioDoProjetil, alunos[i], horizonte)) continue;
            // Vai nos dois: no que ele pega primeiro.
            const float d = (alunos[i].centro - pos).LengthSq();
            if (quem < 0 || d < maisPerto) {
                quem = static_cast<int>(i);
                maisPerto = d;
            }
        }
        return quem;
    }
}
