//
// A fase 2 do Lucas: o DevTools aberto, e o CSS mudando ao vivo.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"
#include "../Shared/Flex.h"

/**
 * CAMADA PURA. Coberto por test_estilo.cpp.
 *
 * O LUCAS ABRE O DEVTOOLS E EDITA O CSS AO VIVO. No campo fica uma grade de
 * cartoes - os .card de uma pagina, 3 linhas de 6 -, e a cada kPorRegra o
 * placar escreve uma regra nova, que transforma a grade INTEIRA de uma vez
 * (com a transition de sempre, 0,5 s):
 *
 *   gap: 96px                      os cartoes se afastam - e voltam com 56px
 *   justify-content: space-between cada linha se redistribui no conteiner,
 *                                  como o flexbox faz (Flex::Posicoes)
 *   flex-direction: column         a grade deita: o eixo principal fica vertical
 *   transform: rotate(45deg)       a grade gira em volta do centro dela
 *   transform: scale(1.6)          a grade cresce - e chega mais perto de voce
 *   display: none                  uma linha flex some: e o vao por onde passar
 *   animation: spin 2s             a grade gira sem parar por 2 s
 *   position: fixed; bottom: 0     a grade CAI ate o chao, espalhada de borda
 *                                  a borda, e outra nasce no lugar dela
 *   :hover { transform: ... }      os cartoes em cima do aluno - onde o mouse
 *                                  esta - descem ate ele e voltam
 *
 * E A CADA REGRA O NAVEGADOR REPINTA: quando a transicao termina, cada
 * cartao solta um tiro reto para baixo (o gatilho "repintar"). As colunas de
 * tiros sao a forma da grade naquele instante - girada, espalhada, sem a linha
 * escondida -, e lendo a regra no placar o aluno sabe onde vai ter vao antes
 * de a grade chegar la. O :hover e o position: fixed nao repintam: o ataque
 * deles e o proprio cartao.
 *
 * A GRADE FICA DO OUTRO LADO DO PROFESSOR (XDaGrade): ele varre o alto
 * devagar, e a grade anda no espelho dele - o painel do DevTools aberto de um
 * lado, a pagina do outro. Embaixo do professor, como era, as colunas do
 * repaint cobriam a coluna dele, e quem desviava nunca o acertava (terminava a
 * fase com 42, tomando so 1,4 acerto).
 */
namespace Estilo {

    constexpr const char* kFaseDoEstilo = "StateTwo";
    constexpr const char* kPostoDaGrade = "grade";
    constexpr const char* kGatilhoDoCartao = "cartao";   ///< um cartao que nasce do professor
    constexpr const char* kGatilhoDoRepaint = "repintar"; ///< o tiro de cada cartao no repaint

    constexpr int kLinhas = 3;
    constexpr int kColunas = 6;
    constexpr int kCartoes = kLinhas * kColunas;

    constexpr float kGapEstreito = 56.0f;
    constexpr float kGapLargo = 96.0f;
    constexpr float kEscalaGrande = 1.6f;
    constexpr float kYDaGrade = 290.0f;
    constexpr float kConteiner = 640.0f;   ///< o tamanho do conteiner no eixo principal

    enum class Tipo { Gap, Justificar, Direcao, Girar, Escalar, Esconder, Animar, Fixar, Hover };

    /// O estado do CSS da grade: o que cada regra ja mudou.
    struct Css {
        float gap = kGapEstreito;
        Flex::Justificar justificar = Flex::Justificar::Centro;
        bool coluna = false;
        float rotacao = 0.0f;      ///< graus
        float escala = 1.0f;
        int linhaOculta = -1;      ///< a linha flex com display: none; -1 nenhuma
    };

    struct Regra {
        Tipo tipo = Tipo::Gap;
        std::string seletor;       ///< ".card", ".card:nth-child(2)", ".card:hover"
        std::string declaracao;    ///< "transform: rotate(45deg);"
    };

    /// @brief A proxima regra, a partir do CSS de agora - e o CSS ja com ela.
    /// Nunca o mesmo tipo duas vezes seguidas; a grade girada ou grande tende a
    /// voltar (rotate(0), scale(1)), para nao ficar torta a fase inteira.
    Regra Proxima(Variacao::Sorteio& s, Css& css, Tipo anterior);

    /// @brief Esta regra repinta? Todas, menos o :hover e o position: fixed.
    bool Repinta(Tipo t);

    /// @brief O placar: o seletor e, embaixo, a declaracao.
    std::string Placar(const Regra& r);

    struct Ponto {
        float x = 0.0f;
        float y = 0.0f;
    };

    /// @brief O lugar de cada cartao, pelo indice (linha * kColunas + coluna
    /// no layout de linhas), com o CSS 'css', a grade centrada em 'centro' e
    /// 'giro' graus a mais da animation. O cartao da linha oculta tambem tem
    /// lugar - quem o esconde e quem chama, com Oculto.
    std::array<Ponto, kCartoes> Posicoes(const Css& css, Ponto centro, float giro = 0.0f);

    /// @brief O cartao 'i' esta na linha com display: none?
    bool Oculto(const Css& css, int i);

    /// @brief A queda do position: fixed: o lugar do cartao 'i' espalhado de
    /// borda a borda numa fileira so, na altura 'y'.
    Ponto Espalhado(int i, float largura, float y);

    /// O RELOGIO: uma regra a cada kPorRegra; a animation gira kGiroPorSegundo
    /// por kDuracaoDoGiro; o :hover desce kDescidaDoHover em kDuracaoDoHover.
    constexpr float kPrimeira = 0.8f;
    constexpr float kPorRegra = 2.0f;
    constexpr float kDuracaoDoGiro = 2.0f;
    constexpr float kGiroPorSegundo = 120.0f;
    constexpr float kDescidaDoHover = 300.0f;
    constexpr float kDuracaoDoHover = 1.0f;
    constexpr float kAlcanceDoHover = 90.0f;   ///< px do aluno, no x
    constexpr float kQuedaDoFixo = 330.0f;     ///< px/s
    constexpr float kEspalhar = 0.45f;         ///< o fixed se espalha antes de cair
    constexpr float kRepaint = 0.6f;           ///< depois da regra: a transicao acabou
    /// O PROFESSOR PARA PARA DIGITAR cada regra no DevTools, e so depois volta
    /// a varrer. Varrendo sem parar, o tiro do aluno (quase 1 s de voo) chegava
    /// depois dele, e quem desviava terminava a fase so com o que ja tinha.
    constexpr float kDigitando = 0.8f;
    /// A CASCATA - o C do CSS: enquanto ele digita, cada letra da regra escorre
    /// reta pela coluna embaixo dele. Ficar embaixo do professor parado custa.
    constexpr const char* kGatilhoDaCascata = "escorrer";
    constexpr int kLetrasDaCascata = 5;
    constexpr float kEntreLetrasDaCascata = 0.1f;
    constexpr float kRapidezDosCartoes = 9.0f; ///< a transition (Ancora::Aproximar)

    /// @brief O x do centro da grade com o professor em 'xDoProfessor': o
    /// espelho dele na janela de 'largura'.
    float XDaGrade(float xDoProfessor, float largura);

    /// @brief Quanto o cartao do :hover desceu, 't' segundos depois da regra.
    float Hover(float t);

    /// @brief O que o Dificil muda: uma regra a cada 1,6 s (e nao 2,0).
    struct Degrau {
        float porRegra = kPorRegra;
        int letras = kLetrasDaCascata;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
