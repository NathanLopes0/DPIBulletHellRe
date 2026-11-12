//
// O modo de dificuldade: Normal ou Dificil.
//

#pragma once

#include <string>

/// O Dificil e um degrau acima, escolhido por quem joga antes das fases (EscolhaDaDificuldade),
/// para quem ja tira 100 no Normal.
namespace Dificuldade {

    enum class Nivel { Normal, Dificil };

    /// Os niveis, na ordem dos cartoes da tela de dificuldade.
    constexpr Nivel kNiveis[] = {Nivel::Normal, Nivel::Dificil};
    constexpr int kQuantos = static_cast<int>(sizeof(kNiveis) / sizeof(kNiveis[0]));

    /// Como o nivel se chama na tela, em maiusculas: "NORMAL", "DIFICIL" (com
    /// acento, em UTF-8).
    const char* Nome(Nivel n);

    /// A cor do Dificil na tela (r, g, b de 0 a 1): o selo da batalha e o valor na tela de
    /// dificuldade sao a mesma coisa, e tem de se reconhecer.
    constexpr float kCorDoDificil[3] = {1.0f, 0.42f, 0.36f};

    /// Uma linha que diz o que o nivel muda, para a tela de dificuldade.
    const char* Descricao(Nivel n);

    /// Como o nivel e escrito nos dados (tentativas.csv) e no log: "normal", "dificil". Sem acento
    /// - um CSV com acento depende da codificacao de quem abre.
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

    /// A invencibilidade depois de um acerto, em segundos: o tempo em que o aluno pisca e nada mais
    /// o acerta.
    constexpr float kInvencibilidadeNormal = 1.2f;
    constexpr float kInvencibilidadeDificil = 0.4f;
    inline float Invencibilidade(const Nivel n) {
        return Escolher(n, kInvencibilidadeNormal, kInvencibilidadeDificil);
    }
}
