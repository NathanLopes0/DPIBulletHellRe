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
            g.rapidezDoProfessor = 240.0f;
            g.acender = 0.6f;
            g.bonus = 5.0f;
            g.fatorDoGanho = 1.25f;
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

    std::vector<Vector2> LugaresDasTochas(Variacao::Sorteio& s, const int n, const Campo& c, const Vector2 bau,
                                          const Vector2 professor) {
        std::vector<Vector2> v;
        const int quantas = std::max(1, n);
        for (int k = 0; k < quantas; ++k) {
            // Uma por fatia da largura, perto das paredes: o professor atravessa a sala.
            const float de = static_cast<float>(k) / static_cast<float>(quantas);
            const float ate = static_cast<float>(k + 1) / static_cast<float>(quantas);
            const float meio = (de + ate) / 2.0f;
            const float x = quantas == 1 ? s.Real(0.2f, 0.8f) : (meio < 0.5f ? s.Real(de + 0.08f, meio) : s.Real(meio, ate - 0.08f));
            Vector2 p(c.x + c.w * x, c.y + c.h * s.Real(0.18f, 0.42f));
            if ((p - bau).Length() < 160.0f) p.y = std::max(c.y + c.h * 0.15f, bau.y - 180.0f);
            v.push_back(p);
        }
        // A ordem: sempre a mais perto de onde ele esta.
        std::vector<Vector2> ordem;
        Vector2 daqui = professor;
        while (!v.empty()) {
            size_t melhor = 0;
            for (size_t i = 1; i < v.size(); ++i) if ((v[i] - daqui).Length() < (v[melhor] - daqui).Length()) melhor = i;
            daqui = v[melhor];
            ordem.push_back(v[melhor]);
            v.erase(v.begin() + static_cast<std::ptrdiff_t>(melhor));
        }
        return ordem;
    }

    int ProximaTocha(const std::vector<bool>& acesas) {
        for (size_t i = 0; i < acesas.size(); ++i) if (!acesas[i]) return static_cast<int>(i);
        return -1;
    }

    Vector2 Passo(const Vector2 de, const Vector2 para, const float rapidez, const float dt) {
        const Vector2 d = para - de;
        const float falta = d.Length();
        const float passo = std::max(0.0f, rapidez) * std::max(0.0f, dt);
        if (falta <= passo || falta < 1e-4f) return para;
        return de + d * (passo / falta);
    }

    float Percurso(const Vector2 de, const std::vector<Vector2>& tochas, const Vector2 bau, const float rapidez,
                   const float acender) {
        float distancia = 0.0f;
        Vector2 daqui = de;
        for (const Vector2& t : tochas) {
            distancia += (t - daqui).Length();
            daqui = t;
        }
        distancia += (bau - daqui).Length();
        return distancia / std::max(1.0f, rapidez) + acender * static_cast<float>(tochas.size());
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

    int MaisPerto(const Vector2 de, const std::vector<Vector2>& alvos) {
        int melhor = -1;
        for (size_t i = 0; i < alvos.size(); ++i) {
            if (melhor < 0 || (alvos[i] - de).Length() < (alvos[static_cast<size_t>(melhor)] - de).Length()) {
                melhor = static_cast<int>(i);
            }
        }
        return melhor;
    }

    Vector2 PousoDaFada(const Vector2 alvo, const float t) {
        return alvo + Vector2(10.0f * std::sin(t * 2.6f), -kAcimaDoAlvo + 4.0f * std::sin(t * 5.2f));
    }

    std::vector<Seta> Mira(const Vector2 alvo, const float raio, const float t) {
        std::vector<Seta> v;
        const float r = raio * (1.0f + 0.08f * std::sin(t * 6.0f));
        const float giro = t * 0.9f;
        for (int k = 0; k < 4; ++k) {
            const float a = giro + 1.5707963f * static_cast<float>(k);
            const Vector2 fora(std::cos(a), std::sin(a)), lado(-fora.y, fora.x);
            const Vector2 ponta = alvo + fora * r;
            const Vector2 tras = alvo + fora * (r + 12.0f);
            v.push_back({ponta, tras + lado * 7.5f, tras - lado * 7.5f});
        }
        return v;
    }
}
