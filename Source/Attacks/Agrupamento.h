//
// A fase 2 do Daniel: as k-medias. A conta tambem e a do poder dele como orientador do TCC.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// Os pontos que a coleta pousou no grafico (Ecossistema.h) viram grupos. Tres centroides nascem
/// longe um do outro e, a cada iteracao, o campo mostra as duas metades das k-medias: a atribuicao
/// (a linha de cada ponto ate o centroide mais perto) e a atualizacao (o centroide desliza ate a
/// media do grupo dele). Cada grupo tem o tipo da maioria dos pontos dele, e e assim que ele ataca:
/// a cada iteracao, um anel de monstrinhos do tipo dele - e o triangulo diz quem passa, quem se
/// absorve e quem machuca.
///
/// Depois chegam os outliers: dados que nao combinam com nada, pousam longe de tudo e pesam mais na
/// media - puxam um centroide para fora do grupo, e o grupo se mistura. Derrubar o outlier conserta o
/// grupo. No fim, os grupos ficam instaveis: as iteracoes apertam.
namespace Agrupamento {

    constexpr const char* kFase = "StateTwo";
    constexpr int kGrupos = 3;
    constexpr const char* kGatilhoDoPonto = "amostra";
    constexpr const char* kGatilhoDoCentroide = "centroide";
    constexpr const char* kGatilhoDoAnel = "convergiu";
    constexpr const char* kGatilhoDoOutlier = "outlier";

    /// O indice do centroide mais perto de 'p' (-1 sem centroide).
    int MaisPerto(const std::vector<Vector2>& centroides, Vector2 p);
    /// O passo de atribuicao: o grupo de cada ponto.
    std::vector<int> Atribuir(const std::vector<Vector2>& pontos, const std::vector<Vector2>& centroides);
    /// O passo de atualizacao: cada centroide vai para a media dos pontos do grupo dele - e cada um
    /// dos 'pesados' (o outlier, ou o aluno no poder do TCC) conta 'peso' vezes no grupo da celula em
    /// que esta. O grupo vazio fica onde estava.
    std::vector<Vector2> Atualizar(const std::vector<Vector2>& pontos, const std::vector<int>& grupos,
                                   const std::vector<Vector2>& centroides, const std::vector<Vector2>& pesados,
                                   float peso);

    /// O tipo do grupo 'g': o da maioria dos pontos dele (no empate, o menor). -1 no grupo vazio.
    int TipoDoGrupo(const std::vector<int>& tipos, const std::vector<int>& grupos, int g);

    /// Os k centroides iniciais: um ponto sorteado, e depois sempre o ponto mais longe dos ja
    /// escolhidos - eles nascem espalhados.
    std::vector<Vector2> Sementes(const std::vector<Vector2>& pontos, int k, Variacao::Sorteio& s);

    /// Onde o outlier pousa: entre uns lugares sorteados do grafico de 'de' a 'ate', o mais longe de
    /// todos os centroides.
    Vector2 LugarDoOutlier(const std::vector<Vector2>& centroides, Vector2 de, Vector2 ate, Variacao::Sorteio& s);

    /// A partir de quando chegam os outliers, e quando os grupos ficam instaveis.
    constexpr float kOutliersDesde = 12.0f;
    constexpr float kInstavelDesde = 17.0f;
    /// O quanto o outlier pesa na media, e o quanto aguenta (o tiro normal tira 1).
    constexpr float kPesoDoOutlier = 5.0f;
    constexpr float kVidaDoOutlier = 2.0f;
    /// O que cada outlier derrubado vale.
    constexpr float kPremioDoOutlier = 0.4f;

    struct Degrau {
        float entreIteracoes = 1.8f;
        float deslize = 0.6f;              ///< quanto o centroide leva deslizando
        int tirosDoAnel = 20;
        float velocidadeDoAnel = 110.0f;
        float entreOutliers = 1.4f;
        int maisOutliers = 4;
        float fatorInstavel = 0.55f;       ///< o intervalo das iteracoes, no fim
        float entreLeques = 1.6f;
        int tirosDoLeque = 9;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
