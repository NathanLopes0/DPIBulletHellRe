//
// A repescagem da Sabrina: o sequenciamento - PCR, shotgun e CRISPR.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Pareamento.h"

/// A repescagem da Sabrina e o laboratorio de genomica, e cada etapa pede a mao do aluno:
///   - na PCR, cada ciclo de temperatura dobra as copias: a 95 graus a dupla fita se separa, a 55 os
///     primers anelam, a 72 a polimerase estende. O tiro do aluno derruba o primer - e a fita sem
///     primer nao se copia. Quem nao atira ve as copias crescerem em exponencial;
///   - no shotgun, a professora dispara reads em leque e eles se montam no contig, que gira no meio
///     do campo como um ponteiro e cresce a cada read que encosta. So as pontas se deixam aparar (as
///     de baixa qualidade): o resto e o consenso, e o tiro morre nele;
///   - no CRISPR, a fita do genoma desce como uma parede, e o guia esta no placar. A Cas9 so corta
///     onde o guia casa e logo depois vem o PAM (N G G): o aluno acha o alvo, atira nele, e a fita
///     abre ali. O resto e blindado - e a Cas9 da professora, fora do alvo, solta estilhacos.
namespace Sequenciamento {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 3> kEtapas = {"Pcr", "Shotgun", "Crispr"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDaCopia = "copia";
    constexpr const char* kGatilhoDoPrimer = "primer";
    constexpr const char* kGatilhoDoRead = "read";
    constexpr const char* kGatilhoDoContig = "contig";
    constexpr const char* kGatilhoDoGenoma = "genoma";
    constexpr const char* kGatilhoDoEstilhaco = "estilhaco";
    constexpr const char* kGatilhoDaTesoura = "tesoura";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A PCR ----

    enum class Passo { Desnatura, Anela, Estende };
    /// Onde cada passo acaba, na fracao do ciclo.
    constexpr float kFimDaDesnaturacao = 0.3f;
    constexpr float kFimDoAnelamento = 0.55f;
    struct Ciclo {
        int numero = 0;
        Passo passo = Passo::Desnatura;
        float temperatura = 95.0f;
    };
    Ciclo CicloNoTempo(float t, float periodo);
    /// Quantas copias ha depois de 'ciclos' ciclos sem nenhum primer derrubado: o dobro a cada um,
    /// ate o 'teto'.
    int Copias(int moldes, int ciclos, int teto);

    // ---- O shotgun ----

    /// As contas do contig: da ponta de tras (-'atras') ate a da frente (+'frente'), de 'espacamento'
    /// em 'espacamento', na reta que passa por 'centro' no angulo 'giro'. A conta 0 e o centro.
    std::vector<Vector2> Contig(Vector2 centro, int atras, int frente, float espacamento, float giro);
    /// A distancia de 'p' ao segmento de 'a' a 'b'.
    float DistanciaAoSegmento(Vector2 p, Vector2 a, Vector2 b);
    /// Se o read em 'p' encosta no contig (a 'alcance' da reta dele): a ponta mais perto do toque
    /// (-1 a de tras, +1 a da frente), ou 0 se nao encosta.
    int Encosta(Vector2 p, Vector2 centro, int atras, int frente, float espacamento, float giro, float alcance);

    // ---- O CRISPR ----

    /// O PAM da Cas9 comeca em 'i': N G G (qualquer base, e dois G).
    bool Pam(const std::vector<Pareamento::Base>& fita, int i);
    /// Quantas vezes o guia aparece seguido do PAM, e onde a primeira comeca (-1, nenhuma).
    int Procurar(const std::vector<Pareamento::Base>& fita, const std::vector<Pareamento::Base>& guia,
                 int* quantas = nullptr);
    /// Um guia de 'n' bases de DNA.
    std::vector<Pareamento::Base> Guia(Variacao::Sorteio& s, int n);
    struct Genoma {
        std::vector<Pareamento::Base> fita;
        int alvo = -1;               ///< onde o guia comeca
        std::vector<int> iscas;      ///< onde comecam os quase-guias (um erro), cada um com PAM depois
    };
    /// Uma fita de 'n' bases com o guia + PAM em um lugar so, e 'iscas' quase-guias com PAM.
    Genoma Montar(Variacao::Sorteio& s, int n, const std::vector<Pareamento::Base>& guia, int iscas);

    // ---- O ritmo ----

    struct Degrau {
        // A PCR
        float periodo = 1.0f;
        int moldes = 2;
        int teto = 44;
        float rapidezMinima = 70.0f;
        float rapidezMaxima = 130.0f;
        int resistenciaDoPrimer = 1;
        // O shotgun
        float espacamento = 22.0f;
        int pontaInicial = 2;
        int pontaMaxima = 17;
        float giro = 0.9f;
        float entreRajadas = 0.45f;
        int tirosDaRajada = 7;
        float aberturaDaRajada = 56.0f;
        float velocidadeDoRead = 200.0f;
        float alcanceDoContig = 13.0f;
        // O CRISPR
        int basesDoGenoma = 26;
        int tamanhoDoGuia = 4;
        int iscas = 3;
        float descida = 170.0f;
        float entreFitas = 3.2f;
        int resistenciaDoAlvo = 1;
        int tirosForaDoAlvo = 5;
        float velocidadeForaDoAlvo = 185.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
