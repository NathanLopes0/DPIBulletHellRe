//
// O padrao Observer: o aluno e o sujeito, e os olhos da professora o assinam.
//

#include "Observer.h"

#include <algorithm>
#include <cmath>

namespace Observer {

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.intervaloMinimo = Dificuldade::Escolher(n, d.intervaloMinimo, 0.42f);
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

    Vector2 NaOrbita(const Vector2& aluno, const int k, const int n, const float t, const float raio) {
        const float volta = static_cast<float>(k) / static_cast<float>(std::max(1, n)) + kVoltasPorSegundo * t;
        const float a = 2.0f * Math::Pi * volta;
        return Vector2(aluno.x + Math::Cos(a) * raio, aluno.y - kAcimaDoAluno - std::fabs(Math::Sin(a)) * raio * kAchatamento);
    }

    std::string PlacarDoObserver(const int inscritos) {
        // \xc3\xaa e o e com circunflexo; \xc2\xb7 o ponto do meio.
        return "Observer: " + std::to_string(inscritos) + (inscritos == 1 ? " olho assina" : " olhos assinam") +
               " voc\xc3\xaa  \xc2\xb7  virou, \xc3\xa9 notifyAll()";
    }

    std::string PlacarDaNotificacao(const Estado& antes, const Estado& agora, const int inscritos) {
        std::string para = "de dire\xc3\xa7\xc3\xa3o";
        if (agora.x != antes.x) para = agora.x < 0 ? "para a esquerda" : "para a direita";
        else if (agora.y != antes.y) para = agora.y < 0 ? "para cima" : "para baixo";
        return "notifyAll(): voc\xc3\xaa virou " + para + "  \xc2\xb7  " + std::to_string(inscritos) +
               (inscritos == 1 ? " aviso" : " avisos");
    }

    std::string PlacarDaConsulta(const float segundos, const int inscritos) {
        const int decimos = static_cast<int>(std::lround(segundos * 10.0f));
        const std::string s = decimos % 10 == 0 ? std::to_string(decimos / 10)
                                                : std::to_string(decimos / 10) + "," + std::to_string(decimos % 10);
        return "polling: " + s + " s sem aviso  \xc2\xb7  " + std::to_string(inscritos) +
               (inscritos == 1 ? " olho pergunta" : " olhos perguntam") + " getEstado()";
    }

    float Raio(const float t) {
        const float u = std::clamp(t / kFimDoObserver, 0.0f, 1.0f);
        return kRaioInicial + (kRaioFinal - kRaioInicial) * u;
    }
}
