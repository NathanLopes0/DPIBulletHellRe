//
// A fase 2 do Lucas: o DevTools aberto, e o CSS mudando ao vivo.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Flex.h"

/// O Lucas abre o devtools e edita o CSS ao vivo.
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

    /// Nunca o mesmo tipo duas vezes seguidas; a grade girada ou grande tende a voltar (rotate(0),
    /// scale(1)), para nao ficar torta a fase inteira.
    Regra Proxima(Variacao::Sorteio& s, Css& css, Tipo anterior);

    /// Esta regra repinta? Todas, menos o :hover e o position: fixed.
    bool Repinta(Tipo t);

    /// O placar: o seletor e, embaixo, a declaracao.
    std::string Placar(const Regra& r);

    struct Ponto {
        float x = 0.0f;
        float y = 0.0f;
    };

    /// O cartao da linha oculta tambem tem lugar - quem o esconde e quem chama, com Oculto.
    std::array<Ponto, kCartoes> Posicoes(const Css& css, Ponto centro, float giro = 0.0f);

    /// O cartao 'i' esta na linha com display: none?
    bool Oculto(const Css& css, int i);

    /// A queda do position: fixed: o lugar do cartao 'i' espalhado de borda a borda numa fileira
    /// so, na altura 'y'.
    Ponto Espalhado(int i, float largura, float y);

    /// O relogio: uma regra a cada kPorRegra; a animation gira kGiroPorSegundo por kDuracaoDoGiro;
    /// o :hover desce kDescidaDoHover em kDuracaoDoHover.
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
    /// O professor para para digitar cada regra no DevTools, e so depois volta a varrer.
    constexpr float kDigitando = 0.8f;
    /// A cascata - o C do CSS: enquanto ele digita, cada letra da regra escorre reta pela coluna
    /// embaixo dele. Ficar embaixo do professor parado custa.
    constexpr const char* kGatilhoDaCascata = "escorrer";
    constexpr int kLetrasDaCascata = 5;
    constexpr float kEntreLetrasDaCascata = 0.1f;
    constexpr float kRapidezDosCartoes = 9.0f; ///< a transition (Ancora::Aproximar)

    /// O x do centro da grade com o professor em 'xDoProfessor': o espelho dele na janela de
    /// 'largura'.
    float XDaGrade(float xDoProfessor, float largura);

    /// Quanto o cartao do :hover desceu, 't' segundos depois da regra.
    float Hover(float t);

    /// O que o Dificil muda: uma regra a cada 1,6 s (e nao 2,0).
    struct Degrau {
        float porRegra = kPorRegra;
        int letras = kLetrasDaCascata;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
