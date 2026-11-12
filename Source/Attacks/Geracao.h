//
// A geracao de texto da fase 1 do Julio: um token de cada vez, cada um saindo
// da ponta da frase.
//

#pragma once

#include <vector>

#include "../Regras/Dificuldade.h"
#include "Temperatura.h"

/// Um modelo de linguagem gera um token de cada vez, e cada um depende do anterior: e isso que
/// "autorregressivo" quer dizer.
namespace Geracao {

    /// Um token a cada kEntreTokens, enquanto a frase e gerada.
    constexpr float kEntreTokens = 0.12f;

    /// O proximo token nasce a frente do anterior, kPasso pixels no rumo sorteado.
    constexpr float kPasso = 40.0f;

    /// O maximo que o rumo mais provavel vira para o aluno, por token, em graus.
    constexpr float kVirada = 12.0f;

    /// Os candidatos de cada token: kCandidatos rumos em kAbertura graus, em
    /// volta do mais provavel (Temperatura::DeAngulo).
    constexpr int kCandidatos = 7;
    constexpr float kAbertura = 100.0f;

    /// A frase de sempre, "o mo de lo te pre diz", tem sete tokens.
    constexpr int kTokensDaFrase = 7;

    /// A diferenca de 'de' para 'para', em graus, pelo menor arco: de -180 a 180.
    float Diferenca(float de, float para);

    /// O rumo mais provavel do proximo token: o do anterior, virado para o alvo no maximo 'virada'
    /// graus.
    float RumoProvavel(float rumo, float aoAlvo, float virada = kVirada);

    /// O sorteio de um token: os rumos candidatos, a chance de cada um e o
    /// escolhido. Os rumos sao absolutos, em graus (0 = direita, 90 = baixo).
    struct Passo {
        std::vector<float> rumos;
        std::vector<float> probabilidades;
        int escolhido = 0;
        [[nodiscard]] float Rumo() const { return rumos.empty() ? 0.0f : rumos[static_cast<size_t>(escolhido)]; }
    };

    /// O proximo token, saindo de 'rumo' com o alvo em 'aoAlvo', com a 'temperatura' da etapa e o
    /// numero 'u' do sorteio, em [0, 1).
    Passo Proximo(float rumo, float aoAlvo, float temperatura, float u);

    /// Quantos tokens a frase tem: os sete de sempre ate a temperatura 1,0; acima, a alucinacao
    /// passa do fim.
    int TokensDaFrase(float temperatura);

    /// O que o Dificil muda: uma frase a cada 1,5 s (e nao 2,0).
    struct Degrau {
        float entreFrases = 2.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
