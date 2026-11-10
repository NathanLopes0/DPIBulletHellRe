//
// A conta da nota de uma batalha: quanto ela vale agora, os acertos que ela levou e as fases que
// passaram limpas.
//

#pragma once

#include <cstddef>

#include "Nota.h"

namespace Nota {

    /// A nota de uma batalha enquanto ela dura. As contas sao as de Nota.h; isto guarda o estado
    /// delas - a nota, os acertos sofridos (que baixam o teto) e os da fase em curso - e e por aqui
    /// que a batalha paga e cobra. Nao conhece batalha, SDL nem disco.
    class Conta {
    public:
        /// 'jogadores': quantos alunos dividem o que cada um causa (Nota::PorJogador).
        explicit Conta(float nota, std::size_t jogadores = 1);

        [[nodiscard]] float Valor() const { return mNota; }
        [[nodiscard]] int AcertosSofridos() const { return mAcertosSofridos; }
        /// Quantas fases pagaram o bonus de fase limpa.
        [[nodiscard]] int FasesLimpas() const { return mFasesLimpas; }

        /// Quanto vale, nesta batalha, algo que um jogador causou.
        [[nodiscard]] float PorJogador(float bruto) const;

        /// Soma um ganho bruto, inteiro: o que e de um jogador passa antes por PorJogador. O teto cai
        /// com os acertos sofridos (Nota::Somar).
        void Subir(float bruto);

        /// Cobra um acerto sofrido: a perda dividida e aliviada (Nota::PerdaPorJogador), vezes
        /// 'multiplicador'. Devolve quanto a nota caiu - perto do zero, menos que a perda.
        float Descer(float multiplicador = 1.0f);

        /// Um aluno gastou o Ponto Extra: a sobrecarga vale os acertos que ele daria nela
        /// (PontoExtra::NotaDepois), repartidos como os de um jogador.
        void GastarPontoExtra();

        /// A fase em curso acabou. Sem acerto sofrido nela, paga o bonus de fase limpa, inteiro, e
        /// devolve true. A primeira chamada e a entrada na primeira fase: nao fecha nada.
        bool FecharFase();

        /// A nota em 'nota', sem conta nenhuma. So a simulacao faz isto (--fim-nota).
        void Definir(const float nota) { mNota = nota; }

    private:
        float mNota;
        std::size_t mJogadores;
        int mAcertosSofridos = 0;
        int mAcertosNaFase = 0;
        bool mPrimeiraFase = true;
        int mFasesLimpas = 0;
    };
}
