//
// A repescagem da Maria Lucia: nao ha bala de prata.
//

#pragma once

#include <string>
#include <vector>

#include "../Gameplay/Dificuldade.h"
#include "../Gameplay/Variacao.h"

/**
 * CAMADA PURA. Coberto por test_bala_de_prata.cpp.
 *
 * "NAO HA BALA DE PRATA" - a frase que a professora repete, do ensaio de Fred
 * Brooks (No Silver Bullet, 1986): nenhuma tecnica, sozinha, resolve a
 * complexidade ESSENCIAL de construir software. Na repescagem ela dispara
 * balas de prata, cada uma com o nome de uma promessa da industria no placar,
 * e cada uma SE DESFAZ no meio do caminho num leque de bugs: a complexidade
 * continua la.
 *
 * O TIRO DO ALUNO E UM TESTE COM USUARIO - a pesquisa dela, IHC e experiencia
 * do usuario: a bala atingida antes de se desfazer cai sem soltar nada, e o
 * placar diz que o teste a reprovou. Testar cedo e mais barato que descobrir
 * tarde.
 *
 * Quem dispara, mede o voo e escreve o placar e a Maria Lucia; aqui moram as
 * promessas, a ordem delas em cada luta, os textos e os relogios.
 */
namespace BalaDePrata {

    /// As promessas, uma por bala - o que ja foi vendido como a solucao. A
    /// tecnologia de 1986 do ensaio, e a de agora.
    const std::vector<std::string>& Promessas();

    /// O gatilho da bala e o dos estilhacos (FasesDeAtaque.h, GatilhoExiste).
    constexpr const char* kGatilhoDaBala = "bala";
    constexpr const char* kGatilhoDoEstilhaco = "estilhaco";

    /// A BALA SE DESFAZ QUANDO CHEGA PERTO DO ALUNO, a kPerto pixels - ou
    /// depois de kVooMaximo, se ele fugiu dela. Com um tempo fixo de voo ela
    /// se desfazia perto da professora, que anda na orbita junto com a bala, e
    /// os bugs nasciam longe de quem tinham de cobrar.
    constexpr float kPerto = 230.0f;
    constexpr float kVooMaximo = 2.4f;

    /// @brief Se a bala, a 'distancia' do aluno e com 'voo' segundos no ar, ja
    /// se desfaz.
    bool SeDesfaz(float distancia, float voo);


    /// Quanto o placar mostra o que aconteceu com a ultima bala.
    constexpr float kNoPlacar = 1.2f;

    /// @brief A ordem das promessas nesta luta: todas, embaralhadas - a
    /// primeira bala nao e sempre a mesma.
    std::vector<int> Sortear(Variacao::Sorteio& s);

    /// @brief O placar da bala que se desfez sozinha: "IA? Nao ha bala de prata."
    std::string Desfeita(const std::string& promessa);

    /// @brief O placar da bala que o aluno acertou: o teste com usuario a
    /// reprovou antes de ela chegar.
    std::string Testada(const std::string& promessa);

    /// @brief O que o Dificil muda: uma bala a cada 1,4 s (e nao 1,7).
    struct Degrau {
        float entreBalas = 1.7f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
