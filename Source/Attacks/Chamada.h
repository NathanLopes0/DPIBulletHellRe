//
// O sorteio do Daniel: no comeco da aula, um aluno explica a aula passada.
//

#pragma once

#include <array>
#include <string>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// O professor de Ciencia de Dados sorteia, no comeco da aula, quem vai explicar a aula anterior. No
/// comeco das fases 2 e 3 e da repescagem, uma roleta passa pelos nomes da turma e para em alguem:
///   - num colega, ele explica (o placar mostra a resposta), e a aula segue;
///   - em voce, "eu escolho voce!": a pergunta de revisao aparece no placar, e as tres respostas
///     descem em cartas. Derrubar a certa paga kPremio na nota; a errada estoura em cacos, e o tempo
///     que acaba estoura as tres.
namespace Chamada {

    constexpr const char* kGatilhoDaResposta = "resposta";
    constexpr const char* kGatilhoDoEngano = "engano";

    /// Uma pergunta de revisao. A opcao 0 e a certa: a ordem das cartas na tela e sorteada.
    struct Pergunta {
        const char* enunciado = "";
        std::array<const char*, 3> opcoes{};
    };
    constexpr int kAulas = 3;
    constexpr int kPerguntasPorAula = 3;
    constexpr int kOpcoes = 3;

    /// As perguntas da aula 'aula' (0: a preparacao, revista no comeco da fase 2; 1: os grupos, na
    /// fase 3; 2: o supervisionado, na repescagem).
    const std::array<Pergunta, kPerguntasPorAula>& Revisao(int aula);

    /// A cara da carta da opcao 'o' da pergunta 'q' da aula 'aula': "R0Q1O2".
    std::string Cara(int aula, int q, int o);

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
