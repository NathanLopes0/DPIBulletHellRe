//
// A geracao de texto da fase 1 do Julio: um token de cada vez, cada um saindo
// da ponta da frase.
//

#pragma once

#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../Shared/Temperatura.h"

/**
 * CAMADA PURA. Coberto por test_geracao.cpp.
 *
 * UM MODELO DE LINGUAGEM GERA UM TOKEN DE CADA VEZ, e cada um depende do
 * anterior: e isso que "autorregressivo" quer dizer. A fase 1 do Julio desenha
 * isso no campo. A frase nao sai mais inteira do professor, como uma rajada:
 * cada token nasce NA PONTA da frase, onde o anterior esta, e o rumo dele e
 * sorteado com a temperatura da etapa em volta do rumo do anterior - virado
 * um pouco para o aluno. A frase vira uma cobra de texto que se curva pelo
 * campo: fria, uma curva lisa que vem na direcao dele; quente, um zigue-zague
 * que alucina.
 *
 * E na alucinacao o modelo nao para no "diz": a frase quente passa do fim e
 * continua (TokensDaFrase).
 */
namespace Geracao {

    /// Um token a cada kEntreTokens, enquanto a frase e gerada.
    constexpr float kEntreTokens = 0.12f;

    /// O PROXIMO TOKEN NASCE A FRENTE DO ANTERIOR, kPasso pixels no rumo
    /// sorteado: a frase cresce para a frente, como texto sendo escrito, e e
    /// isso que desenha a curva. Nascendo em cima do anterior, os tokens so se
    /// separavam pela diferenca de rumo e a frase virava um bolo.
    constexpr float kPasso = 40.0f;


    /// O MAXIMO QUE O RUMO MAIS PROVAVEL VIRA PARA O ALUNO, por token, em
    /// graus. E o que deixa a frase fria desviavel: ela vem na direcao do
    /// aluno numa curva, e nao como um tiro que corrige a mira a cada token.
    constexpr float kVirada = 12.0f;

    /// Os candidatos de cada token: kCandidatos rumos em kAbertura graus, em
    /// volta do mais provavel (Temperatura::DeAngulo).
    constexpr int kCandidatos = 7;
    constexpr float kAbertura = 100.0f;

    /// A frase de sempre, "o mo de lo te pre diz", tem sete tokens.
    constexpr int kTokensDaFrase = 7;

    /// @brief A diferenca de 'de' para 'para', em graus, pelo menor arco: de
    /// -180 a 180.
    float Diferenca(float de, float para);

    /// @brief O rumo mais provavel do proximo token: o do anterior, virado
    /// para o alvo no maximo 'virada' graus.
    float RumoProvavel(float rumo, float aoAlvo, float virada = kVirada);

    /// O sorteio de um token: os rumos candidatos, a chance de cada um e o
    /// escolhido. Os rumos sao absolutos, em graus (0 = direita, 90 = baixo).
    struct Passo {
        std::vector<float> rumos;
        std::vector<float> probabilidades;
        int escolhido = 0;
        [[nodiscard]] float Rumo() const { return rumos.empty() ? 0.0f : rumos[static_cast<size_t>(escolhido)]; }
    };

    /// @brief O proximo token, saindo de 'rumo' com o alvo em 'aoAlvo', com a
    /// 'temperatura' da etapa e o numero 'u' do sorteio, em [0, 1).
    Passo Proximo(float rumo, float aoAlvo, float temperatura, float u);

    /// @brief Quantos tokens a frase tem: os sete de sempre ate a temperatura
    /// 1,0; acima, a alucinacao passa do fim - um a mais a cada 0,2 de
    /// temperatura (quatro a mais em 1,8, seis em 2,2).
    int TokensDaFrase(float temperatura);

    /// @brief O que o Dificil muda: uma frase a cada 1,5 s (e nao 2,0).
    struct Degrau {
        float entreFrases = 2.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
