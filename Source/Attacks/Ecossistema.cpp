//
// A fase 1 do Daniel: a coleta, o ecossistema dos dados.
//

#include "Ecossistema.h"

#include <algorithm>
#include <cmath>

#include "Tipos.h"

namespace Ecossistema {

    Onda OndaNoTempo(const float t) {
        if (t < kFimDaEscolha) return Onda::Nenhuma;
        if (t < kFimDoAbsorver) return Onda::Absorver;
        if (t < kFimDoPassar) return Onda::Passar;
        if (t < kFimDoDesviar) return Onda::Desviar;
        return Onda::Misturada;
    }

    int TipoDaOnda(const Onda o, const int tipoDoAluno, Variacao::Sorteio& s) {
        const int t = tipoDoAluno >= 0 && tipoDoAluno < Tipos::kTipos ? tipoDoAluno : 0;
        switch (o) {
            case Onda::Absorver: return Tipos::Vencido(t);
            case Onda::Passar: return t;
            case Onda::Desviar: return Tipos::QueVence(t);
            case Onda::Misturada: return s.Inteiro(0, Tipos::kTipos - 1);
            case Onda::Nenhuma: break;
        }
        return -1;
    }

    Vector2 CentroDaEspecie(const int tipo) {
        Vector2 c;
        switch (tipo) {
            case Tipos::kFogo: c = Vector2(0.78f, 0.28f); break;
            case Tipos::kAgua: c = Vector2(0.50f, 0.52f); break;
            default: c = Vector2(0.22f, 0.76f); break;
        }
        return c;
    }

    Vector2 LugarNoGrafico(const int tipo, Variacao::Sorteio& s) {
        const Vector2 c = CentroDaEspecie(tipo);
        // Um espalhamento parecido com o normal: a media de dois sorteios fica perto do centro.
        const auto ruido = [&s]() { return (s.Real(-1.0f, 1.0f) + s.Real(-1.0f, 1.0f)) * 0.5f * 0.16f; };
        return Vector2(std::clamp(c.x + ruido(), 0.04f, 0.96f), std::clamp(c.y + ruido(), 0.04f, 0.96f));
    }

    float XDaFonte(const int k) {
        constexpr float kX[] = {0.18f, 0.5f, 0.82f};
        return kX[std::clamp(k, 0, 2)];
    }

    Degrau DegrauDo(const Dificuldade::Nivel nivel) {
        Degrau g;
        if (nivel == Dificuldade::Nivel::Dificil) {
            g.entreAmostras = 0.26f;
            g.porVez = 4;
            g.velocidade = 230.0f;
            g.entreLeques = 1.3f;
            g.tirosDoLeque = 8;
        }
        return g;
    }
}
