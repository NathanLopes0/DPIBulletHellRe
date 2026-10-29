//
// O modo de dificuldade: Normal ou Dificil.
//

#pragma once

#include <string>

/**
 * CAMADA PURA. Coberto por test_dificuldade.cpp.
 *
 * O NORMAL E O JOGO COMO ELE FOI MEDIDO: cada fase foi afinada para que o
 * piloto que persegue passe com folga e o parado reprove (Ferramentas/), e
 * nada aqui mexe nisso. O DIFICIL e um degrau acima, escolhido por quem joga
 * antes das fases (EscolhaDaDificuldade), para quem ja tira 100 no Normal: mais coisa para desviar - mais
 * tiros, mais depressa, menos tempo entre eles.
 *
 * O DEGRAU NAO E UM MULTIPLICADOR UNICO. Quase todo ataque de hoje e conduzido
 * pelo C++ do chefe, e o tempo dele e o conceito: a palavra do Ricardo chega na
 * porta que o placar anunciou, o pacote do Vitor chega ao roteador no RTT.
 * Acelerar todo projetil por igual quebraria essas contas. Por isso cada chefe
 * decide o proprio degrau, no modulo de cada fase, e pergunta aqui so QUAL
 * valor usar (Escolher).
 *
 * A nota vale o mesmo nos dois modos: o Dificil e para quem quer, e nao uma
 * exigencia do curso.
 */
namespace Dificuldade {

    enum class Nivel { Normal, Dificil };

    /// Os niveis, na ordem dos cartoes da tela de dificuldade.
    constexpr Nivel kNiveis[] = {Nivel::Normal, Nivel::Dificil};
    constexpr int kQuantos = static_cast<int>(sizeof(kNiveis) / sizeof(kNiveis[0]));

    /// Como o nivel se chama na tela, em maiusculas: "NORMAL", "DIFICIL" (com
    /// acento, em UTF-8 - a Zelda tem o I agudo).
    const char* Nome(Nivel n);

    /// A COR DO DIFICIL na tela (r, g, b de 0 a 1): o selo da batalha e o valor
    /// na tela de dificuldade sao a mesma coisa, e tem de se reconhecer. Um vermelho
    /// claro, que le como aviso e nao se confunde com o dourado do TESTE FINAL
    /// nem com o verde da FASE LIMPA.
    constexpr float kCorDoDificil[3] = {1.0f, 0.42f, 0.36f};

    /// Uma linha que diz o que o nivel muda, para a tela de dificuldade.
    const char* Descricao(Nivel n);

    /// Como o nivel e escrito nos dados (tentativas.csv) e no log: "normal",
    /// "dificil". Sem acento de proposito - um CSV com acento depende da
    /// codificacao de quem abre.
    const char* Chave(Nivel n);

    /// Anda `passo` niveis, dando a volta nas pontas - com dois niveis, para
    /// qualquer lado e trocar.
    Nivel Mover(Nivel n, int passo);

    /// O valor do nivel: `normal` no Normal, `dificil` no Dificil. E por aqui
    /// que cada chefe escreve o proprio degrau, ao lado do valor de sempre.
    template <typename T>
    T Escolher(const Nivel n, const T normal, const T dificil) {
        return n == Nivel::Dificil ? dificil : normal;
    }

    /**
     * @brief A INVENCIBILIDADE DEPOIS DE UM ACERTO, em segundos: o tempo em que
     * o aluno pisca e nada mais o acerta.
     *
     * NO NORMAL e 1,2 s, como sempre foi: tomar um tiro tira o aluno do
     * aperto. NO DIFICIL e um terco disso. Com 1,2 s, quem nao sabia desviar
     * atravessava a rajada piscando, tomava UM acerto onde merecia tres, e
     * passava assim mesmo; com 0,4 s, o tiro de tras do primeiro tambem
     * acerta. E O UNICO DEGRAU QUE NAO E DE UM CHEFE: vale para todos, e e o
     * que faz o Dificil cobrar desviar, e nao so aguentar.
     */
    constexpr float kInvencibilidadeNormal = 1.2f;
    constexpr float kInvencibilidadeDificil = 0.4f;
    inline float Invencibilidade(const Nivel n) {
        return Escolher(n, kInvencibilidadeNormal, kInvencibilidadeDificil);
    }
}
