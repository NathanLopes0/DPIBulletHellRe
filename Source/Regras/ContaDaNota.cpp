//
// A conta da nota de uma batalha.
//

#include "ContaDaNota.h"

#include <algorithm>

#include "PontoExtra.h"

namespace Nota {

    Conta::Conta(const float nota, const std::size_t jogadores) : mNota(nota), mJogadores(jogadores) {}

    float Conta::PorJogador(const float bruto) const {
        return Nota::PorJogador(bruto, mJogadores);
    }

    void Conta::Subir(const float bruto) {
        // A extra so existe acima da nota cheia: com o teto de quem levou dano, nao ha.
        if (bruto > 0.0f && TetoCom(mAcertosSofridos) >= kNotaMaxima) {
            // Ja na cheia, o ganho bruto inteiro; chegando nela, so o que passa do 100.
            if (ECheia(mNota)) mExtra += bruto;
            else mExtra += std::max(0.0f, mNota + Ganho(mNota, bruto) - kNotaMaxima);
        }
        mNota = Somar(mNota, bruto, mAcertosSofridos);
    }

    float Conta::Descer(const float multiplicador) {
        // A contagem e no instante do acerto, e nao num balanco no fim da batalha.
        ++mAcertosSofridos;
        ++mAcertosNaFase;
        const float antes = mNota;
        const float perda = PerdaPorJogador(kPerdaPorAcerto, mJogadores) * multiplicador * mFatorDaPerda;
        mNota = Subtrair(mNota, perda);
        mExtra = std::max(0.0f, mExtra - perda);
        return antes - mNota;
    }

    void Conta::GastarPontoExtra() {
        mNota = PontoExtra::NotaDepois(mNota, mAcertosSofridos, PorJogador(kGanhoPorAcerto),
                                       PontoExtra::TirosNaSobrecarga());
    }

    bool Conta::Desfazer(const Marca& marca) {
        if (mNota >= marca.nota) return false;
        const int desfeitos = std::max(0, mAcertosSofridos - marca.acertosSofridos);
        mNota = marca.nota;
        mExtra = std::max(mExtra, marca.extra);
        mAcertosSofridos -= desfeitos;
        mAcertosNaFase = std::max(0, mAcertosNaFase - desfeitos);
        return true;
    }

    bool Conta::FecharFase() {
        if (mPrimeiraFase) {
            mPrimeiraFase = false;
            mAcertosNaFase = 0;
            return false;
        }
        const bool limpa = mAcertosNaFase == 0;
        if (limpa) {
            // Da fase, e nao de um jogador: pago inteiro.
            Subir(kBonusDeFaseLimpa);
            ++mFasesLimpas;
        }
        mAcertosNaFase = 0;
        return limpa;
    }
}
