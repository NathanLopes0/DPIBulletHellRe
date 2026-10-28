//
// O poder do orientador no TCC: o "especial" da materia que ele da.
//

#include "Poder.h"

#include <algorithm>
#include <cmath>

namespace Poder {

    Descricao Da(const Golpe::Materia m) {
        Descricao d;
        switch (m) {
            case Golpe::Materia::Andre:
                d = {Tipo::Rajada, "for (;;)", "um laço sem fim no Hugo", 2.4f, 0.0f, 40, 0.06f};
                break;
            case Golpe::Materia::Salles:
                d = {Tipo::Capivara, "a capivara", "come os tiros perto de você", 5.0f, 95.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Ricardo:
                d = {Tipo::Congelar, "HLT", "o clock parou: tudo congela", 2.8f, 0.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Thiago:
                d = {Tipo::Rollback, "ROLLBACK", "os tiros do Hugo voltam", 0.0f, 0.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Luizinho:
                d = {Tipo::Liberar, "free()", "o coletor de lixo passou", 0.0f, 280.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Vladimir:
                d = {Tipo::Converter, "o autômato rejeita", "os tiros em volta viram seus", 0.0f, 240.0f, 0, 0.0f};
                break;
            case Golpe::Materia::Julio:
                d = {Tipo::Agentes, "agentes autônomos", "três robôs caçam o Hugo", 5.0f, 70.0f, 3, 0.22f};
                break;
            case Golpe::Materia::MariaLucia:
                d = {Tipo::BalaDePrata, "a bala de prata", "o escudo do Hugo quebrou", 0.0f, 0.0f, 1, 0.0f};
                break;
            case Golpe::Materia::Lucas:
                d = {Tipo::PromiseAll, "Promise.all()", "doze promessas, juntas", 0.7f, 60.0f, 12, 0.0f};
                break;
            case Golpe::Materia::Vitor:
                d = {Tipo::Fork, "fork()", "o processo filho atira junto", 6.0f, 0.0f, 0, 0.0f};
                break;
            default:
                break;
        }
        return d;
    }

    float Fracao(const int carga) {
        return std::clamp(static_cast<float>(carga) / static_cast<float>(kCarga), 0.0f, 1.0f);
    }

    bool Ataca(const Tipo t) {
        return t == Tipo::Rajada || t == Tipo::Converter || t == Tipo::Agentes || t == Tipo::PromiseAll ||
               t == Tipo::Fork || t == Tipo::BalaDePrata;
    }

    Vector2 Agente(const Vector2& centro, const int k, const int n, const float t, const float raio) {
        const float a = 2.0f * Math::Pi * (static_cast<float>(k) / static_cast<float>(std::max(1, n)) + 0.5f * t);
        return centro + Vector2(Math::Cos(a), Math::Sin(a)) * raio;
    }

    float Espelho(const float x, const float esquerda, const float direita) {
        return std::clamp(esquerda + direita - x, esquerda, direita);
    }

    int QuadroDaMateria(const Golpe::Materia m) {
        // Os quadros 3 a 12 da folha, na ordem do enum (Generico e o 0).
        if (m == Golpe::Materia::Generico) return -1;
        return 2 + static_cast<int>(m);
    }

    bool NoAlcance(const Vector2& centro, const Vector2& tiro, const float raio) {
        return (tiro - centro).LengthSq() <= raio * raio;
    }
}
