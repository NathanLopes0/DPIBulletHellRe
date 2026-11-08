//
// O padrao Observer: o aluno e o sujeito, e os olhos da professora o assinam.
//

#pragma once

#include "../Regras/Dificuldade.h"
#include "../Motor/Math.h"
#include "../Regras/Variacao.h"

/// A fase 3 da Maria Lucia e um padrao de projeto, o observer: o aluno e o sujeito, e a professora
/// prende nele observadores.
namespace Observer {

    /// Os dois gatilhos (FasesDeAtaque.h, GatilhoExiste): o ataque que
    /// responde a uma notificacao e o que responde a uma consulta.
    constexpr const char* kGatilhoDaNotificacao = "notificacao";
    constexpr const char* kGatilhoDaConsulta = "consulta";

    /// O posto dos observadores (BossProjectile::DefinirPosto), de onde os
    /// avisos saem. Tem de ser um dos nomes de PostoExiste (RegrasDeAtaque.h).
    constexpr const char* kPostoDosObservadores = "observadores";

    /// As caras do olho, que a professora troca a cada quadro: para onde ele olha, e o piscar do
    /// aviso.
    constexpr const char* kOlhaEsquerda = "OlhaEsquerda";
    constexpr const char* kOlhaFrente = "OlhaFrente";
    constexpr const char* kOlhaDireita = "OlhaDireita";
    constexpr const char* kPisca = "Pisca";

    /// O rumo de uma velocidade num eixo: -1, 0 ou 1, com uma zona morta para
    /// o tremor de quem esta quase parado nao contar como andar.
    int Rumo(float velocidade, float zonaMorta = 20.0f);

    /// O estado do sujeito: o ultimo rumo que nao foi zero, em cada eixo.
    struct Estado {
        int x = 0;
        int y = 0;
        bool operator==(const Estado& o) const { return x == o.x && y == o.y; }
        bool operator!=(const Estado& o) const { return !(*this == o); }
    };

    Estado Proximo(const Estado& antes, Vector2 velocidade);

    enum class Aviso { Nenhum, Notifica, Consulta };

    /// O relogio do sujeito.
    struct Sujeito {
        float intervaloMinimo = 0.5f;   ///< entre dois avisos quaisquer
        float consulta = 2.0f;          ///< sem aviso por isto, a consulta

        Estado agora;                   ///< o estado deste quadro
        Estado notificado;              ///< o ultimo que os observadores receberam
        float desdeOUltimo = 0.0f;
        bool comecou = false;
    };

    /// O primeiro quadro so le o estado, sem avisar.
    Aviso Avancar(Sujeito& s, Vector2 velocidadeDoAluno, float deltaTime);

    /// Para onde o olho olha: -1 para a esquerda, 0 para a frente, 1 para a direita - e o
    /// rastreamento do olhar, a tecnica de ihc.
    int Olhar(Vector2 olho, Vector2 aluno, float zonaMorta = 40.0f);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// O que o Dificil muda: o sujeito notifica com 0,35 s entre dois avisos (e nao 0,5).
    struct Degrau {
        float intervaloMinimo = Sujeito{}.intervaloMinimo;
        float consulta = Sujeito{}.consulta;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O sujeito desta luta: o do degrau, com o polling sorteado de 85% a 115% do dele - de quanto
    /// em quanto tempo ela pergunta nao se decora.
    Sujeito Sortear(Variacao::Sorteio& s, const Degrau& d);
}

