//
// A repescagem do Daniel: a explicabilidade dos modelos.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A repescagem do Daniel comeca com o sorteio (Chamada.h) e depois abre o modelo:
///   - na caixa-preta, o professor vira o modelo que ninguem entende: cada palpite (um anel, um
///     leque, uma espiral) sai sem aviso. Mas toda duvida que acerta o professor e uma pergunta - "por
///     que?" -, e o modelo explica: quanto mais perguntas nos ultimos segundos, mais cedo a linha de
///     cada palpite aparece antes dele;
///   - no SHAP, a explicacao de uma previsao vira uma cascata: do valor base, cada atributo empurra a
///     previsao para um lado - uma faixa de tiros por atributo, cada uma comecando onde a anterior
///     acabou -, e a cascata termina na previsao, f(x). Mas o modelo errou: o valor real (dourado)
///     esta em outro lugar, porque um ou dois atributos empurram para o lado errado. O tiro do aluno
///     atravessa as faixas; so a ponta de cada barra cai. Derrubar a ponta tira o atributo, e a
///     cascata se refaz sem ele: levar f(x) ate o valor real explica a previsao (vale nota, e as faixas
///     somem). Se o tempo acabar, a previsao estoura num anel e as faixas caem.
namespace Explicacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateFinal";
    constexpr std::array<const char*, 3> kEtapas = {"Sorteio", "CaixaPreta", "Shap"};
    constexpr std::array<float, 3> kDuracoes = {6.0f, 4.5f, 9.5f};
    constexpr const char* kGatilhoDoPalpite = "palpite";
    constexpr const char* kGatilhoDaBarra = "barra";
    constexpr const char* kGatilhoDaPrevisao = "previsao";
    constexpr const char* kGatilhoDaCaixa = "caixa";

    /// Em que etapa se esta no instante 't' da fase: 0, 1 ou 2.
    int EtapaNoTempo(float t);

    // ---- A caixa-preta ----

    enum class Palpite { Anel, Leque, Espiral };
    constexpr int kPalpites = 3;

    /// Quantos segundos antes do palpite a linha dele aparece, pelas perguntas dos ultimos
    /// kJanelaDasPerguntas segundos: quase nada sem pergunta, e ate kAntecedenciaMaxima.
    float Antecedencia(int perguntas, Dificuldade::Nivel n);
    constexpr float kJanelaDasPerguntas = 3.0f;
    constexpr float kAntecedenciaMaxima = 1.0f;

    /// Os rumos (em graus) dos 'tiros' de um palpite: o anel, em volta, girado de 'giro'; o leque,
    /// aberto em volta de 'mira'; a espiral, dois bracos de 'giro'.
    std::vector<float> Rumos(Palpite p, float mira, float giro, int tiros);

    // ---- O SHAP ----

    /// Os atributos do catalogo, na ordem das barras.
    constexpr std::array<const char*, 5> kAtributos = {"ataque", "defesa", "velocidade", "HP", "tipo"};

    struct Barra {
        float de = 0.0f;
        float ate = 0.0f;
        int atributo = 0;
    };
    /// A cascata: a barra de cada atributo comeca onde a anterior acabou, a partir de 'base'. O fim
    /// da ultima e a previsao.
    std::vector<Barra> Cascata(float base, const std::vector<float>& contribuicoes);

    /// As contribuicoes de uma previsao, uma por atributo: cada uma entre 'minimo' e 'maximo' em
    /// modulo, com o sinal sorteado - mas a soma de cada prefixo fica dentro de [-limite, limite]: a
    /// cascata nunca sai do campo.
    std::vector<float> Contribuicoes(Variacao::Sorteio& s, int quantas, float minimo, float maximo, float limite);

    /// As contribuicoes de uma previsao que acerta 'alvo': cada uma entre 'minimo' e 'maximo' em
    /// modulo, com o sinal sorteado, e a soma de todas e 'alvo' - a ultima fecha a conta.
    std::vector<float> ContribuicoesPara(Variacao::Sorteio& s, int quantas, float alvo, float minimo, float maximo);
    /// A previsao com os atributos tirados (o tiro do aluno derruba a barra): a base mais o que ficou.
    float Previsao(float base, const std::vector<float>& contribuicoes, const std::vector<bool>& tiradas);

    /// Uma previsao que errou, para o aluno explicar: das 'quantas' contribuicoes (entre 'minimo' e
    /// 'maximo' em modulo), as 'erradas' empurram todas para o mesmo lado, para longe do valor real.
    /// Sem elas, a cascata acaba em 'real'; com todas, a pelo menos 'minimo' dele. Cada degrau da
    /// cascata fica entre 'esquerda' e 'direita'.
    struct Explicada {
        std::vector<float> contribuicoes;
        std::vector<bool> erradas;
    };
    Explicada Explicar(Variacao::Sorteio& s, int quantas, int erradas, float base, float real, float minimo, float maximo,
                       float esquerda, float direita);
    /// Quanto f(x) pode ficar longe do valor real e contar como explicada (px).
    constexpr float kTolerancia = 24.0f;
    /// O que cada previsao explicada paga na nota (Battle::Conquistar).
    constexpr float kPremioDaExplicacao = 2.5f;

    /// O placar da cascata: "ataque +12 · defesa -8 · ...", na 'escala' (px por ponto).
    std::string Legenda(const std::vector<float>& contribuicoes, float escala);

    // ---- O ritmo ----

    struct Degrau {
        float entrePalpites = 0.75f;
        int tirosDoAnel = 24;
        int tirosDoLeque = 9;
        float velocidade = 210.0f;
        float entreBarras = 0.38f;       ///< uma barra da cascata a cada
        float avisoDaBarra = 0.5f;       ///< a linha de cada barra, antes dos tiros
        int tirosDaPrevisao = 12;
        float paraExplicar = 3.8f;       ///< o tempo, com a cascata inteira no campo, ate a previsao estourar
        float chanceDeDuasErradas = 0.3f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
