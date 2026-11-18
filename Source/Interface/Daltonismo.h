//
// O modo daltonismo: as cores do jogo corrigidas para quem confunde vermelho e verde (ou azul e
// amarelo).
//

#pragma once

namespace Daltonismo {

    /// Desligado, ou o tipo de daltonismo que a correcao compensa. A deuteranopia (o verde) e a
    /// protanopia (o vermelho) confundem o vermelho com o verde; a tritanopia, o azul com o amarelo.
    enum class Modo { Desligado, Deuteranopia, Protanopia, Tritanopia };
    constexpr int kModos = 4;

    struct Rgb {
        unsigned char r = 0, g = 0, b = 0;
    };

    /// O que o olho com este daltonismo ve de 'c' (as matrizes de Machado, Oliveira e Fernandes, 2009,
    /// na severidade total). Desligado, a propria cor.
    Rgb Simular(Rgb c, Modo m);

    /// A cor corrigida (daltonizacao): o que se perde da cor nesse olho e devolvido nos canais que
    /// ele ve, e as cores que ele confundia se separam. O cinza, que nao perde nada, fica cinza.
    Rgb Corrigir(Rgb c, Modo m);

    /// O nome na tela de opcoes, e no arquivo de opcoes.
    const char* Nome(Modo m);
    const char* NomeNoArquivo(Modo m);
    /// O modo de nome 'nome' no arquivo; false quando nao ha.
    bool DoArquivo(const char* nome, Modo& saida);

    /// O modo 'passo' casas depois, dando a volta: a opcao e uma lista curta.
    Modo Mover(Modo m, int passo);

    /// O modo em que o jogo desenha agora - um so, da maquina, como as outras opcoes. Quem pinta (as
    /// texturas, as linhas, as caixas, o texto) pergunta aqui.
    Modo Atual();
    void Definir(Modo m);
}
