//
// O padrao Observer: o aluno e o sujeito, e os olhos da professora o assinam.
//

#pragma once

#include "../Dificuldade.h"
#include "../Math.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * A fase 3 da Maria Lucia e um padrao de projeto, o OBSERVER: o aluno e o
 * SUJEITO, e a professora prende nele OBSERVADORES - post-its com um olho, cada
 * um ligado a ele por uma linha, a assinatura. Quando o estado do sujeito
 * muda, ele NOTIFICA todos os observadores de uma vez, e cada um responde com
 * um aviso mirado onde ele vai estar.
 *
 * O ESTADO E PARA ONDE O ALUNO ESTA INDO, e nao onde ele esta: o ultimo rumo
 * que nao foi zero, em cada eixo. PARAR NAO MUDA O ESTADO; VIRAR MUDA. E quem
 * vira e ja volta antes de o aviso sair (o intervalo minimo entre dois avisos)
 * nao notifica nada, porque o estado voltou a ser o ultimo notificado.
 *
 * E QUANDO NADA MUDA, A PROFESSORA PERGUNTA: e o POLLING, a alternativa que o
 * padrao existe para evitar. Sem aviso por 'consulta' segundos, ela consulta o
 * estado mesmo assim - quem fica parado tambem leva aviso, so que mais devagar.
 *
 * Quem faz isso no jogo e a MariaLucia (os gatilhos, as linhas, os olhos); aqui
 * ficam as decisoes, cobertas por test_observer.cpp.
 */
namespace Observer {

    /// Os dois gatilhos (FasesDeAtaque.h, GatilhoExiste): o ataque que
    /// responde a uma notificacao e o que responde a uma consulta.
    constexpr const char* kGatilhoDaNotificacao = "notificacao";
    constexpr const char* kGatilhoDaConsulta = "consulta";

    /// O posto dos observadores (BossProjectile::DefinirPosto), de onde os
    /// avisos saem. Tem de ser um dos nomes de PostoExiste (RegrasDeAtaque.h).
    constexpr const char* kPostoDosObservadores = "observadores";

    /// As caras do olho, que a professora troca a cada quadro: para onde ele
    /// olha, e o piscar do aviso. Tem de existir no tipo "Observador" de
    /// marialucia/projeteis.json - um teste confere.
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

    /// O estado depois de um quadro andando a 'velocidade': o eixo em que o
    /// aluno nao anda guarda o rumo de antes.
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

    /// Um quadro: NOTIFICA quando o estado de agora difere do ultimo notificado
    /// e ja passou o intervalo minimo; CONSULTA quando ninguem avisou por
    /// 'consulta' segundos. O primeiro quadro so le o estado, sem avisar.
    Aviso Avancar(Sujeito& s, Vector2 velocidadeDoAluno, float deltaTime);

    /// Para onde o olho olha: -1 para a esquerda, 0 para a frente, 1 para a
    /// direita - e o rastreamento do olhar, a tecnica de IHC.
    int Olhar(Vector2 olho, Vector2 aluno, float zonaMorta = 40.0f);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// @brief O que o Dificil muda: o sujeito notifica com 0,35 s entre dois
    /// avisos (e nao 0,5) - quem vira e volta depressa ja nao escapa do aviso
    /// -, e sem aviso ela consulta aos 1,7 s (e nao 2,0).
    struct Degrau {
        float intervaloMinimo = Sujeito{}.intervaloMinimo;
        float consulta = Sujeito{}.consulta;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// @brief O sujeito desta luta: o do degrau, com o polling sorteado de 85%
    /// a 115% do dele - de quanto em quanto tempo ela pergunta nao se decora.
    Sujeito Sortear(Variacao::Sorteio& s, const Degrau& d);
}

