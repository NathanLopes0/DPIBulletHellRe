//
// A fase 1 do Lucas: uma pagina sendo renderizada no navegador.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Gameplay/Dificuldade.h"
#include "../Gameplay/Variacao.h"

/**
 * CAMADA PURA. Coberto por test_pagina.cpp.
 *
 * O CAMPO E A JANELA DO NAVEGADOR, e o Lucas renderiza uma pagina nela. O
 * layout fica desenhado a fase inteira - o header no alto, onde ele esta, e
 * embaixo o nav, o main e o aside lado a lado - e cada pagina, gerada a cada
 * vez, poe os elementos dela nas regioes, UM EMBAIXO DO OUTRO, no fluxo do
 * documento, como o navegador faz. Um elemento e renderizado a cada
 * kPorElemento, e o placar diz qual - "<img src="gato.png">" e embaixo
 * "body > main".
 *
 * CADA ELEMENTO ATACA DO SEU JEITO, e o tipo dele e o aviso:
 *
 *   <button>    o cursor voa do professor ate o botao e CLICA: sai um leque.
 *               O cursor e derrubavel - derrubado, o clique nao acontece.
 *   <img>       o spinner gira enquanto a imagem carrega; carregada, ela
 *               estoura num anel.
 *   <marquee>   uma linha de tags atravessa a tela de lado, descendo.
 *   <table>     as linhas da tabela acendem uma a uma, e cada uma cai.
 *   <a href>    o professor NAVEGA ate a ancora - a outra regiao - e chega
 *               num anel.
 *   <input>     o texto e digitado letra por letra, e cada letra sai em voce.
 *
 * Renderizada a pagina inteira, o F5: o layout fica, os elementos somem, e
 * vem outra.
 */
namespace Pagina {

    constexpr const char* kFaseDaPagina = "StateOne";
    constexpr const char* kPostoDaPagina = "pagina";   ///< o cursor e o spinner, que o Lucas move

    constexpr const char* kGatilhoDoCursor = "cursor";
    constexpr const char* kGatilhoDoClique = "clique";
    constexpr const char* kGatilhoDoSpinner = "spinner";
    constexpr const char* kGatilhoDaCarga = "carregar";
    constexpr const char* kGatilhoDoMarquee = "marquee";
    constexpr const char* kGatilhoDaCelula = "celula";
    constexpr const char* kGatilhoDaNavegacao = "navegar";
    constexpr const char* kGatilhoDaDigitacao = "digitar";

    struct Retangulo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        [[nodiscard]] float Cx() const { return x + w / 2.0f; }
        [[nodiscard]] float Cy() const { return y + h / 2.0f; }
    };

    // ---- O LAYOUT --------------------------------------------------------------

    enum class Regiao { Header, Nav, Main, Aside };
    constexpr int kRegioes = 4;

    /// O layout numa janela de 'largura': o header de borda a borda no alto, e
    /// embaixo dele o nav, o main e o aside - o main com o dobro da largura.
    constexpr float kMargem = 40.0f;
    constexpr float kYDoHeader = 60.0f;
    constexpr float kAlturaDoHeader = 90.0f;
    constexpr float kYDoCorpo = 170.0f;
    constexpr float kAlturaDoCorpo = 330.0f;
    constexpr float kEntreRegioes = 16.0f;
    std::array<Retangulo, kRegioes> Layout(float largura);

    std::string NomeDaRegiao(Regiao r);

    // ---- OS ELEMENTOS ----------------------------------------------------------

    enum class Tipo { Botao, Imagem, Marquee, Tabela, Link, Input };

    struct Elemento {
        Tipo tipo = Tipo::Botao;
        Regiao regiao = Regiao::Main;
        Retangulo caixa;
        Regiao ancora = Regiao::Main;   ///< so do <a href>: para onde ele navega
        std::string atributo;           ///< o que o placar escreve dentro da tag
    };

    /// O tamanho de cada tipo, e o espaco entre dois elementos no fluxo.
    Retangulo Tamanho(Tipo t, const Retangulo& regiao);
    constexpr float kEntreElementos = 14.0f;

    /// @brief Uma pagina nova: de 6 a 8 elementos, em ordem de documento - o
    /// nav, depois o main, depois o aside -, cada um embaixo do anterior da
    /// mesma regiao. O que nao cabe na regiao fica de fora. Toda pagina tem
    /// pelo menos tres tipos diferentes.
    std::vector<Elemento> Gerar(Variacao::Sorteio& s, float largura);

    /// @brief "<img src=\"gato.png\">" e embaixo "body > main".
    std::string Placar(const Elemento& e);

    /// @brief Os centros das celulas da tabela, linha por linha.
    constexpr int kLinhasDaTabela = 3;
    constexpr int kColunasDaTabela = 4;
    std::vector<std::vector<std::array<float, 2>>> Celulas(const Retangulo& tabela);

    // ---- O RELOGIO -----------------------------------------------------------------

    constexpr float kPrimeiro = 0.6f;
    constexpr float kPorElemento = 1.2f;
    constexpr float kRecarga = 1.2f;          ///< o F5 entre uma pagina e a seguinte

    constexpr float kVooDoCursor = 0.6f;      ///< do professor ao botao; e o clique
    constexpr float kCarregando = 0.9f;       ///< o spinner, antes do anel
    constexpr int kPontosDoSpinner = 6;
    constexpr float kRaioDoSpinner = 22.0f;
    constexpr float kVoltasDoSpinner = 1.5f;  ///< por segundo
    constexpr int kTagsDoMarquee = 7;
    constexpr float kEntreTagsDoMarquee = 46.0f;
    constexpr float kDescidaDoMarquee = 55.0f;   ///< px/s, enquanto atravessa
    constexpr float kEntreLinhas = 0.3f;      ///< a tabela acende uma linha a cada
    constexpr int kLetras = 6;
    constexpr float kEntreLetras = 0.11f;

    /// @brief O que o Dificil muda: um elemento a cada 0,95 s (e nao 1,2), e o
    /// spinner carrega em 0,7 s.
    struct Degrau {
        float porElemento = kPorElemento;
        float carregando = kCarregando;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
