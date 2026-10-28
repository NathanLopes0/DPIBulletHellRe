//
// O escudo do Hugo e o objetivo de cada etapa do TCC.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Dificuldade.h"
#include "../Golpe.h"
#include "../Math.h"
#include "../Variacao.h"

/**
 * CAMADA PURA. Coberto por test_objetivo.cpp.
 *
 * O TCC AINDA NAO ESTA PRONTO, e enquanto nao estiver o Hugo e blindado: o
 * tiro que o acerta so carrega o poder do orientador, e nao paga nota. Em cada
 * etapa o aluno cumpre um OBJETIVO - derrubar os alvos que o Hugo poe em campo
 * - e abre uma JANELA de kJanela segundos em que o Hugo toma dano. Acabada a
 * janela, o escudo volta e vem outro objetivo.
 *
 *   a proposta        as ASSINATURAS (do orientador, da coordenacao, da banca)
 *   o desenvolvimento os BUGS dos experimentos, que andam e aguentam dois tiros
 *   a defesa          a RESPOSTA CERTA a pergunta da banca: o placar escreve a
 *                     pergunta, e tres baloes - A, B e C, cada um com a
 *                     resposta escrita embaixo - voam no campo. A errada
 *                     custa: quem perguntou ataca.
 *   as correcoes      as CORRECOES em vermelho na versao final
 *
 * Os alvos sao projeteis do Hugo, DERRUBAVEIS, no posto "objetivo" - os poderes
 * do orientador nao os apagam (Orientador::TirosSoltos), e o Hugo os move.
 */
namespace Objetivo {

    enum class Etapa { Nenhuma, Proposta, Desenvolvimento, Defesa, Correcoes };

    /// "StateOne" e a proposta, e assim por diante.
    Etapa DaFase(const std::string& fase);

    constexpr const char* kPostoDoObjetivo = "objetivo";
    constexpr const char* kGatilhoDoAlvo = "alvo";     ///< assinatura, resposta, correcao
    constexpr const char* kGatilhoDoBug = "bug";       ///< o bug, que aguenta dois tiros

    /// A faixa do campo onde os alvos ficam: abaixo do cronograma, acima do aluno.
    constexpr float kYDeCima = 380.0f;
    constexpr float kYDeBaixo = 540.0f;
    constexpr float kMargem = 120.0f;

    /// Um alvo do objetivo: a animacao, onde ele fica, e se derruba-lo e certo.
    struct Alvo {
        std::string animacao;
        Vector2 lugar;
        bool certo = true;
    };

    /// @brief Os alvos de um objetivo novo da 'etapa', numa janela de
    /// 'largura'. Na defesa, 'certa' (0 a 2) e a resposta certa.
    std::vector<Alvo> Gerar(Etapa etapa, Variacao::Sorteio& s, float largura, Dificuldade::Nivel n, int certa = 0);

    /// @brief Para onde o alvo 'k' vai agora, 't' segundos depois de nascer em
    /// 'lugar': as assinaturas e as respostas balancam, as correcoes boiam. O
    /// bug anda por conta propria (Paragem).
    Vector2 Balanco(Etapa etapa, const Vector2& lugar, int k, float t);

    /// @brief A proxima parada de um bug: um ponto qualquer da faixa.
    Vector2 Paragem(Variacao::Sorteio& s, float largura);
    constexpr float kRapidezDoBug = 130.0f;

    // ---- A DEFESA ------------------------------------------------------------------

    /// Uma pergunta da banca, com as tres respostas na ordem do placar.
    struct Questao {
        std::string pergunta;
        std::array<std::string, 3> respostas;
        int certa = 0;
    };

    /// @brief A pergunta do professor 'm', com as respostas embaralhadas.
    Questao Perguntar(Golpe::Materia m, Variacao::Sorteio& s);

    // ---- A JANELA ------------------------------------------------------------------

    /// O QUE O OBJETIVO PAGA, em nota bruta (Battle::Conquistar): cada alvo
    /// derrubado, e o escudo aberto. Sem isso, blindado, o Hugo so tirava nota
    /// - e o aluno que tomava uns acertos antes da primeira janela zerava.
    /// No Dificil, um pouco menos.
    float PorAlvo(Dificuldade::Nivel n);
    float PorJanela(Dificuldade::Nivel n);

    /// ABERTO, A MONOGRAFIA SE DESFAZ: as paginas escorrem retas embaixo do
    /// Hugo, uma a cada tanto. A janela e a hora de acertar, e ficar parado
    /// embaixo dele, atirando, custa.
    float EntrePaginasSoltas(Dificuldade::Nivel n);

    /// @brief Quantos segundos o Hugo fica aberto depois de um objetivo.
    float Janela(Etapa etapa, Dificuldade::Nivel n);
    constexpr float kRecomeco = 0.8f;    ///< da janela fechada ao objetivo seguinte

    /// @brief O que o placar diz do objetivo, faltando 'faltam' alvos.
    std::string Falta(Etapa etapa, int faltam);
}
