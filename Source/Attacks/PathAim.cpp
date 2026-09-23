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

Vector2 AjustarModulo(const Vector2& velocidade, float novoModulo) {

    const float modulo = velocidade.Length();
    if (Math::NearZero(modulo)) {
        // Sem direcao para preservar. Devolver um vetor qualquer com o modulo
        // pedido seria escolher uma direcao no lugar de quem chamou.
        return velocidade;
    }

    if (novoModulo < 0.0f) novoModulo = 0.0f;

    return velocidade * (novoModulo / modulo);
}

FaseDoPulso CalcularFaseDoPulso(const float tempoDecorrido, const RitmoDeCiclos& ritmo,
                                const float moduloInvestida, const float moduloPausa) {

    FaseDoPulso fase;

    const float ciclo = ritmo.Ciclo();
    if (ritmo.repeticoes <= 0 || ciclo <= 0.0f || tempoDecorrido < ritmo.atraso) {
        fase.ciclo = (ritmo.repeticoes <= 0 || ciclo <= 0.0f) ? ritmo.repeticoes : 0;
        return fase;
    }

    const float t = tempoDecorrido - ritmo.atraso;
    fase.ciclo = static_cast<int>(t / ciclo);

    if (fase.ciclo >= ritmo.repeticoes) {
        fase.ciclo = ritmo.repeticoes;
        return fase;
    }

    const float dentroDoCiclo = t - static_cast<float>(fase.ciclo) * ciclo;
    const bool investindo = dentroDoCiclo < ritmo.duracaoInvestida;

    // A sequencia TERMINA NUMA INVESTIDA, nao numa pausa: sao N investidas com
    // N-1 pausas entre elas. Terminar na pausa deixava o projetil a poucos
    // px/s, lento demais para sair da tela dentro da batalha.
    if (!investindo && fase.ciclo == ritmo.repeticoes - 1) {
        fase.ciclo = ritmo.repeticoes;
        return fase;
    }

    fase.ativo = true;
    fase.investindo = investindo;
    fase.modulo = investindo ? moduloInvestida : moduloPausa;
    return fase;
}
