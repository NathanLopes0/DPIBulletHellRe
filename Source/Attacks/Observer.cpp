//
// O padrao Observer: o aluno e o sujeito, e os olhos da professora o assinam.
//

#include "Observer.h"

namespace Observer {

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.intervaloMinimo = Dificuldade::Escolher(n, d.intervaloMinimo, 0.35f);
        d.consulta = Dificuldade::Escolher(n, d.consulta, 1.7f);
        return d;
    }

    Sujeito Sortear(Variacao::Sorteio& s, const Degrau& d) {
        Sujeito sujeito;
        sujeito.intervaloMinimo = d.intervaloMinimo;
        sujeito.consulta = s.Real(0.85f * d.consulta, 1.15f * d.consulta);
        return sujeito;
    }


    int Rumo(const float velocidade, const float zonaMorta) {
        if (velocidade > zonaMorta) return 1;
        if (velocidade < -zonaMorta) return -1;
        return 0;
    }

    Estado Proximo(const Estado& antes, const Vector2 velocidade) {
        Estado e = antes;
        if (const int rx = Rumo(velocidade.x); rx != 0) e.x = rx;
        if (const int ry = Rumo(velocidade.y); ry != 0) e.y = ry;
        return e;
    }

    Aviso Avancar(Sujeito& s, const Vector2 velocidadeDoAluno, const float deltaTime) {
        if (!s.comecou) {
            s.comecou = true;
            s.agora = Proximo(Estado{}, velocidadeDoAluno);
            s.notificado = s.agora;
            s.desdeOUltimo = 0.0f;
            return Aviso::Nenhum;
        }

        s.agora = Proximo(s.agora, velocidadeDoAluno);
        s.desdeOUltimo += deltaTime;

        if (s.agora != s.notificado && s.desdeOUltimo >= s.intervaloMinimo) {
            s.notificado = s.agora;
            s.desdeOUltimo = 0.0f;
            return Aviso::Notifica;
        }
        if (s.desdeOUltimo >= s.consulta) {
            s.desdeOUltimo = 0.0f;
            return Aviso::Consulta;
        }
        return Aviso::Nenhum;
    }

    int Olhar(const Vector2 olho, const Vector2 aluno, const float zonaMorta) {
        const float dx = aluno.x - olho.x;
        if (dx > zonaMorta) return 1;
        if (dx < -zonaMorta) return -1;
        return 0;
    }
}
