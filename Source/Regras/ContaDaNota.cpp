//
// A conta da nota de uma batalha.
//

#include "ContaDaNota.h"

#include "PontoExtra.h"

namespace Nota {

    Conta::Conta(const float nota, const std::size_t jogadores) : mNota(nota), mJogadores(jogadores) {}

    float Conta::PorJogador(const float bruto) const {
        return Nota::PorJogador(bruto, mJogadores);
    }

    void Conta::Subir(const float bruto) {
        mNota = Somar(mNota, bruto, mAcertosSofridos);
    }

    float Conta::Descer(const float multiplicador) {
        // A contagem e no instante do acerto, e nao num balanco no fim da batalha.
        ++mAcertosSofridos;
        ++mAcertosNaFase;
        const float antes = mNota;
        mNota = Subtrair(mNota, PerdaPorJogador(kPerdaPorAcerto, mJogadores) * multiplicador);
        return antes - mNota;
    }

    void Conta::GastarPontoExtra() {
        mNota = PontoExtra::NotaDepois(mNota, mAcertosSofridos, PorJogador(kGanhoPorAcerto),
                                       PontoExtra::TirosNaSobrecarga());
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
