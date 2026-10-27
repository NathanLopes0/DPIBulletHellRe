//
// A fase 1 do Lucas: um documento HTML sendo escrito, tag por tag.
//

#pragma once

#include <string>
#include <vector>

#include "../Dificuldade.h"
#include "../Variacao.h"
#include "Fase.h"

/**
 * CAMADA PURA. Coberto por test_dom.cpp.
 *
 * O LUCAS ESCREVE UM DOCUMENTO HTML NA FRENTE DO ALUNO, e o documento e
 * gerado a cada vez - um <main> com uma lista, um paragrafo com link, uma
 * imagem... A cada kPorEvento sai a proxima tag, e o placar a escreve com o
 * caminho dela no DOM ("main > ul > li"):
 *
 *   <ul>     ABRE o elemento: uma caixa de tags nasce no lugar dele e cresce
 *            (CaixaAttack) - os filhos nascem DENTRO do pai, menores, e o que
 *            fica na tela sao caixas aninhadas, o box model desenhado.
 *   </ul>    FECHA: a caixa daquele elemento volta - as tags passam pelo
 *            centro e saem do outro lado, mais depressa. A caixa vira do
 *            avesso.
 *   <img>    um elemento VAZIO abre e fecha de uma vez: um anel.
 *
 * O professor vai ate o lugar de cada elemento para abri-lo. Lendo o placar o
 * aluno sabe qual caixa vai virar do avesso: a do ultimo elemento aberto.
 */
namespace Dom {

    constexpr const char* kFaseDoDom = "StateOne";
    constexpr const char* kGatilhoDaAbertura = "abrir";   ///< a caixa de um elemento
    constexpr const char* kGatilhoDoVazio = "vazio";      ///< o anel de <img>, <input>

    struct Ponto {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Elemento {
        std::string tag;
        int pai = -1;
        int profundidade = 0;
        bool vazio = false;
        std::vector<int> filhos;
        Ponto centro;
    };

    /// O documento: elementos[0] e a raiz.
    struct Documento {
        std::vector<Elemento> elementos;
    };

    /// O lugar de cada elemento: a raiz no alto, num x sorteado a cada
    /// documento ate kDesvioDaRaiz do meio, e os filhos lado a lado embaixo do
    /// pai, cada geracao mais junta - a indentacao. O professor fica em cima
    /// da raiz: correndo de elemento em elemento, a 520 px/s, o tiro do aluno
    /// nao o achava, e quem desviava terminava a fase so com 59,5.
    constexpr float kYDaRaiz = 210.0f;
    constexpr float kDesvioDaRaiz = 220.0f;

    constexpr float kEntreGeracoes = 70.0f;
    constexpr float kEntreIrmaos = 340.0f;   ///< na primeira geracao; metade a cada uma
    constexpr float kMargem = 150.0f;

    /// @brief Um documento novo: uma raiz (div, main, section ou article) com
    /// dois ou tres filhos, cada um um molde de HTML de verdade - ul com li,
    /// p com a ou strong, div com p e img, h1, img. No maximo kMaisElementos.
    constexpr int kMaisElementos = 10;
    Documento Gerar(Variacao::Sorteio& s, float largura);

    enum class Tipo { Abre, Fecha, Vazio };
    struct Evento {
        Tipo tipo = Tipo::Abre;
        int elemento = 0;
    };

    /// @brief A ordem em que as tags saem: a de quem escreve o arquivo - abre
    /// o pai, escreve os filhos, fecha o pai.
    std::vector<Evento> Eventos(const Documento& d);

    /// @brief "<ul>", "</ul>" ou "<img>".
    std::string Tag(const Documento& d, const Evento& e);

    /// @brief O caminho do elemento no DOM, da raiz ate ele: "main > ul > li".
    std::string Caminho(const Documento& d, int elemento);

    /// @brief O placar: a tag, e embaixo o caminho.
    std::string Placar(const Documento& d, const Evento& e);

    /// @brief O tamanho em que a caixa do elemento nasce: a raiz com o do
    /// arquivo, e cada geracao 30% menor - o filho nasce dentro do pai.
    float Encolher(int profundidade);

    /// O RELOGIO: o primeiro documento logo depois de abrir, uma tag a cada
    /// kPorEvento, e uma pausa entre um documento e o seguinte.
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorEvento = 0.4f;
    constexpr float kEntreDocumentos = 0.9f;

    /// FECHAR: o quanto a caixa volta mais depressa do que abriu.
    constexpr float kVolta = 1.3f;

    /// @brief O que o Dificil muda: uma tag a cada 0,32 s (e nao 0,4).
    struct Degrau {
        float porEvento = kPorEvento;
    };


    Degrau DegrauDo(Dificuldade::Nivel n);
}
