//
// A fase 3 do Michel: as redes neurais e as residuais.
//

#pragma once

#include <array>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 3 do Michel e a rede por dentro. Entre o professor e o aluno ficam as camadas de neuronios,
/// e os sinais (tiros) andam pelas ligacoes de uma camada para a seguinte. Cada neuronio soma o que
/// chega e, passando do limiar, dispara pelas ligacoes fortes; a ultima camada dispara no aluno:
///   - nos neuronios, a rede so propaga. Um neuronio derrubado sai da rede por um tempo (o dropout):
///     o que chegaria nele se perde. Quem joga escolhe que caminho cortar;
///   - na retropropagacao, o erro volta: a rede aprende qual saida esta em cima do aluno, e as
///     ligacoes ate ela engrossam. Ficar parado e ensinar a rede;
///   - na residual, a rede fica funda, e os atalhos pulam duas camadas de uma vez, levando o sinal
///     inteiro. O dropout no meio nao corta o atalho: e preciso derrubar onde ele chega.
namespace RedeNeural {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateThree";
    constexpr std::array<const char*, 3> kEtapas = {"Neuronios", "Retropropagacao", "Residual"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDoNeuronio = "neuronio";
    constexpr const char* kGatilhoDoSinal = "sinal";
    constexpr const char* kGatilhoDaSaida = "saida";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    /// As camadas de cada etapa: quantos neuronios em cada uma, da entrada (no alto) a saida.
    std::vector<int> CamadasDa(int etapa);

    /// Um atalho: do neuronio 'de' da camada 'camada' ao 'ate' da camada 'camada + pulo'.
    struct Atalho {
        int camada = 0;
        int de = 0;
        int ate = 0;
        int pulo = 2;
    };

    struct Rede {
        std::vector<int> camadas;
        /// pesos[k][i * camadas[k + 1] + j]: do neuronio i da camada k ao j da seguinte.
        std::vector<std::vector<float>> pesos;
        std::vector<Atalho> atalhos;
        [[nodiscard]] float Peso(int k, int i, int j) const;
        float& Peso(int k, int i, int j);
    };

    /// Uma rede com as 'camadas' e os pesos sorteados entre 'menor' e 'maior'.
    Rede Montar(const std::vector<int>& camadas, Variacao::Sorteio& s, float menor, float maior);

    /// As ligacoes fortes do neuronio 'i' da camada 'k': os da camada seguinte com peso a partir
    /// de 'limiar'. A ultima camada nao liga em ninguem.
    std::vector<int> Ligacoes(const Rede& r, int k, int i, float limiar);

    /// O neuronio da camada de 'n2' alinhado ao 'i' da de 'n1': a mesma fracao da largura.
    int Alinhado(int i, int n1, int n2);
    /// Os atalhos (a identidade) de 'pulo' camadas: de cada neuronio das camadas 0, pulo, 2*pulo...
    /// ao alinhado com ele.
    std::vector<Atalho> Atalhos(const std::vector<int>& camadas, int pulo);

    /// O neuronio soma a 'entrada'; se passou do 'limiar', dispara e desconta o limiar.
    bool Somar(float& soma, float entrada, float limiar);

    /// A retropropagacao, simplificada: a saida 'alvo' devia disparar e as outras nao. O erro volta
    /// camada por camada pelos proprios pesos, e cada peso anda 'taxa' na direcao dele, entre
    /// 'menor' e 'maior'.
    void Retropropagar(Rede& r, int alvo, float taxa, float menor, float maior);

    /// A saida mais perto do aluno, pelo x de cada uma.
    int SaidaDoAluno(const std::vector<float>& xDasSaidas, float xDoAluno);

    // ---- O ritmo ----

    struct Degrau {
        float entreEntradas = 0.42f;     ///< o professor manda um sinal a um neuronio da entrada
        float velocidadeDoSinal = 150.0f;
        float limiar = 1.0f;             ///< o das camadas do meio
        float limiarDaSaida = 1.5f;
        float ligacaoForte = 0.55f;      ///< o peso a partir do qual a ligacao leva sinal
        float pesoMenor = 0.15f;
        float pesoMaior = 1.0f;
        int resistencia = 3;             ///< os tiros que derrubam um neuronio
        float dropout = 3.2f;            ///< o tempo que ele fica fora
        float entreTreinos = 0.9f;
        float taxa = 0.18f;
        int tirosDaSaida = 3;
        float aberturaDaSaida = 24.0f;
        float velocidadeDaSaida = 200.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// Onde as camadas ficam no campo, em fracao da altura: a primeira e a ultima.
    constexpr float kAlturaDaEntrada = 0.2f;
    constexpr float kAlturaDaSaida = 0.6f;
}
