//
// A fase 3 do Daniel: o overfitting - o modelo que decora o aluno, e o professor que nao gosta disso.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// O professor treina um modelo com os passos do aluno. O modelo grava por onde ele anda (os pontos
/// brancos: os dados de treino) e desenha uma curva que passa EXATAMENTE por todos eles, cheia de
/// voltas - o retrato do overfitting. E atira com o que aprendeu: no lugar para onde o aluno estava
/// indo (a previsao) e nos lugares por onde ele mais passou (o que o modelo decorou). Quem repete o
/// caminho e acertado; quem muda o caminho faz o modelo errar.
///
/// Os tiros saem dos tres nucleos do modelo, em volta do professor - um de cada tipo, e o triangulo
/// vale: o tiro do nucleo do seu tipo passa, o do que voce vence voce absorve, o do que vence voce
/// machuca. Os nucleos abrem um de cada vez; o aberto apanha (o tipo que voce vence apanha o dobro),
/// o fechado so faz "tim".
///
/// Na validacao, o modelo e posto diante do que nunca viu e nao generaliza: chegam dados novos, longe
/// da curva, e ele erra todos. A curva cai aos pedacos, os tiros param, e os tres nucleos ficam
/// abertos: e a janela para quebra-los.
namespace Sobreajuste {

    constexpr const char* kFase = "StateThree";

    /// No comeco o modelo so grava; depois atira; na validacao, ele erra os dados novos.
    constexpr float kTreino = 4.0f;
    constexpr float kValidacao = 14.0f;
    /// Quanto os dados novos levam chegando, e quanto os pedacos da curva levam caindo.
    constexpr float kRiscando = 0.9f;
    constexpr float kCaindo = 1.4f;

    /// A memoria do modelo: um ponto a cada kEntreRegistros segundos, os ultimos kMaisRegistros.
    constexpr float kEntreRegistros = 0.25f;
    constexpr int kMaisRegistros = 24;
    struct Memoria {
        std::vector<Vector2> pontos;
        float proximo = 0.0f;
    };
    void Registrar(Memoria& m, Vector2 aluno, float agora);

    /// Para onde o aluno vai: o ultimo ponto mais a velocidade media dos ultimos registros, 'adiante'
    /// segundos a frente. Sem dois pontos, o ultimo (ou o zero).
    Vector2 Previsao(const Memoria& m, float adiante);

    /// O que o modelo decorou: o centro das 'quantas' celulas (de 'lado' px) com mais registros, a
    /// mais cheia primeiro.
    std::vector<Vector2> Decorados(const Memoria& m, float lado, int quantas);

    /// A curva sobreajustada: passa por todos os 'pontos', na ordem (Catmull-Rom, 'porTrecho' pedacos
    /// entre dois pontos), e ainda balanca 'exagero' px para os lados entre eles - as voltas de quem
    /// decorou o ruido.
    std::vector<Vector2> Curva(const std::vector<Vector2>& pontos, int porTrecho, float exagero);

    /// Os nucleos: tres, em volta do professor, girando devagar.
    constexpr int kNucleos = 3;
    constexpr float kRaioDaOrbita = 110.0f;
    constexpr float kGiroDaOrbita = 40.0f;    ///< graus por segundo
    Vector2 OndeDoNucleo(int k, Vector2 professor, float t);
    /// Um aberto de cada vez, trocando a cada kEntreAberturas; na validacao, todos.
    constexpr float kEntreAberturas = 2.5f;
    bool Aberto(int k, float t);

    struct Degrau {
        float entreTiros = 0.55f;           ///< os da previsao: os tres nucleos juntos, um de cada tipo
        float velocidade = 270.0f;
        float adiante = 0.85f;              ///< quanto do tempo de viagem do tiro a previsao adianta
        float entreDecorados = 1.6f;        ///< os do que ele decorou
        int tirosNoDecorado = 7;
        float avisoDoDecorado = 0.6f;       ///< a mira aparece no lugar antes dos tiros
        float vidaDoNucleo = 30.0f;
        float premioDoNucleo = 2.5f;
        float entreLeques = 1.7f;           ///< o leque do professor
        int tirosDoLeque = 9;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);
}
