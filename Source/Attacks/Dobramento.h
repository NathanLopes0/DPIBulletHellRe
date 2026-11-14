//
// A fase 3 da Sabrina: a proteina - a cadeia, o dobramento e o encaixe.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "Helice.h"

/// A fase 3 da Sabrina e a da proteina, do fio ao formato:
///   - na cadeia, o ribossomo solta o polipeptideo, que persegue o aluno como uma cobra e cresce um
///     aminoacido por vez. O tiro do aluno e a protease: o residuo atingido corta a cadeia ali, e o
///     pedaco da ponta se desfaz;
///   - no dobramento, a cadeia colapsa num globulo (os hidrofobicos se escondem no nucleo, os polares
///     ficam na superficie) e o modelo 3D rola pelo campo, girando: como na fase 1, so a frente
///     machuca, e os polares da frente borrifam agua para fora;
///   - no encaixe, a proteina abre o sitio ativo: um anel com uma boca que gira. Dentro dele, o aluno
///     e o ligante encaixado - a agua e os inibidores morrem na parede, e a afinidade sobe (o Delta G
///     cai) -, mas o tiro do aluno tambem morre na parede: so sai pela boca.
namespace Dobramento {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateThree";
    constexpr std::array<const char*, 3> kEtapas = {"Cadeia", "Dobramento", "Encaixe"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 7.0f, 7.0f};
    constexpr const char* kGatilhoDoResiduo = "residuo";
    constexpr const char* kGatilhoDoSolto = "solto";
    constexpr const char* kGatilhoDoAtomo = "atomo";
    constexpr const char* kGatilhoDaAgua = "agua";
    constexpr const char* kGatilhoDoSitio = "sitio";
    constexpr const char* kGatilhoDoInibidor = "inibidor";
    constexpr const char* kGatilhoDoLeque = "aminoacido";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A cadeia ----

    /// O seguir-o-lider: a primeira conta vai para 'cabeca', e cada outra fica a 'espacamento' da
    /// anterior, na direcao em que ja estava.
    void Arrastar(std::vector<Vector2>& contas, Vector2 cabeca, float espacamento);

    /// Cortar a cadeia no residuo 'i': ele e o que vem depois saem (e voltam); 'i' fica de tamanho.
    std::vector<Vector2> Cortar(std::vector<Vector2>& contas, int i);

    // ---- O dobramento ----

    /// Onde cada residuo fica no globulo dobrado (o modelo, centrado na origem, de raio 1): os
    /// hidrofobicos no nucleo, de raio 'nucleo', e os outros na superficie, espalhados pela espiral de
    /// Fibonacci. Na ordem dos residuos.
    std::vector<Helice::Ponto3> Globulo(const std::vector<bool>& hidrofobico, float nucleo);

    /// O passo de quem rola no campo: a velocidade reflete na parede de 'campo' encolhido de 'raio'.
    /// Devolve a nova posicao; 'velocidade' muda quando bate.
    Vector2 Quicar(Vector2 onde, Vector2& velocidade, float dt, float raio, Vector2 deCanto, Vector2 ateCanto);

    // ---- O encaixe ----

    /// As 'n' contas da parede do sitio: um arco de circulo de 'raio' em volta de 'centro', deixando a
    /// boca de 'abertura' radianos centrada no angulo 'boca'.
    std::vector<Vector2> Sitio(Vector2 centro, float raio, int n, float boca, float abertura);
    /// Se 'p' esta encaixado: dentro do sitio, a 'folga' da parede.
    bool Encaixado(Vector2 p, Vector2 centro, float raio, float folga);
    /// O Delta G (kcal/mol, negativo) de quem ficou 'segundos' encaixado: quanto mais tempo, mais
    /// forte a ligacao.
    float DeltaG(float segundos);

    /// O movimento browniano da agua: a velocidade ganha o 'chute' e volta aos poucos para a 'media'.
    Vector2 Browniano(Vector2 velocidade, Vector2 media, Vector2 chute, float dt, float retorno);

    // ---- O ritmo ----

    struct Degrau {
        // A cadeia
        int residuosIniciais = 9;
        int residuosMaximos = 28;
        float entreResiduos = 0.25f;
        float espacamento = 17.0f;
        float rapidez = 210.0f;
        float giro = 2.9f;              ///< radianos por segundo
        int resistencia = 1;
        float vidaDosSoltos = 1.0f;
        float voltaDaCadeia = 0.7f;     ///< quando a cadeia acaba, a proxima sai do ribossomo
        float entreLeques = 1.1f;
        int tirosDoLeque = 5;
        float velocidadeDoLeque = 200.0f;
        // O dobramento
        int residuosDoGlobulo = 26;
        float raioDoGlobulo = 116.0f;
        float nucleo = 0.36f;
        float colapso = 1.3f;
        float giroY = 1.1f;
        float giroX = 0.55f;
        float limiar = 0.15f;
        float rapidezDoGlobulo = 150.0f;
        float perseguicao = 0.9f;       ///< radianos por segundo: o globulo rola atras do aluno
        float entreBorrifos = 0.55f;
        float velocidadeDoBorrifo = 200.0f;
        int tirosDoBorrifo = 2;         ///< por polar da frente
        // O encaixe
        float raioDoSitio = 76.0f;
        int contasDoSitio = 20;
        float abertura = 1.05f;
        float giroDaBoca = 0.75f;
        float passeioDoSitio = 0.24f;   ///< a fracao da largura
        float entreGotas = 0.045f;
        float velocidadeDaGota = 150.0f;
        float chute = 900.0f;           ///< o empurrao aleatorio, px/s por segundo
        float entreInibidores = 0.7f;
        int tirosDoInibidor = 4;
        float velocidadeDoInibidor = 225.0f;
        float premioPorSegundo = 0.8f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
