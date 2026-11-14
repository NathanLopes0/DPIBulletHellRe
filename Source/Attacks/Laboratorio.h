//
// A repescagem do Michel: o MaVIlab - segmentacao, dados no tempo e deteccao.
//

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// A repescagem do Michel acontece no laboratorio dele, o MaVIlab, e cada etapa e uma aplicacao:
///   - na segmentacao, o encoder-decoder pinta a mascara de onde o aluno esta, da grade mais grossa
///     a mais fina (2x2, 4x4, 8x8, 16x16). A celula marcada pisca e vira pixels de tiro; o atalho
///     (a skip connection) traz tambem a celula de onde o aluno estava no passo anterior;
///   - na recorrente, a rede lembra do caminho do aluno e preve onde ele vai estar: o fantasma
///     pontilhado e a previsao, e os tiros vao para la. Na LSTM, a memoria tambem lembra da curva -
///     andar em circulo deixa de enganar. Quem quebra o padrao, escapa;
///   - na deteccao, os colegas do MaVIlab andam pelo campo, e o detector procura o aluno entre eles.
///     A confianca de cada caixa sobe com o movimento e com os tiros; quando o detector decide (o
///     NMS fica com a caixa mais confiante), a caixa escolhida e cercada - e, se era um aluno, o
///     rastreador a segue por um tempo, atirando dos cantos. Quem fica quieto como um colega na hora
///     certa, passa despercebido.
namespace Laboratorio {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 3> kEtapas = {"Segmentacao", "Recorrente", "Deteccao"};
    constexpr std::array<float, 3> kDuracoes = {7.0f, 6.0f, 7.0f};
    constexpr const char* kGatilhoDoPixel = "pixel";
    constexpr const char* kGatilhoDaPrevisao = "previsao";
    constexpr const char* kGatilhoDoColega = "colega";
    constexpr const char* kGatilhoDoCerco = "cerco";
    constexpr const char* kGatilhoDoRuido = "ruido";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A segmentacao ----

    /// Um passo do decoder: a grade n x n, quantas celulas em volta da do aluno entram (a dilatacao),
    /// o aviso antes de a mascara virar tiro, quando ele comeca (desde o comeco da etapa) e o
    /// espacamento dos pixels, em pixels de tela.
    struct PassoDoDecoder {
        int n = 2;
        int dilatacao = 0;
        float aviso = 1.0f;
        float quando = 0.0f;
        float espacamento = 40.0f;
    };
    constexpr std::array<PassoDoDecoder, 4> kDecoder = {{{2, 0, 1.15f, 0.3f, 46.0f},
                                                        {4, 0, 0.95f, 2.0f, 40.0f},
                                                        {8, 1, 0.75f, 3.6f, 34.0f},
                                                        {16, 2, 0.6f, 5.0f, 30.0f}}};

    struct Mascara {
        int n = 0;
        std::vector<bool> cel;
        [[nodiscard]] bool Em(int c, int l) const;
        [[nodiscard]] int Quantas() const;
    };

    /// A celula (coluna, linha) da grade n x n em que cai um ponto do campo normalizado (0 a 1).
    std::pair<int, int> CelulaDe(Vector2 normalizado, int n);
    /// A mascara: as celulas dos 'alunos' e dos 'atalhos' (as posicoes do passo anterior), cada uma
    /// com 'dilatacao' celulas em volta. Tudo normalizado de 0 a 1.
    Mascara Segmentar(int n, const std::vector<Vector2>& alunos, const std::vector<Vector2>& atalhos, int dilatacao);
    /// Os pixels de tiro dentro do retangulo de 'de' a 'ate', numa grade de 'espacamento', centrada.
    std::vector<Vector2> Pixels(Vector2 de, Vector2 ate, float espacamento);

    // ---- A recorrente ----

    /// O estado escondido da rede: a ultima posicao, a velocidade lembrada e o quanto ela gira.
    struct Memoria {
        Vector2 onde;
        Vector2 velocidade;
        float giro = 0.0f;   ///< radianos por segundo
        Vector2 ultima;      ///< a velocidade do ultimo passo, sem lembrar de nada: o giro sai dela
        bool comecou = false;
    };
    /// Lembra da posicao nova: a velocidade e o giro lembrados andam para os de agora, e
    /// 'esquecimento' por segundo diz o quao rapido (a porta do esquecimento).
    void Lembrar(Memoria& m, Vector2 onde, float dt, float esquecimento);
    /// Onde a rede acha que o aluno vai estar daqui a 'daqui' segundos. Sem giro, em linha reta.
    Vector2 Prever(const Memoria& m, float daqui, bool comGiro);
    /// O tempo de um tiro de 'origem' com 'rapidez' encontrar a previsao.
    float Encontro(const Memoria& m, Vector2 origem, float rapidez, bool comGiro);
    /// O maior giro que a memoria guarda (rad/s): mais que isso e ruido.
    constexpr float kGiroMaximo = 3.5f;

    // ---- A deteccao ----

    struct Caixa {
        Vector2 de;
        Vector2 ate;
    };
    Caixa CaixaEm(Vector2 centro, float lado);
    /// A intersecao sobre a uniao de duas caixas (0 a 1).
    float Iou(const Caixa& a, const Caixa& b);
    /// A supressao de nao maximos: da mais confiante para a menos, cada caixa que cobre mais de
    /// 'iouMaximo' de uma ja escolhida sai. Devolve as 'quantas' primeiras escolhidas.
    std::vector<int> Nms(const std::vector<Caixa>& caixas, const std::vector<float>& confiancas, float iouMaximo,
                         int quantas);
    /// A confianca de uma caixa anda para a de agora: um tanto pelo movimento (pixels por segundo) e
    /// um tanto pelos tiros (por segundo).
    float Confianca(float anterior, float movimento, float tirosPorSegundo, float dt);
    constexpr float kConfiancaBase = 0.12f;

    /// Os colegas do MaVIlab: quantos andam pelo campo, e o nome de cada um no placar.
    constexpr int kColegas = 5;
    std::string Colega(int k);

    // ---- O ritmo ----

    struct Degrau {
        float vidaDoPixel = 0.8f;
        float entrePrevisoes = 0.55f;
        int tirosDaPrevisao = 3;
        float aberturaDaPrevisao = 14.0f;
        float velocidadeDaPrevisao = 240.0f;
        float esquecimento = 3.0f;
        float rodadaDoDetector = 2.3f;   ///< de uma deteccao a outra
        float procura = 1.35f;           ///< o tempo da rodada em que as caixas so olham
        float avisoDoCerco = 0.55f;
        int tirosDoCerco = 18;
        float raioDoCerco = 110.0f;
        float rastreio = 1.1f;           ///< o tempo que o rastreador segue o aluno detectado
        float entreRastreios = 0.3f;
        float ladoDoRastreio = 140.0f;   ///< a caixa que segue o aluno; os tiros saem dos cantos
        float entreRuidos = 0.5f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
