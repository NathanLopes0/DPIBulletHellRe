//
// A fase 3 do Lucas: o Elixir, numa sala de masmorra.
//

#include "Masmorra.h"

#include <algorithm>
#include <cmath>

namespace Masmorra {

    Degrau DegrauDo(const Dificuldade::Nivel nivel) {
        Degrau g;
        if (nivel == Dificuldade::Nivel::Dificil) {
            g.gosmas = 4;
            g.rapidezDasGosmas = 95.0f;
            g.chegada = 5.4f;
            g.bonus = 5.0f;
            g.fatorDoGanho = 2.5f;
            g.furia = 4.2f;
            g.bracos = 5;
            g.entreRajadas = 0.07f;
            g.velocidadeDaFuria = 230.0f;
            g.giro = 95.0f;
            g.entreAneis = 0.6f;
            g.tirosDoAnel = 22;
            g.entreLeques = 1.2f;
            g.tirosDoLeque = 5;
        }
        return g;
    }

    Vector2 LugarDoBau(Variacao::Sorteio& s, const Campo& c, const Vector2 aluno) {
        Vector2 melhor;
        float maisLonge = -1.0f;
        // Algumas tentativas: fica a primeira longe o bastante, ou a mais longe delas.
        for (int k = 0; k < 12; ++k) {
            const Vector2 p(c.x + c.w * s.Real(0.12f, 0.88f), c.y + c.h * s.Real(0.55f, 0.85f));
            const float d = (p - aluno).Length();
            if (d >= kLongeDoAluno) return p;
            if (d > maisLonge) { maisLonge = d; melhor = p; }
        }
        return melhor;
    }

    std::vector<Vector2> LugaresDasGosmas(Variacao::Sorteio& s, const int n, const Campo& c, const Vector2 bau) {
        std::vector<Vector2> v;
        for (int k = 0; k < n; ++k) {
            // Uma por fatia da largura, para elas nao nascerem juntas.
            const float de = static_cast<float>(k) / static_cast<float>(n), ate = static_cast<float>(k + 1) / static_cast<float>(n);
            Vector2 p(c.x + c.w * (0.08f + 0.84f * s.Real(de, ate)), c.y + c.h * s.Real(0.3f, 0.55f));
            if ((p - bau).Length() < 120.0f) p.y = std::max(c.y + c.h * 0.25f, bau.y - 140.0f);
            v.push_back(p);
        }
        return v;
    }

    Vector2 CaminhoDoProfessor(const Vector2 de, const Vector2 bau, const float tc, const float chegada,
                               const Campo& c) {
        // Dois trechos: de lado, rente ao alto, e depois a descida. O tempo de cada um e o tamanho dele.
        const float alto = std::max(c.y + 70.0f, std::min(de.y, c.y + 90.0f));
        const Vector2 canto(bau.x, alto);
        const Vector2 comeco(de.x, alto);
        const float a = std::fabs(canto.x - comeco.x) + std::fabs(alto - de.y);
        const float b = std::fabs(bau.y - alto);
        const float total = std::max(1.0f, a + b);
        const float u = std::clamp(tc / std::max(0.1f, chegada), 0.0f, 1.0f) * total;
        if (u <= std::fabs(alto - de.y)) {
            const float f = u / std::max(1.0f, std::fabs(alto - de.y));
            return de + (comeco - de) * f;
        }
        if (u <= a) {
            const float f = (u - std::fabs(alto - de.y)) / std::max(1.0f, std::fabs(canto.x - comeco.x));
            return comeco + (canto - comeco) * f;
        }
        const float f = (u - a) / std::max(1.0f, b);
        return canto + (bau - canto) * f;
    }

    bool Alcanca(const Vector2 aluno, const Vector2 coisa) {
        return (aluno - coisa).Length() <= kRaioDePegar;
    }

    std::vector<float> RumosDaEspiral(const int bracos, const float tf, const float giroPorSegundo) {
        std::vector<float> v;
        const int n = std::max(1, bracos);
        for (int k = 0; k < n; ++k) {
            v.push_back(std::fmod(tf * giroPorSegundo + 360.0f * static_cast<float>(k) / static_cast<float>(n), 360.0f));
        }
        return v;
    }
}
