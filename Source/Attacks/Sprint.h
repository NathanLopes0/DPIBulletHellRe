//
// A sprint dos metodos ageis: o cursor arrasta as historias para o quadro.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Variacao.h"

/// Na fase 2 da Maria Lucia (Metodos Ageis) cada etapa e uma sprint, com o tempo fixo de um
/// timebox.
namespace Sprint {

    /// O quadro: uma fileira de vagas abaixo de onde o professor anda e acima de onde o aluno vive.
    struct Quadro {
        float centroX = 600.0f;
        float altura = 520.0f;
        float espaco = 110.0f;   ///< entre o centro de duas vagas
        float vaga = 40.0f;      ///< a largura de uma historia, para a lei de Fitts
    };

    /// As constantes da lei de Fitts, em segundos e em segundos por bit.
    struct Fitts {
        float a = 0.35f;
        float b = 0.18f;
    };

    /// A lei de Fitts: a + b log2(distancia / largura + 1). Largura que nao e
    /// positiva conta como um pixel - o alvo mais dificil que existe.
    float TempoDeFitts(float distancia, float largura, const Fitts& f = {});

    /// O minimo solavanco: 10u^3 - 15u^4 + 6u^5, de 0 a 1, com u preso a [0, 1].
    float SemSolavanco(float u);

    /// A vaga 'i' de 'n', centradas no quadro.
    Vector2 Vaga(int i, int n, const Quadro& q = {});

    /// Onde a historia 'i' fica na pilha, em relacao a ponta do cursor.
    Vector2 NaPilha(int i);

    /// Onde fica o centro do desenho do cursor quando a ponta esta em 'ponta'.
    /// A ponta da seta esta no pixel (6, 4) do quadro de 32.
    Vector2 CentroDoCursor(Vector2 ponta, float escala);

    /// Tudo o que uma sprint vai fazer, decidido no disparo.
    struct Plano {
        Vector2 origem;                 ///< onde o cursor pega a pilha: o professor
        Vector2 soltura;                ///< onde solta: o meio do quadro
        std::vector<Vector2> vagas;
        float arraste = 0.0f;           ///< quanto o arraste leva (Fitts)
        float espalha = 0.0f;           ///< quanto as historias levam ate as vagas
        float revisao = 0.0f;           ///< quando o que sobrou cai
    };

    /// Quanto as historias levam da pilha ate as vagas, depois de soltas.
    constexpr float kEspalha = 0.35f;

    /// O clique da revisao comeca este tanto antes dela: e o aviso.
    constexpr float kAvisoDaRevisao = 0.4f;

    /// A revisao de um ataque que nao escreveu "revisao" no bloco "sprint".
    constexpr float kRevisaoPadrao = 2.5f;

    /// O posto das historias paradas no quadro (BossProjectile::DefinirPosto). Tem de ser um dos
    /// nomes de PostoExiste (RegrasDeAtaque.h).
    constexpr const char* kPostoDoQuadro = "quadro";

    /// O plano de uma sprint de 'n' historias saindo de 'origem'.
    Plano Planejar(Vector2 origem, int n, float revisao, const Quadro& q = {}, const Fitts& f = {});

    /// Onde a ponta do cursor esta 't' segundos depois do disparo.
    Vector2 PontaDoCursor(const Plano& p, float t);

    /// Onde a historia 'i' esta 't' segundos depois do disparo, ate a revisao.
    /// Depois dela quem a move e a queda (o behavior), e isto devolve a vaga.
    Vector2 PosicaoDaHistoria(const Plano& p, int i, float t);

    /// O que o cursor esta fazendo: a cara dele e quando ele some.
    enum class Gesto { Arrastando, Soltando, Esperando, Clicando, Acabou };
    Gesto GestoDoCursor(const Plano& p, float t);

    // ---- O planning poker de cada luta ----

    /// Quantas historias cada sprint tem: as cartas do baralho, uma por sprint.
    constexpr int kSprints = 5;
    using Poker = std::array<int, kSprints>;
    constexpr Poker kBaralho = {1, 2, 3, 5, 8};

    /// O poker desta luta: as quatro primeiras cartas em qualquer ordem, e o 8 sempre na ultima
    /// sprint. O total de historias e o de sempre.
    Poker Sortear(Variacao::Sorteio& s);

    /// O placar da sprint 'sprint' (1 a 5) de 'pontos' pontos.
    std::string Anuncio(int sprint, int pontos);
}

