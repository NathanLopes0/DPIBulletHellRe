//
// O Scrum da fase 2 da Maria Lucia: o quadro, as sprints e as dailies.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// A fase 2 e uma sequencia de sprints de tempo fixo (o timebox). Em cada uma:
///   - o planning: as historias caem na coluna A FAZER, cada uma com os pontos que o planning poker
///     deu (1, 2, 3, 5 ou 8);
///   - as dailies: a cada daily o cursor da professora arrasta o quadro uma coluna para a direita -
///     A FAZER, FAZENDO, FEITO. Na FAZENDO a historia trabalha: atira, e a mais pesada atira mais;
///   - a entrega: o que estava FEITO cai como incremento, no campo do aluno, e estoura embaixo.
/// O aluno derruba historia: tirar escopo da sprint. O grafico de burndown no canto mostra os pontos
/// que faltam, daily a daily. Cada sprint traz historias novas: o Ponto Extra limpa o quadro, e a
/// sprint seguinte o enche de novo.
namespace Scrum {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateTwo";
    constexpr const char* kGatilhoDaHistoria = "historia";
    constexpr const char* kGatilhoDoTrabalho = "trabalho";
    constexpr const char* kGatilhoDaEntrega = "entrega";
    constexpr const char* kGatilhoDaDaily = "daily";
    constexpr const char* kGatilhoDoCursor = "cursor";
    constexpr const char* kGatilhoDoTitulo = "titulo";

    constexpr int kSprints = 5;
    constexpr int kColunas = 3;
    /// Antes da primeira sprint, o backlog; depois da ultima, a retrospectiva.
    constexpr float kBacklog = 0.75f;
    constexpr float kSprint = 3.6f;
    constexpr float kRetrospectiva = Fase::kDuracao - kBacklog - kSprint * kSprints;

    /// As dailies, em segundos desde o comeco da sprint.
    constexpr std::array<float, 3> kDailies = {0.7f, 1.6f, 2.5f};

    /// A sprint no instante 't' da fase: -1 no backlog, 0 a 4, e kSprints na retrospectiva.
    int SprintNoTempo(float t);
    float InicioDaSprint(int sprint);

    /// Quantas dailies ja houve 'dentro' segundos depois do comeco da sprint (0 a 3).
    int Dailies(float dentro);

    /// Quantas historias a sprint 'sprint' (0 a 4) planeja: mais a cada sprint (a velocidade sobe).
    int Historias(int sprint, Dificuldade::Nivel n);

    /// Os pontos de 'n' historias, tirados do baralho do planning poker.
    constexpr std::array<int, 5> kBaralho = {1, 2, 3, 5, 8};
    std::vector<int> Pontos(Variacao::Sorteio& s, int n);

    /// O lugar da historia 'posicao' (de cima para baixo) na coluna 'coluna' (0 A FAZER, 1 FAZENDO,
    /// 2 FEITO) do quadro que comeca em 'esquerda', tem 'largura' e o topo em 'topo'.
    Vector2 Vaga(int coluna, int posicao, float esquerda, float largura, float topo);
    constexpr float kEntreHistorias = 40.0f;
    constexpr float kTopoDasVagas = 34.0f;    ///< abaixo do topo do quadro: o titulo da coluna

    /// Quantos tiros a historia de 'pontos' solta a cada vez que trabalha, na FAZENDO.
    int TirosDaHistoria(int pontos);
    constexpr float kEntreTrabalhos = 0.6f;

    /// Quantas dailies a historia leva para ser entregue: duas para chegar a FEITO, a terceira a solta.
    constexpr int kDailiesAteAEntrega = 3;

    /// O placar da sprint: "Sprint 3 · daily 2 · faltam 5 pontos".
    std::string Placar(int sprint, int dailies, int faltam);

    /// Os titulos das colunas, para as caras em projeteis.json.
    constexpr std::array<const char*, kColunas> kTitulos = {"AFazer", "Fazendo", "Feito"};

    /// A cara da historia de 'pontos' pontos: "P1", "P2", "P3", "P5" ou "P8".
    std::string Cara(int pontos);
}
