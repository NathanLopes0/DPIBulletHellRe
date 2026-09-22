//
// Resolucao do REFERENCIAL de um caminho: origem e orientacao.
//

#include "PathAim.h"

namespace {

    /// Converte uma direcao em cos/sin. Devolve false se o vetor for nulo.
    bool DirecaoParaRotacao(const Vector2& direcao, float& cos, float& sin) {
        const float comprimento = direcao.Length();
        if (Math::NearZero(comprimento)) {
            return false;
        }
        cos = direcao.x / comprimento;
        sin = direcao.y / comprimento;
        return true;
    }

}

Referencial ResolverReferencial(const ContextoDeResolucao& contexto, const Mira& mira) {

    Referencial referencial;
    referencial.origem = contexto.posicaoProjetil;

    // Angulo absoluto nao depende de nada do mundo: resolve e sai.
    if (mira.modo == Mira::AnguloFixo) {
        referencial.cos = Math::Cos(mira.parametro);
        referencial.sin = Math::Sin(mira.parametro);
        return referencial;
    }

    // Tenta o modo pedido quando ele depende do jogador.
    if (contexto.temJogador &&
        (mira.modo == Mira::MirarNoJogador || mira.modo == Mira::MirarPrevendo)) {

        Vector2 alvo = contexto.posicaoJogador;

        if (mira.modo == Mira::MirarPrevendo) {
            // Extrapola pela velocidade atual do jogador. Parado, a velocidade
            // e zero e isto vira a mira normal.
            alvo += contexto.velocidadeJogador * mira.parametro;
        }

        if (DirecaoParaRotacao(alvo - contexto.posicaoProjetil,
                               referencial.cos, referencial.sin)) {
            return referencial;
        }
        // Jogador exatamente em cima do projetil: cai para o proximo nivel.
    }

    // Fallback 1: alinhar com a direcao de voo.
    if (DirecaoParaRotacao(contexto.velocidadeProjetil,
                           referencial.cos, referencial.sin)) {
        return referencial;
    }

    // Fallback 2: identidade. A forma sai exatamente como foi escrita.
    referencial.cos = 1.0f;
    referencial.sin = 0.0f;
    return referencial;
}

Vector2 DirecionarPreservandoModulo(const Vector2& velocidadeAtual,
                                    const Vector2& direcaoDesejada,
                                    const float moduloMinimo) {

    const float comprimentoDirecao = direcaoDesejada.Length();
    if (Math::NearZero(comprimentoDirecao)) {
        // Nao ha para onde apontar: mantem o que ja existia.
        return velocidadeAtual;
    }

    float modulo = velocidadeAtual.Length();
    if (Math::NearZero(modulo)) {
        modulo = moduloMinimo;
    }

    return direcaoDesejada * (modulo / comprimentoDirecao);
}
