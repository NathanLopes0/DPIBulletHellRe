//
// O Ponto Extra do co-op: o feixe ate o parceiro e o escudo em volta dele.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"
#include "Nota.h"

/// Em dupla, o Ponto Extra e do outro (Battle::AoGastarPontoExtra).
namespace PontoExtra {

    /// O raio do escudo, do centro do aluno. Um pouco maior que o sprite: o projetil some antes de
    /// encostar, e e isso que se le como escudo.
    constexpr float kRaioDoEscudo = 56.0f;

    /// Quanto o feixe leva de quem gastou ate o parceiro, e quanto o rastro
    /// leva para se apagar depois de chegar.
    constexpr float kViagemDoFeixe = 0.35f;
    constexpr float kRastroDoFeixe = 0.4f;

    /// A fracao do caminho que a cabeca do feixe ocupa: um traco curto, e nao
    /// a linha inteira acendendo de uma vez.
    constexpr float kTamanhoDoFeixe = 0.25f;

    /// Se um projetil de raio 'raio' em 'projetil' encosta no escudo de quem esta em 'centro'.
    bool NoEscudo(Vector2 centro, Vector2 projetil, float raio);

    /// O anel do escudo como 'lados' segmentos: pares (de, ate).
    std::vector<std::pair<Vector2, Vector2>> Anel(Vector2 centro, float raio, int lados = 24);

    /// O que o feixe desenha 't' segundos depois do Ponto Extra.
    struct Feixe {
        bool cabeca = false;      ///< a cabeca ainda viaja
        Vector2 cabecaDe, cabecaAte;
        float rastro = 0.0f;      ///< de 1 (aceso) a 0 (apagado): a linha inteira
    };

    /// O feixe de 'de' ate 'ate' no instante 't'. Depois de kViagemDoFeixe + kRastroDoFeixe ele nao
    /// desenha mais nada.
    Feixe FeixeEm(Vector2 de, Vector2 ate, float t);

    /// Se o feixe ainda desenha alguma coisa no instante 't'.
    bool FeixeAceso(float t);

    // ---- O Ponto Extra de solo ----

    /// A cada quantos acertos no professor ele deixa cair um Ponto Extra: 80, com uma folga
    /// sorteada de 8 para cima ou para baixo.
    constexpr int kAcertosParaCair = 80;
    constexpr int kFolgaDaQueda = 8;

    /// Quanto o aluno fica sem atirar depois de um Ponto Extra de solo.
    constexpr float kSobrecarga = 1.5f;

    /// No co-op, quanto o parceiro fica imune e com o escudo - e quanto quem gastou espera para
    /// gastar outro.
    constexpr float kImunidadeDoPresente = 3.0f;

    /// O intervalo entre dois tiros do aluno (Player::Atirar).
    constexpr float kEntreTiros = 0.12f;

    /// Quantos tiros o aluno daria em 'sobrecarga' segundos.
    int TirosNaSobrecarga(float sobrecarga = kSobrecarga, float entreTiros = kEntreTiros);

    /// A nota depois de 'tiros' acertos de 'porAcerto' bruto cada, partindo de 'nota', com
    /// 'acertosSofridos' (o teto por dano vale).
    float NotaDepois(float nota, int acertosSofridos, float porAcerto, int tiros);

    /// A onda do Ponto Extra: a rapidez com que o raio cresce, e ate onde.
    constexpr float kRapidezDaOnda = 1500.0f;
    constexpr float kDuracaoDaOnda = 1.0f;
    [[nodiscard]] inline float RaioDaOnda(const float t) { return t * kRapidezDaOnda; }
}
