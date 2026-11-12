//
// A repescagem da Maria Lucia: nao ha bala de prata.
//

#pragma once

#include <string>
#include <vector>

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

    /// O que o Dificil muda: uma bala a cada 1,4 s (e nao 1,7).
    struct Degrau {
        float entreBalas = 1.7f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
