//
// A fase 1 do Daniel (INF 425 - Ciencia de Dados): a coleta, o ecossistema dos dados. E a final, o
// conjunto de teste.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O campo e um grafico de dispersao - o x e o ataque, o y a defesa - e os dados chegam das tres
/// fontes (o sensor, a planilha e a API) como monstrinhos de fogo, agua e planta, voando ate o lugar
/// deles no grafico. Quem chega pousa e vira um ponto: o conjunto de dados cresce, e a fase 2 o
/// agrupa. Cada especie mora numa regiao do grafico.
///
/// Voando, a amostra tem tipo (Tipos.h): a do seu passa, a que voce vence voce absorve, a que vence
/// voce machuca. As ondas chegam na ordem que ensina o triangulo, sem uma palavra: primeiro so a que
/// voce absorve, depois so a sua, depois so a que te vence, e por fim todas misturadas.
///
/// A final e o conjunto de teste: dados que o modelo nunca viu. As amostras vem de onde nunca vieram
/// (as laterais), e as especies moram em outros lugares do grafico - a distribuicao mudou.
namespace Ecossistema {

    constexpr const char* kFase = "StateOne";
    constexpr const char* kFaseDoTeste = "StateFinal";

    /// A primeira etapa e a escolha do monstrinho (Tipos::kTempoDaEscolha); as amostras chegam
    /// depois dela.
    constexpr float kFimDaEscolha = 6.0f;

    enum class Onda { Nenhuma, Absorver, Passar, Desviar, Misturada };
    constexpr float kFimDoAbsorver = 9.0f;
    constexpr float kFimDoPassar = 12.0f;
    constexpr float kFimDoDesviar = 16.0f;
    /// A onda da fase 1 no instante 't'.
    Onda OndaNoTempo(float t);
    /// O tipo da amostra da onda, para o aluno do tipo 'tipoDoAluno' (na dupla, o do primeiro: as
    /// ondas misturadas sao iguais para os dois).
    int TipoDaOnda(Onda o, int tipoDoAluno, Variacao::Sorteio& s);

    /// O grafico, em coordenadas de 0 a 1: x e o ataque, y a defesa (1 em cima). O centro da regiao
    /// de cada especie: fogo ataca muito e defende pouco, planta o contrario, agua no meio. No
    /// conjunto de teste ('teste'), a distribuicao mudou: o ataque se inverte.
    Vector2 CentroDaEspecie(int tipo, bool teste = false);
    /// Um lugar sorteado na regiao da especie.
    Vector2 LugarNoGrafico(int tipo, Variacao::Sorteio& s, bool teste = false);

    /// As tres fontes, no alto: o sensor, a planilha e a API.
    constexpr std::array<const char*, 3> kFontes = {"Sensor", "Planilha", "Api"};
    /// O x de cada fonte, de 0 a 1 da largura do campo.
    float XDaFonte(int k);

    /// Uma parte das amostras e "medida" no aluno: em vez do lugar dela, pousa onde ele estava.
    constexpr float kChanceDeMedirOAluno = 0.5f;
    /// O quanto a amostra aguenta: o tiro normal tira 1 (Tipos::FatorDoDano).
    constexpr float kVidaDaAmostra = 2.0f;
    /// Os pontos que ficam no grafico: os mais velhos saem.
    constexpr int kMaisPontos = 60;

    /// No conjunto de teste, de onde a amostra 'k' vem: das laterais, alternando, numa altura que
    /// sobe e desce com o tempo (0 a 1 da altura do campo; o x e 0 ou 1).
    Vector2 OrigemDoTeste(int k, float t);

    struct Degrau {
        float entreAmostras = 0.32f;
        int porVez = 3;
        float velocidade = 175.0f;
        float entreLeques = 1.4f;          ///< o leque do proprio professor
        int tirosDoLeque = 8;
        float entreAmostrasDoTeste = 0.38f;
    };
    Degrau DegrauDo(Dificuldade::Nivel nivel);
}
