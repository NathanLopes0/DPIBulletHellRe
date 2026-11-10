//
// A devolutiva da prova: onde a nota caiu e o que mais acertou o aluno - e, na dupla, a parte de
// cada um.
//

#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

/// Quem reprova merece saber por que (Resultado.h): a fase em que a nota mais caiu e o que mais o
/// acertou. Na dupla, o que cada um fez, para o parceiro que cobriu ver que contou. A batalha anota
/// num Caderno enquanto a luta acontece, e isto resume. Nao conhece batalha, SDL nem disco.
namespace Devolutiva {

    /// A causa de um acerto sofrido que nao foi um projetil: o aluno encostou no professor.
    inline const std::string kEsbarrao = "esbarrao";

    /// O que custou nota numa fase.
    struct Fase {
        int ordem = 0;            ///< 1 para a primeira fase jogada
        std::string anuncio;      ///< o nome que o placar deu a ela ("h(k) = k mod m")
        bool final = false;       ///< a prova final (a StateFinal), e nao uma fase numerada
        float perdido = 0.0f;     ///< a nota que os acertos sofridos levaram nela
        int acertos = 0;          ///< quantos acertos sofridos
    };

    /// O que acertou o aluno: um tipo de projetil (o nome em projeteis.json), ou o esbarrao.
    struct Causa {
        std::string causa;
        float perdido = 0.0f;
        int acertos = 0;
    };

    class Caderno {
    public:
        /// Comecou a fase 'anuncio'; 'final' quando e a prova final.
        void ComecarFase(const std::string& anuncio, bool final = false);

        /// Um acerto sofrido levou 'perda' de nota, por 'causa'.
        void Perder(const std::string& causa, float perda);

        /// A fase em que mais nota caiu (a primeira, no empate); nada se nenhuma caiu.
        [[nodiscard]] std::optional<Fase> PiorFase() const;

        /// As causas, da que mais custou para a que menos (a que acertou mais vezes, no empate).
        [[nodiscard]] std::vector<Causa> Causas() const;

        [[nodiscard]] const std::vector<Fase>& Fases() const { return mFases; }

    private:
        std::vector<Fase> mFases;
        std::map<std::string, Causa> mCausas;
    };

    /// "Onde a nota mais caiu: fase 2, h(k) = k mod m (-18.00)" - ou "prova final (-20.00)". A perda
    /// vai com duas casas e ponto, como a barra da nota. O nome da fase so entra se a linha couber em
    /// kMaximoDaLinha letras: o painel encolheria a linha inteira.
    std::string LinhaDaFase(const Fase& f);

    /// Quantas letras cabem numa linha do resultado sem a letra encolher.
    constexpr std::size_t kMaximoDaLinha = 58;

    /// Quantas causas a devolutiva mostra, no maximo: as que mais custaram.
    constexpr int kCausasNaTela = 3;

    // ---- A justica, na dupla ----

    /// O que um aluno fez na prova da dupla.
    struct Parte {
        std::string quem;         ///< a matricula, ou "Visitante"
        int acertos = 0;          ///< acertos no professor
        int coberturas = 0;       ///< projeteis que iam no parceiro e ele derrubou (Cobertura.h)
        int sofridos = 0;         ///< acertos que ele sofreu
    };

    /// "89384: 120 acertos, 8 coberturas, 3 sofridos" - o singular e o plural certos.
    std::string LinhaDaParte(const Parte& p);

    /// A mesma, sem o nome: "120 acertos, 8 coberturas, 3 sofridos". Vai embaixo do titulo.
    std::string NumerosDaParte(const Parte& p);

    /// O titulo de cada aluno da dupla, pelo que mais fez - e nunca uma posicao (Emmerich e Masuch: os
    /// numeros lado a lado viram placar entre parceiros). Tres papeis, nesta ordem: "quem mais cobriu"
    /// (coberturas), "quem mais perguntou" (acertos) e "quem mais desviou" (menos sofridos). Um papel
    /// tem dono quando um aluno e melhor nele por uma margem (kMargemDoPapel): 39 a 38 e empate. Cada
    /// aluno fica com o primeiro de que e dono, e os dois titulos nunca se repetem. Quem nao e dono de
    /// nenhum ganha o que fez, sem comparacao ("fez 12 perguntas"). Vazio fora da dupla.
    std::vector<std::string> Titulos(const std::vector<Parte>& partes);

    /// Quanto um aluno precisa estar a frente, numa fracao do maior dos dois, para o papel ser dele.
    constexpr float kMargemDoPapel = 0.1f;
}
