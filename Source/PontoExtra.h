//
// O Ponto Extra do co-op: o feixe ate o parceiro e o escudo em volta dele.
//

#pragma once

#include <vector>

#include "Math.h"
#include "Nota.h"

/**
 * CAMADA PURA. Coberto por test_ponto_extra.cpp.
 *
 * EM DUPLA, O PONTO EXTRA E DO OUTRO (Battle::OnPlayerUsedExtraPoint): quem
 * gasta continua atirando, e o parceiro ganha um ESCUDO que destroi os
 * projeteis do chefe que encostam nele, pelo tempo da imunidade. Antes o
 * presente era so a imunidade, e a unica pista dela era o aluno piscar azul -
 * no meio de uma tela cheia, ninguem via quem tinha protegido quem.
 *
 * Por isso o efeito agora se ve: um FEIXE sai de quem gastou e corre ate o
 * parceiro, e o anel do escudo fica em volta dele enquanto dura. Aqui moram a
 * geometria e os relogios; quem desenha e quem destroi e a Battle.
 */
namespace PontoExtra {

    /// O raio do escudo, do centro do aluno. Um pouco maior que o sprite: o
    /// projetil some ANTES de encostar, e e isso que se le como escudo.
    constexpr float kRaioDoEscudo = 56.0f;

    /// Quanto o feixe leva de quem gastou ate o parceiro, e quanto o rastro
    /// leva para se apagar depois de chegar.
    constexpr float kViagemDoFeixe = 0.35f;
    constexpr float kRastroDoFeixe = 0.4f;

    /// A fracao do caminho que a cabeca do feixe ocupa: um traco curto, e nao
    /// a linha inteira acendendo de uma vez.
    constexpr float kTamanhoDoFeixe = 0.25f;

    /// @brief Se um projetil de raio 'raio' em 'projetil' encosta no escudo
    /// de quem esta em 'centro'.
    bool NoEscudo(Vector2 centro, Vector2 projetil, float raio);

    /// @brief O anel do escudo como 'lados' segmentos: pares (de, ate).
    std::vector<std::pair<Vector2, Vector2>> Anel(Vector2 centro, float raio, int lados = 24);

    /// O que o feixe desenha 't' segundos depois do Ponto Extra.
    struct Feixe {
        bool cabeca = false;      ///< a cabeca ainda viaja
        Vector2 cabecaDe, cabecaAte;
        float rastro = 0.0f;      ///< de 1 (aceso) a 0 (apagado): a linha inteira
    };

    /// @brief O feixe de 'de' ate 'ate' no instante 't'. Depois de
    /// kViagemDoFeixe + kRastroDoFeixe ele nao desenha mais nada.
    Feixe FeixeEm(Vector2 de, Vector2 ate, float t);

    /// @brief Se o feixe ainda desenha alguma coisa no instante 't'.
    bool FeixeAceso(float t);

    // ---- O PONTO EXTRA DE SOLO -----------------------------------------------------
    //
    // SOZINHO, O ALUNO TROCA TIRO POR NOTA: gastar um Ponto Extra o deixa
    // kSobrecarga segundos sem atirar e, em troca, paga a nota que ele teria
    // ganhado ACERTANDO TODOS os tiros desse tempo - pela curva da nota de
    // agora (Nota::Somar), entao vale menos acima de 60, como o tiro vale. E
    // a ONDA (kRapidezDaOnda) sai dele e desfaz os tiros do professor a
    // medida que passa por eles.

    /// Quanto o aluno fica sem atirar depois de um Ponto Extra de solo. Era 3 s.
    constexpr float kSobrecarga = 1.5f;

    /// No co-op, quanto o parceiro fica imune e com o escudo - e quanto quem
    /// gastou espera para gastar outro. Nao mudou com a sobrecarga do solo.
    constexpr float kImunidadeDoPresente = 3.0f;

    /// O intervalo entre dois tiros do aluno (Player::Shoot).
    constexpr float kEntreTiros = 0.12f;

    /// @brief Quantos tiros o aluno daria em 'sobrecarga' segundos.
    int TirosNaSobrecarga(float sobrecarga = kSobrecarga, float entreTiros = kEntreTiros);

    /// @brief A nota depois de 'tiros' acertos de 'porAcerto' bruto cada,
    /// partindo de 'nota', com 'acertosSofridos' (o teto por dano vale).
    float NotaDepois(float nota, int acertosSofridos, float porAcerto, int tiros);

    /// A onda do Ponto Extra: a rapidez com que o raio cresce, e ate onde.
    constexpr float kRapidezDaOnda = 1500.0f;
    constexpr float kDuracaoDaOnda = 1.0f;
    [[nodiscard]] inline float RaioDaOnda(const float t) { return t * kRapidezDaOnda; }
}
