//
// A final do Daniel: a matriz de confusao - onde o modelo acerta e onde ele erra.
//

#pragma once

#include <array>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O exame final: o modelo, ja treinado, classifica dados novos, e o campo e a matriz de confusao
/// dele - uma grade 3 x 3, as linhas o tipo de verdade (fogo, agua, planta, de cima para baixo) e as
/// colunas o tipo que o modelo previu (da esquerda para a direita). Cada monstrinho voa do professor
/// para a casa em que o modelo o pos. Na diagonal (acertou) ele fica quieto e so conta; fora dela
/// (errou), a casa acende e ele atira no aluno - no tipo de verdade dele, e o triangulo vale. O tiro
/// do aluno o corrige: ele desliza para a diagonal, e a correcao vale nota. Se ninguem o corrige a
/// tempo, o erro vaza para a producao: estoura num anel. Em cima, a acuracia - os acertos da
/// diagonal sobre o total - sobe a cada correcao.
namespace Confusao {

    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 2> kEtapas = {"Classificar", "Acuracia"};
    constexpr std::array<float, 2> kDuracoes = {11.0f, 9.0f};
    /// Na segunda etapa o modelo classifica mais depressa.
    constexpr float kFimDaClassificacao = 11.0f;

    constexpr const char* kGatilhoDaAmostra = "classificar";
    constexpr const char* kGatilhoDoErro = "erro";
    constexpr const char* kGatilhoDoVazamento = "vazou";

    constexpr int kTipos = 3;
    using Matriz = std::array<std::array<int, kTipos>, kTipos>;

    /// A grade no campo, em pixels.
    struct Grade {
        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    };
    /// O centro da casa da linha 'real' e da coluna 'previsto'.
    Vector2 CentroDaCelula(const Grade& g, int real, int previsto);
    /// O lugar do 'k'-esimo monstrinho de uma casa: ate kPorCelula lado a lado, sem sair dela.
    constexpr int kPorCelula = 4;
    Vector2 LugarNaCelula(const Grade& g, int real, int previsto, int k);

    /// O que o modelo preve para um dado do tipo 'real': acerta com a chance 'acuracia'; errando,
    /// e um dos outros dois.
    int Previsto(Variacao::Sorteio& s, int real, float acuracia);

    /// Os acertos (a diagonal) sobre o total; zero com a matriz vazia.
    float Acuracia(const Matriz& m);
    /// A acuracia em por cento, inteira: "62%".
    int Porcento(const Matriz& m);

    struct Degrau {
        float entreAmostras = 0.85f;
        float naSegundaEtapa = 0.62f;      ///< o intervalo na etapa da acuracia
        float acuracia = 0.5f;             ///< a chance de o modelo acertar cada um
        float velocidade = 300.0f;         ///< o voo do professor ate a casa
        int errosNoCampo = 5;              ///< mais que isto, o modelo acerta (a tela nao enche)
        float vidaDoErro = 2.0f;           ///< o tiro normal tira 1 (Tipos::FatorDoDano)
        float paciencia = 5.0f;            ///< o tempo ate o erro vazar
        float entreTirosDoErro = 1.6f;
        float velocidadeDoErro = 200.0f;
        int tirosDoVazamento = 10;
        float velocidadeDoVazamento = 130.0f;
        float premioDaCorrecao = 1.2f;     ///< bruto (Battle::Conquistar)
        float entreLeques = 1.9f;
        int tirosDoLeque = 7;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);
}
