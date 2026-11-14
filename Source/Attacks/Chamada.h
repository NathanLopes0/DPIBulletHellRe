//
// O sorteio do Daniel: no comeco da aula, um aluno explica a aula passada.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O professor de Ciencia de Dados sorteia, no comeco da aula, quem vai explicar a aula anterior. No
/// comeco das fases 2 e 3 e da repescagem, uma roleta passa pelos nomes da turma e para em alguem:
///   - num colega, ele explica (o placar mostra a resposta), e a aula segue;
///   - em voce, "eu escolho voce!": a pergunta aparece no placar e o desenho dela no campo, e as tres
///     cartas ficam no desenho, presas as opcoes (Alfinete). Derrubar a certa paga kPremio na nota; a
///     errada estoura em cacos, e o tempo que acaba estoura as tres.
/// A pergunta nao e de decorar: e de olhar o desenho - o ponto que nao combina, quantos grupos ha, a
/// reta que separa as cores. Quem conhece a materia reconhece o padrao; quem nao conhece, ve.
namespace Chamada {

    constexpr const char* kGatilhoDaResposta = "resposta";
    constexpr const char* kGatilhoDoEngano = "engano";

    constexpr int kAulas = 3;
    constexpr int kOpcoes = 3;
    /// A distancia minima, na horizontal e no quadro de 0 a 1, entre dois candidatos da mesma mancha.
    constexpr float kSeparacao = 0.13f;

    /// As caras das cartas: as letras dos candidatos e os numeros da contagem.
    constexpr std::array<const char*, 6> kCartas = {"CartaA", "CartaB", "CartaC", "Carta2", "Carta3", "Carta4"};

    /// Os jeitos de perguntar, dois por aula: a preparacao (o ponto que nao combina, a cor errada), os
    /// grupos (quantos sao, qual e o centroide) e o supervisionado (a reta que separa, o vizinho).
    enum class Desafio { Outlier, CorErrada, Grupos, Centroide, Reta, Vizinho };

    /// Um ponto do desenho, num quadro de 0 a 1 (y para baixo): a cor e a especie (-1: cinza).
    struct Marca {
        Vector2 onde;
        int cor = -1;
    };
    struct Questao {
        Desafio tipo = Desafio::Outlier;
        std::string enunciado;
        std::vector<Marca> pontos;
        std::vector<Vector2> losangos;               ///< o Centroide: os candidatos
        std::vector<std::array<Vector2, 2>> retas;   ///< a Reta: as candidatas
        std::array<Vector2, kOpcoes> cartas{};       ///< onde cada carta fica, no quadro
        std::array<std::string, kOpcoes> caras{};
        int certa = 0;
        std::string resposta;                        ///< o que o placar diz depois
    };
    /// Uma questao da aula 'aula' (0: a preparacao, revista no comeco da fase 2; 1: os grupos, na
    /// fase 3; 2: o supervisionado, na repescagem): o jeito e o desenho sao sorteados.
    Questao Gerar(int aula, Variacao::Sorteio& s);
    /// Se as cartas de 'tipo' marcam um ponto (ou losango) do desenho: elas ficam logo acima dele,
    /// presas por um alfinete, e o ponto ganha um anel - a carta nao esconde a cor.
    bool Alfinete(Desafio tipo);

    /// Quem explica: -1 e um colega; 0 ou 1, o aluno dessa posicao. Sozinho, metade das vezes e
    /// voce; em dupla, um terco cada e um terco um colega.
    int Sortear(Variacao::Sorteio& s, int alunos);
    /// O nome do colega 'k' na roleta: "Aluno 7", "Aluna 12"...
    std::string Colega(int k);
    constexpr int kColegas = 6;
    /// O tempo da roleta, e o de responder depois dela.
    constexpr float kRoleta = 1.5f;
    constexpr float kParaResponder = 4.2f;
    /// O que a resposta certa paga na nota (Battle::Conquistar).
    constexpr float kPremio = 6.0f;
    /// O que a roleta mostra em 't' segundos de giro: um nome da turma por vez, cada vez mais devagar.
    /// 'nomes' e quantos nomes ela tem para mostrar.
    int NomeNaRoleta(float t, int nomes);
}
