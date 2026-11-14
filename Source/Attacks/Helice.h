//
// A fase 1 da Sabrina: a dupla helice - o modelo 3D que gira.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Pareamento.h"

/// A fase 1 da Sabrina e a aula de estrutura: a molecula e um modelo 3D girando na tela, como nos
/// visualizadores de molecula, e so a parte da FRENTE machuca - a de tras e o contorno apagado, por
/// onde se passa:
///   - na helice, a dupla helice fica de pe e anda de um lado ao outro do campo, girando; a fita
///     molde solta a base de cada par quando ele passa pela borda;
///   - na torcao, ela deita e desce sobre o aluno como uma rosca: as brechas da fita de tras andam de
///     lado, e quem atravessa anda junto. A torcao aumenta (o superenrolamento);
///   - na desnaturacao, o calor sobe e os pares se soltam: A-T, com duas pontes de hidrogenio, antes
///     de G-C, com tres (e os vizinhos ajudam a segurar). O par que solta arremessa as duas bases no
///     aluno, e a helice incha onde ja abriu.
namespace Helice {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr std::array<const char*, 3> kEtapas = {"Helice", "Torcao", "Desnaturacao"};
    constexpr std::array<float, 3> kDuracoes = {7.0f, 6.0f, 7.0f};
    constexpr const char* kGatilhoDaFita = "fita";
    constexpr const char* kGatilhoDaBase = "base";
    constexpr const char* kGatilhoDoLeque = "leque";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- O espaco ----

    /// Um ponto do modelo: x para a direita, y para baixo (como na tela) e z para fora dela, na
    /// direcao de quem joga.
    struct Ponto3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };
    /// Gira em volta do eixo vertical: x e z trocam.
    Ponto3 GirarEmY(Ponto3 p, float angulo);
    /// Gira em volta do eixo horizontal: y e z trocam.
    Ponto3 GirarEmX(Ponto3 p, float angulo);

    /// O ponto visto por uma camera a 'distancia' do plano da tela: o que vem para a frente cresce e
    /// se afasta do centro; o que vai para tras encolhe.
    struct NaTela {
        Vector2 onde;
        float escala = 1.0f;
    };
    NaTela Projetar(Ponto3 p, Vector2 centro, float distancia);

    /// Se a conta esta na frente: o z passa de 'limiar' vezes o raio.
    bool NaFrente(float z, float raio, float limiar);

    // ---- A dupla helice ----

    /// O angulo da fita (0 ou 1) na altura 's' (em pares): a fita 1 vem 'sulco' radianos depois - e o
    /// que faz o sulco maior e o menor.
    float Angulo(int fita, float s, float giro, float torcao, float sulco);
    /// O ponto da fita de pe (o eixo vertical): os pares descem de 'passo' em 'passo'.
    Ponto3 DePe(float s, float raio, float passo, float angulo);
    /// O ponto da fita deitada (o eixo horizontal): os pares andam para a direita.
    Ponto3 Deitada(float s, float raio, float passo, float angulo);

    /// A borda que o angulo cruzou indo de 'antes' a 'depois': +1 a direita (passou por um multiplo
    /// par de pi), -1 a esquerda (impar), 0 nenhuma.
    int Borda(float antes, float depois);

    /// Quanto as brechas de uma rosca andam de lado, em px por segundo: o giro sobre a torcao.
    float AndarDaRosca(float giroPorSegundo, float torcaoPorPx);

    // ---- A desnaturacao ----

    /// Uma fita molde de 'n' bases (os pares sao ela e a complementar), em blocos ricos em A-T e em
    /// G-C: a ordem da fusao fica legivel.
    std::vector<Pareamento::Base> Sortear(Variacao::Sorteio& s, int n);
    /// A temperatura (graus) em que o par 'k' solta: as pontes dele e, um pouco, as dos vizinhos.
    float Fusao(const std::vector<Pareamento::Base>& fita, int k);
    /// A temperatura no instante 't' da etapa: sobe em reta de 'inicial' a 'final'.
    float Temperatura(float t, float duracao, float inicial, float final);
    /// O raio do par 'desde' segundos depois de soltar: incha ate (1 + inchaco) vezes 'raio' em
    /// 'tempo' segundos. Antes de soltar ('desde' negativo), o raio.
    float RaioDoPar(float desde, float raio, float inchaco, float tempo);

    // ---- O ritmo ----

    struct Degrau {
        // A helice de pe
        int pares = 14;
        float raio = 88.0f;
        float passo = 42.0f;
        float torcao = 0.6f;            ///< radianos por par
        float sulco = 2.5f;             ///< radianos entre as fitas
        float giro = 1.6f;              ///< radianos por segundo
        float limiar = 0.25f;           ///< a fracao do raio a partir da qual a frente machuca
        float varredura = 0.24f;        ///< a fracao da largura
        float periodo = 7.0f;
        int contasPorPar = 3;
        float velocidadeDaBase = 165.0f;
        int aCadaPar = 2;               ///< so um par a cada tantos solta a base pela borda
        bool duasFitasAtiram = false;
        // A torcao
        float raioDeitada = 62.0f;
        float passoDeitada = 30.0f;     ///< px por par
        float torcaoInicial = 0.36f;    ///< radianos por par
        float torcaoFinal = 0.56f;
        float giroDeitada = 2.3f;
        float periodoDaDescida = 6.0f;
        int contasPorParDeitada = 2;
        float entreLeques = 1.2f;
        int tirosDoLeque = 3;
        float velocidadeDoLeque = 200.0f;
        // A desnaturacao
        float temperaturaInicial = 70.0f;
        float temperaturaFinal = 100.0f;
        float inchaco = 0.85f;
        float tempoDoInchaco = 0.6f;
        float giroQuente = 3.0f;        ///< o giro no fim: o calor agita
        int tirosDaFusao = 1;           ///< por base solta
        float aberturaDaFusao = 16.0f;
        float velocidadeDaFusao = 195.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
