//
// A fase 3 do Daniel: o aprendizado supervisionado e os comites.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 3 do Daniel comeca com o sorteio (Chamada.h) e depois classifica. O campo e dividido pela
/// fronteira de decisao entre duas classes - fogo e agua -, e o lado em que o aluno esta e a classe
/// dele: a amostra da classe dele passa por ele, e so a da outra o acerta (BossProjectile::Classe).
/// O professor joga rajadas rotuladas, uma classe por vez; quem joga escolhe o lado pela cor que vem:
///   - no perceptron, a fronteira e uma reta, e o modelo aprende com os erros a cada epoca - os dados
///     mudam (o drift), e a reta corre atras; antes de cada epoca, a reta nova aparece tracejada;
///   - no comite, tres arvores de decisao votam: o campo vira uma colcha de quadrados, e a classe de
///     cada lugar e a da maioria. De tempos em tempos uma arvore e replantada (o bagging), e a colcha
///     muda.
namespace Classificacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateThree";
    constexpr std::array<const char*, 3> kEtapas = {"Sorteio", "Perceptron", "Comite"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDaAmostra = "rotulada";
    constexpr const char* kGatilhoDaArvore = "arvore";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    /// As duas classes, e a cara da amostra de cada uma.
    constexpr int kClasses = 2;
    constexpr std::array<const char*, kClasses> kCaras = {"Fogo", "Agua"};

    // ---- O perceptron ----
    // As contas sao no campo normalizado: de -1 a 1 nos dois eixos.

    /// A reta a*x + b*y + c = 0; o lado >= 0 e a classe 1.
    struct Reta {
        float a = 1.0f;
        float b = 0.0f;
        float c = 0.0f;
    };
    int Lado(const Reta& r, Vector2 p);
    /// A reta de normal no angulo 'graus', a 'deslocamento' da origem.
    Reta RetaPeloAngulo(float graus, float deslocamento);
    /// O pedaco da reta dentro da caixa de -1 a 1. Devolve false se ela nao a corta.
    bool Segmento(const Reta& r, Vector2& de, Vector2& ate);

    struct Epoca {
        Reta modelo;
        int erros = 0;     ///< as amostras que o modelo errou nesta epoca (antes de corrigir)
    };
    /// Uma epoca do perceptron: 'amostras' sorteadas, rotuladas pela 'verdade'; cada erro puxa a reta
    /// (w += taxa * (y - y') * x). A reta sai normalizada (|(a, b)| = 1).
    Epoca Treinar(const Reta& modelo, const Reta& verdade, Variacao::Sorteio& s, int amostras, float taxa);

    // ---- O comite ----

    /// Uma arvore de decisao de profundidade 2: o primeiro corte e no x; cada lado corta no y, e as
    /// quatro folhas dizem a classe (de cima para baixo, da esquerda para a direita).
    struct Arvore {
        float corteX = 0.0f;
        std::array<float, 2> corteY{0.0f, 0.0f};
        std::array<int, 4> folhas{0, 1, 1, 0};
    };
    int Classe(const Arvore& a, Vector2 p);
    /// O voto do comite: a classe da maioria (no empate, 0).
    int Voto(const std::vector<Arvore>& comite, Vector2 p);
    /// O conceito do comite: as quatro quinas de um "xis" em volta de 'cortes' - o que uma arvore so
    /// aprende, e uma reta nao.
    int Verdade(Vector2 cortes, Vector2 p);
    /// Uma arvore plantada numa amostra (o bagging): os cortes da verdade com 'ruido', e cada folha
    /// com 'chanceDeErro' de sair trocada.
    Arvore Plantar(Variacao::Sorteio& s, Vector2 cortes, float ruido, float chanceDeErro);

    /// A classe da proxima rajada: a outra, quase sempre.
    int ClasseDaRajada(Variacao::Sorteio& s, int anterior);

    // ---- O ritmo ----

    struct Degrau {
        float entreRajadas = 0.58f;
        int tirosDaRajada = 15;
        float velocidade = 265.0f;
        float entreEpocas = 2.0f;
        float entreReplantios = 2.3f;
        float aviso = 0.6f;          ///< a reta (ou a arvore) nova, tracejada, antes de valer
        float entreChuvas = 0.07f;   ///< os dados de treino, rotulados, chovendo do alto
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    constexpr int kArvores = 3;
    constexpr float kTaxa = 0.35f;
    constexpr int kAmostrasPorEpoca = 24;
}
