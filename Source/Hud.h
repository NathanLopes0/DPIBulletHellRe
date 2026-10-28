//
// A HUD da batalha: a moldura do campo, o relogio da fase e os efeitos.
//

#pragma once

#include <vector>

#include "Math.h"

/**
 * CAMADA PURA. Coberto por test_hud.cpp.
 *
 * AS CONTAS DA HUD, sem SDL: onde fica a borda que o aluno nao atravessa,
 * quanto ele esta encostado nela, de que cor esta o relogio da fase, e quanto
 * dura cada faisca. Quem desenha sao DrawMolduraDoCampoComponent,
 * DrawRelogioDaFaseComponent e a Battle.
 */
namespace Hud {

    // ---- A MOLDURA DO CAMPO --------------------------------------------------------

    /// Um retangulo em pixels de tela.
    struct Caixa {
        float esquerda = 0.0f, topo = 0.0f, direita = 0.0f, baixo = 0.0f;
    };

    /**
     * @brief ATE ONDE O DESENHO DO ALUNO CHEGA, e nao o centro dele.
     *
     * Player::BorderLimitCheck prende o CENTRO a uma largura (e uma altura) de
     * sprite da borda do campo; o desenho, centrado, chega entao a MEIA
     * largura dela. A moldura e desenhada exatamente ali: o aluno encosta
     * nela, e nunca passa por cima.
     */
    Caixa AreaDoAluno(const Caixa& campo, float larguraDoSprite, float alturaDoSprite);

    /// O quanto o aluno esta encostado em cada parede, de 0 (longe) a 1 (colado).
    struct Encosto {
        float esquerda = 0.0f, direita = 0.0f, topo = 0.0f, baixo = 0.0f;
    };

    /// A partir de que distancia (px, da borda do desenho ate a parede) a
    /// parede comeca a acender.
    constexpr float kAvisoDaParede = 48.0f;

    /// @brief O encosto do aluno de centro 'centro' e meio tamanho 'meio'.
    Encosto Encostado(const Caixa& area, const Vector2& centro, const Vector2& meio);

    /// UM CLARAO da moldura: a cor dele some em 'duracao' segundos.
    /// @return a intensidade agora, 'idade' segundos depois de aceso (0 a 1).
    float Clarao(float idade, float duracao);

    // ---- O RELOGIO DA FASE ---------------------------------------------------------

    /// A cor do relogio com 'fracao' (0 a 1) do tempo ainda por vir: verde no
    /// comeco, ambar no meio, vermelho no fim.
    struct Rgb {
        unsigned char r, g, b;
    };
    Rgb CorDoRelogio(float fracao);

    /// OS ULTIMOS SEGUNDOS da fase: o relogio pulsa.
    constexpr float kUrgente = 3.0f;
    bool Urgente(float restante);

    /// Os dentes do relogio: um a cada 5 s, sem os das pontas.
    constexpr float kEntreMarcas = 5.0f;
    std::vector<float> Marcas(float duracao);   ///< fracoes de 0 a 1, de baixo para cima

    // ---- AS FAISCAS ----------------------------------------------------------------

    enum class Faisca {
        Acerto,      ///< o tiro do aluno no professor: miuda, branca
        Derrubada,   ///< o tiro do aluno derrubou um projetil: laranja, maior
        Dano,        ///< o aluno foi acertado: o anel vermelho que abre
        Coleta,      ///< o aluno pegou um Ponto Extra: o anel dourado
    };

    /// Quanto cada uma dura.
    float Vida(Faisca f);

    /// @brief O raio de uma faisca com 'idade' segundos: cresce e para.
    float Raio(Faisca f, float idade);

    /// @brief Quao acesa ela esta (0 a 1): some no fim da vida.
    float Brilho(Faisca f, float idade);
}
