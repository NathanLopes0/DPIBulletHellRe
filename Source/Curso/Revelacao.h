//
// A revelacao da materia secreta: o que a selecao de fases faz no dia em que ela aparece.
//

#pragma once

#include <string>
#include <vector>

#include "../Motor/Math.h"

/// No dia em que a materia secreta abre, a selecao de fases para tudo e a revela, em cinco tempos:
///   - o escuro: um veu cobre a tela, e a camera desliza ate a ala das optativas;
///   - a carga: quadradinhos dourados e ciano giram em espiral ate o meio da tela, cada vez mais
///     rapido, a tela treme, e um losango de contorno pisca la no meio;
///   - o estouro: um clarao branco, tres ondas em losango, uma chuva de faiscas - e o losango da
///     materia nasce grande, no meio da tela, com o reflexo girando;
///   - os parabens: o titulo desce, o nome da materia e o professor aparecem, e o botao para seguir;
///   - a ida: o losango voa ate o lugar dele na grade, o veu sai, e ele fica em foco.
/// Tudo e funcao do tempo: a cena so pergunta onde cada coisa esta. Os tempos do fim contam do
/// instante em que o aluno aperta o botao.
namespace Revelacao {

    // ---- Os tempos (segundos desde o comeco) ----
    constexpr float kEscuro = 0.5f;        ///< o veu termina de cobrir
    constexpr float kComecaACarga = 0.6f;
    constexpr float kEstouro = 2.4f;       ///< o fim da carga
    constexpr float kTitulo = 2.7f;        ///< o titulo comeca a descer
    constexpr float kNome = 3.1f;          ///< o nome e o professor aparecem
    constexpr float kPodeSeguir = 3.8f;    ///< o botao passa a valer
    constexpr float kIda = 0.8f;           ///< a ida, depois do botao

    /// A opacidade do veu (0 a 1). 'desdeOBotao' < 0: o aluno ainda nao apertou.
    float Veu(float t, float desdeOBotao);
    constexpr float kVeuCheio = 0.88f;

    /// Os quadradinhos da carga: 'quantos' em espiral ate 'centro', que nascem longe e chegam juntos
    /// no estouro. Fora da carga, nenhum.
    struct Pixel {
        Vector2 onde;
        float lado = 4.0f;
        float opacidade = 0.0f;
        bool dourado = true;   ///< dourado ou ciano, um de cada
    };
    std::vector<Pixel> Carga(Vector2 centro, float t, int quantos);
    constexpr int kPixelsDaCarga = 56;
    constexpr float kRaioDaCarga = 560.0f;

    /// O losango de contorno que pisca no meio durante a carga, cada vez mais rapido: o raio e a
    /// opacidade (0 sem ele).
    struct Contorno {
        float raio = 0.0f;
        float opacidade = 0.0f;
    };
    Contorno ContornoDaCarga(float t);

    /// Quanto a tela treme (px): cresce na carga, explode no estouro e some.
    float Tremor(float t);
    /// O deslocamento do tremor neste instante: um vaivem que nao se repete, do tamanho de Tremor.
    Vector2 Sacudida(float t);

    /// O clarao branco do estouro (0 a 1).
    float Clarao(float t);

    /// As ondas em losango do estouro: tres, uma atras da outra.
    struct Onda {
        float raio = 0.0f;
        float opacidade = 0.0f;
        int cor = 0;   ///< 0 dourada, 1 ciano, 2 branca
    };
    std::vector<Onda> Ondas(float t);
    constexpr int kOndas = 3;
    constexpr float kVidaDaOnda = 0.9f;
    constexpr float kRaioDaOnda = 720.0f;

    /// As faiscas do estouro: espirram do centro e caem.
    struct Faisca {
        Vector2 onde;
        float opacidade = 0.0f;
        float lado = 3.0f;
        int cor = 0;   ///< 0 dourada, 1 ciano, 2 branca
    };
    std::vector<Faisca> Faiscas(Vector2 centro, float t, int quantas);
    constexpr int kFaiscas = 72;
    constexpr float kVidaDaFaisca = 1.6f;
    constexpr float kGravidade = 420.0f;

    /// A escala do losango grande (em vezes a do botao): zero antes do estouro, um pulo que passa do
    /// tamanho e volta, e uma respiracao leve depois.
    float Escala(float t);
    constexpr float kEscalaGrande = 3.0f;

    /// O anel dourado que pulsa em volta do losango depois do estouro, um a cada kEntreHalos.
    Contorno Halo(float t);
    constexpr float kEntreHalos = 1.2f;

    /// Quanto o titulo ja desceu (0 a 1).
    float DescidaDoTitulo(float t);
    /// A opacidade do nome e do professor (0 a 1).
    float Nome(float t);
    /// Se o "BOTAO 1 continuar" aparece neste instante: pisca, depois de kPodeSeguir.
    bool Aviso(float t);

    /// A ida: onde o losango esta 'desdeOBotao' segundos depois do botao, de 'meio' ate 'lugar', em
    /// arco, e a escala dele (de kEscalaGrande ate 1).
    struct Ida {
        Vector2 onde;
        float escala = 1.0f;
        bool chegou = false;
    };
    Ida IdaEm(Vector2 meio, Vector2 lugar, float desdeOBotao);

    /// "a 17\xc2\xaa materia": a frase dos parabens, com o numero de materias do curso.
    std::string Parabens(int materiasDoCurso);
}
