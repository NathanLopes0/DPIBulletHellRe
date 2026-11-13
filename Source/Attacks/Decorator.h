//
// O padrao Decorator da fase 3 da Maria Lucia.
//

#pragma once

#include <array>
#include <string>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"

/// A segunda metade da fase dos padroes de projeto: a bala que ganha decoradores. A professora solta
/// um componente simples, devagar, de um lado dela, para fora e para baixo; a cada kEntreCamadas ele
/// e embrulhado por mais um decorador, que lhe da um comportamento a mais - Rapida (anda mais),
/// Dividida (vira duas), Teleguiada (vira para o aluno, por kGuia segundos) -, e o placar escreve a
/// composicao: "new Teleguiada(new Dividida(new Rapida(bala)))". O tiro do aluno tira o decorador de
/// fora: a bala perde a camada e o comportamento dela, e so cai quando nao tem mais nenhuma.
///
/// O componente nasce ao lado da professora, e nao embaixo dela: embaixo ele estaria na linha dos
/// tiros de quem mira nela, e cairia antes de ganhar a primeira camada.
namespace Decorator {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kGatilhoDaBala = "componente";
    constexpr const char* kGatilhoDaDivisao = "dividida";

    enum class Camada { Rapida, Dividida, Teleguiada };
    constexpr std::array<Camada, 3> kOrdem = {Camada::Rapida, Camada::Dividida, Camada::Teleguiada};
    constexpr int kCamadas = static_cast<int>(kOrdem.size());

    /// O nome do decorador, como no codigo: "Rapida", "Dividida", "Teleguiada" (com acento na tela).
    const char* Nome(Camada c);

    /// A composicao de 'camadas' decoradores sobre a bala, de dentro para fora.
    std::string Placar(int camadas);

    /// A cara da bala com 'camadas' decoradores: "D0" a "D3".
    std::string Cara(int camadas);

    constexpr float kEntreCamadas = 0.75f;   ///< segundos entre um decorador e o seguinte
    constexpr float kFatorDaRapida = 1.6f;   ///< o que a Rapida multiplica na velocidade
    constexpr float kAberturaDaDividida = 26.0f;   ///< graus entre as duas metades
    constexpr float kGiroDaTeleguiada = 105.0f;    ///< graus por segundo, virando para o aluno
    constexpr float kGuia = 2.2f;                  ///< segundos em que a Teleguiada vira; depois, reta
    constexpr float kVelocidadeDoComponente = 125.0f;

    /// Onde o componente nasce: kAoLado da professora, de um lado e do outro, alternando.
    constexpr float kAoLado = 120.0f;
    /// Para onde ele sai: kSaida graus para fora da vertical, para baixo.
    constexpr float kSaida = 40.0f;

    /// O angulo de saida, em graus (0 e a direita, 90 e para baixo), do componente que nasce do lado
    /// 'lado' (+1 a direita da professora, -1 a esquerda).
    float Saida(int lado);

    /// A Teleguiada: o rumo 'agora' (em radianos) virado na direcao de 'falta' (do projetil ao aluno),
    /// no maximo kGiroDaTeleguiada graus por segundo, pelo lado mais curto.
    float Guiar(float agora, Vector2 falta, float deltaTime);

    /// O que cada decorador faz, para o placar: "Rápida: anda 1,6× mais", "Dividida: vira duas",
    /// "Teleguiada: persegue você". E o que sai quando o aluno o tira.
    std::string OQueFaz(Camada c);
    std::string OQueSai(Camada c);

    /// O que o Dificil muda: um componente a cada 0,95 s (e nao 1,15).
    struct Degrau {
        float entreComponentes = 1.15f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
