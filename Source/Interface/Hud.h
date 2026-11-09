//
// A HUD da batalha: a moldura do campo, o relogio da fase e os efeitos.
//

#pragma once

#include <cstddef>
#include <vector>

#include "../Motor/Math.h"

/// Quem desenha sao DrawMolduraDoCampoComponent, DrawRelogioDaFaseComponent e a Battle.
namespace Hud {

    // ---- A moldura do campo ----

    /// Um retangulo em pixels de tela.
    struct Caixa {
        float esquerda = 0.0f, topo = 0.0f, direita = 0.0f, baixo = 0.0f;
    };

    /// Ate onde o desenho do aluno chega, e nao o centro dele.
    Caixa AreaDoAluno(const Caixa& campo, float larguraDoSprite, float alturaDoSprite);

    /// O quanto o aluno esta encostado em cada parede, de 0 (longe) a 1 (colado).
    struct Encosto {
        float esquerda = 0.0f, direita = 0.0f, topo = 0.0f, baixo = 0.0f;
    };

    /// A partir de que distancia (px, da borda do desenho ate a parede) a
    /// parede comeca a acender.
    constexpr float kAvisoDaParede = 48.0f;

    /// O encosto do aluno de centro 'centro' e meio tamanho 'meio'.
    Encosto Encostado(const Caixa& area, const Vector2& centro, const Vector2& meio);

    /// Um clarao da moldura: a cor dele some em 'duracao' segundos.
    float Clarao(float idade, float duracao);

    // ---- O relogio da fase ----

    /// A cor do relogio com 'fracao' (0 a 1) do tempo ainda por vir: verde no
    /// comeco, ambar no meio, vermelho no fim.
    struct Rgb {
        unsigned char r, g, b;
    };
    Rgb CorDoRelogio(float fracao);

    /// Os ultimos segundos da fase: o relogio pulsa.
    constexpr float kUrgente = 3.0f;
    bool Urgente(float restante);

    /// Os dentes do relogio: um a cada 5 s, sem os das pontas.
    constexpr float kEntreMarcas = 5.0f;
    std::vector<float> Marcas(float duracao);   ///< fracoes de 0 a 1, de baixo para cima

    // ---- As faiscas ----

    enum class Faisca {
        Acerto,      ///< o tiro do aluno no professor: miuda, branca
        Derrubada,   ///< o tiro do aluno derrubou um projetil: laranja, maior
        Dano,        ///< o aluno foi acertado: o anel vermelho que abre
        Coleta,      ///< o aluno pegou um Ponto Extra: o anel dourado
        Dissolvido,  ///< o tiro que a onda do Ponto Extra desfez: o brilho dourado
        Resposta,    ///< a Duvida acertou o professor: o "?" vira um "!" que sobe e some
        RespostaDePerto,  ///< a Duvida perguntada de perto (Nota::FatorDePerto): "!!"
        Raspao,      ///< um projetil passou rente ao aluno sem acerta-lo (Raspao.h): o risco azul
        Cobertura,   ///< na dupla, o tiro derrubou o projetil que ia no parceiro: a derrubada dourada
    };

    /// Quanto cada uma dura.
    float Vida(Faisca f);

    /// O raio de uma faisca com 'idade' segundos: cresce e para.
    float Raio(Faisca f, float idade);

    /// Quao acesa ela esta (0 a 1): some no fim da vida.
    float Brilho(Faisca f, float idade);

    // ---- A duvida (o tiro do aluno) ----

    /// A cor do aluno 'quem' (0 ou 1): azul o primeiro, laranja o segundo.
    Rgb CorDoAluno(std::size_t quem);

    /// O balao que o escudo barra murcha e fica cinza, em vez de sumir: a pergunta que nao foi
    /// ouvida. Dura isto.
    constexpr float kMurcha = 0.3f;

    /// O tamanho do balao 'idade' segundos depois de murchar: de 1 ate 0,3, devagar no fim.
    float Murcho(float idade);
}
