//
// A fase 2 do Daniel: os atributos e o aprendizado nao supervisionado.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// A fase 2 do Daniel comeca com o sorteio (Chamada.h) e depois tira estrutura de dados sem rotulo:
///   - na PCA, uma nuvem alongada aparece, e a componente principal - a direcao de maior variancia -
///     corta o campo. Cada ponto desce reto ate ela (a projecao: de duas dimensoes para uma), e a
///     fileira que sobra GIRA em volta da media: uma barra de tiros que varre o campo;
///   - nas k-medias, a nuvem vira grupos, e os k centroides vao, a cada iteracao, para a media dos
///     pontos da celula deles (o Voronoi, desenhado no campo). O aluno tambem e um ponto - pesado -:
///     o centroide da celula em que ele esta vem para cima dele. Trocar de celula troca quem
///     persegue. No fim, o cotovelo: o k sobe.
namespace Agrupamento {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateTwo";
    constexpr std::array<const char*, 3> kEtapas = {"Sorteio", "PCA", "KMedias"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 6.0f, 8.0f};
    constexpr const char* kGatilhoDoPonto = "amostra";
    constexpr const char* kGatilhoDoCentroide = "centroide";
    constexpr const char* kGatilhoDoAnel = "convergiu";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A PCA ----

    struct Eixo {
        Vector2 centro;
        Vector2 direcao;   ///< de comprimento 1
    };
    /// A componente principal: a reta pela media, na direcao de maior variancia. Sem pontos (ou com
    /// todos no mesmo lugar), a horizontal.
    Eixo ComponentePrincipal(const std::vector<Vector2>& ps);
    /// O ponto da reta mais perto de 'p': a projecao.
    Vector2 Projetar(const Eixo& e, Vector2 p);
    /// 'p' girado de 'graus' em volta de 'centro'.
    Vector2 Girar(Vector2 p, Vector2 centro, float graus);

    // ---- As k-medias ----

    /// O indice do centroide mais perto de 'p' (-1 sem centroide).
    int MaisPerto(const std::vector<Vector2>& centroides, Vector2 p);
    /// O passo de atribuicao: o grupo de cada ponto.
    std::vector<int> Atribuir(const std::vector<Vector2>& pontos, const std::vector<Vector2>& centroides);
    /// O passo de atualizacao: cada centroide vai para a media dos pontos do grupo dele - e cada
    /// aluno, que tambem e um ponto, conta 'pesoDoAluno' vezes no grupo da celula em que esta. O grupo
    /// vazio fica onde estava.
    std::vector<Vector2> Atualizar(const std::vector<Vector2>& pontos, const std::vector<int>& grupos,
                                   const std::vector<Vector2>& centroides, const std::vector<Vector2>& alunos,
                                   float pesoDoAluno);
    /// A inercia: a soma das distancias ao quadrado de cada ponto ao centroide do grupo dele. E o que
    /// o metodo do cotovelo olha.
    float Inercia(const std::vector<Vector2>& pontos, const std::vector<int>& grupos,
                  const std::vector<Vector2>& centroides);
    /// A celula de Voronoi do centroide 'i' dentro da caixa de 'de' a 'ate': o poligono convexo de quem
    /// esta mais perto dele que de qualquer outro.
    std::vector<Vector2> Celula(const std::vector<Vector2>& centroides, int i, Vector2 de, Vector2 ate);

    // ---- O ritmo ----

    struct Degrau {
        int pontos = 64;
        float entreIteracoes = 1.1f;
        float pesoDoAluno = 12.0f;
        float giroDoEixo = 240.0f;    ///< o quanto a fileira da PCA gira, em graus
        int tirosDoAnel = 20;         ///< o anel de quando o centroide chega
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O que cada iteracao anda: o centroide desliza ate a media nova em kDeslize segundos, e os
    /// pontos chegam kAperto mais perto do centroide deles (os grupos apertam).
    constexpr float kDeslize = 0.45f;
    constexpr float kAperto = 0.3f;
    /// Os centroides de cada parte: tres, e um quarto no cotovelo.
    constexpr int kGrupos = 3;
    constexpr float kCotovelo = 4.5f;   ///< segundos depois do comeco das k-medias
}
