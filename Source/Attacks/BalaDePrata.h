//
// A repescagem da Maria Lucia: nao ha bala de prata.
//

#pragma once

#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// "NAO HA BALA DE PRATA" - a frase que a professora repete, do ensaio de Fred Brooks (No Silver
/// Bullet, 1986).
namespace BalaDePrata {

    const std::vector<std::string>& Promessas();

    /// O gatilho da bala e o dos estilhacos (FasesDeAtaque.h, GatilhoExiste).
    constexpr const char* kGatilhoDaBala = "bala";
    constexpr const char* kGatilhoDoEstilhaco = "estilhaco";

    /// A bala se desfaz quando chega perto do aluno, a kPerto pixels - ou depois de kVooMaximo, se
    /// ele fugiu dela.
    constexpr float kPerto = 230.0f;
    constexpr float kVooMaximo = 2.4f;

    /// Se a bala, a 'distancia' do aluno e com 'voo' segundos no ar, ja se desfaz.
    bool SeDesfaz(float distancia, float voo);

    /// Quanto o placar mostra o que aconteceu com a ultima bala.
    constexpr float kNoPlacar = 1.2f;

    /// A ordem das promessas nesta luta: todas, embaralhadas - a primeira bala nao e sempre a
    /// mesma.
    std::vector<int> Sortear(Variacao::Sorteio& s);

    /// O placar da bala que se desfez sozinha: "IA? Nao ha bala de prata."
    std::string Desfeita(const std::string& promessa);

    /// O placar da bala que o aluno acertou: o teste com usuario a reprovou antes de ela chegar.
    std::string Testada(const std::string& promessa);

    // ---- O lobisomem (a repescagem refeita) ----
    //
    // Brooks: o projeto de software e um lobisomem, e todo mundo procura a bala de prata que o mate.
    // A professora atira as balas de prata - as promessas - no lobisomem, e nenhuma o mata: cada uma
    // se desfaz em bugs contra ele, e ele cresce (a complexidade essencial fica). Enquanto isso ele
    // persegue o aluno e uiva. Some com o Ponto Extra e volta: a complexidade essencial nao some.

    constexpr const char* kGatilhoDoLobisomem = "lobisomem";
    constexpr const char* kGatilhoDoUivo = "uivo";

    /// Quanto ele anda, 't' segundos depois de aparecer: devagar no comeco, mais depressa no fim.
    float RapidezDoLobisomem(float t);

    /// O lobisomem em 'lobo' dando um passo de 'dt' na direcao de 'alvo', sem passar dele.
    Vector2 PassoDoLobisomem(const Vector2& lobo, const Vector2& alvo, float rapidez, float dt);

    /// A bala em 'bala' chegou no lobisomem de 'escala'?
    bool Atingiu(const Vector2& bala, const Vector2& lobo, float escala);
    constexpr float kRaioDoLobisomem = 34.0f;

    /// A escala do lobisomem depois de 'balas' balas: cresce, ate kMaiorEscala.
    constexpr float kEscalaInicial = 1.3f;
    constexpr float kMaiorEscala = 2.1f;
    float EscalaDoLobisomem(int balas);

    constexpr float kEntreUivos = 3.0f;
    constexpr float kVolta = 1.5f;   ///< o lobisomem que sumiu volta depois disto

    /// O placar da bala que chegou nele: "IA? Não matou o lobisomem."
    std::string NaoMatou(const std::string& promessa);

    /// O que o Dificil muda: uma bala a cada 1,4 s (e nao 1,7).
    struct Degrau {
        float entreBalas = 1.7f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
