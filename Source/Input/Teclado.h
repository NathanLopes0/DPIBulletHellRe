//
// O teclado digital da tela de matricula.
//

#pragma once

#include <string>
#include <vector>

#include "Navegacao.h"

/**
 * CAMADA PURA. So o desenho do teclado e o significado de cada tecla; nao conhece
 * SDL, fonte, cena nem arquivo.
 *
 * POR QUE ISTO EXISTE. O gabinete nao tem teclado: so manche e botoes. A
 * matricula passa a ser digitada escolhendo numeros numa grade na tela.
 *
 * E POR QUE E UM MODULO, e nao um vetor dentro da cena: a grade da navegacao sai
 * DAQUI, derivada das mesmas teclas que a tela desenha. Quando a selecao de
 * fases tinha duas descricoes do mesmo layout - uma para desenhar e outra para
 * navegar - elas se descolaram em silencio e a seta passou a andar por uma tela
 * que nao existia mais. Aqui so ha uma.
 */
namespace Teclado {

    /// O que uma tecla faz quando apertada.
    enum class Acao {
        Digito,     ///< acrescenta `digito` ao que foi digitado
        Apagar,     ///< apaga o ultimo
        Entrar,     ///< valida e segue
        Visitante,  ///< joga sem salvar - ver Identificacao
        Voltar      ///< volta ao menu
    };

    struct Tecla {
        Acao acao = Acao::Digito;
        char digito = '0';      ///< so vale quando acao == Digito
        std::string rotulo;     ///< o que aparece escrito na tecla
        int coluna = 0;
        int linha = 0;
    };

    /**
     * @brief O teclado, na ordem em que as teclas sao criadas.
     *
     * O desenho e o de um telefone, com uma coluna de acoes a direita:
     *
     *     1  2  3  | APAGAR
     *     4  5  6  | ENTRAR
     *     7  8  9  | VISITANTE
     *        0     | VOLTAR
     *
     * O zero fica sozinho embaixo do 8, como num telefone. As colunas tem alturas
     * diferentes de proposito, e a navegacao lida com isso.
     */
    const std::vector<Tecla>& Teclas();

    /**
     * @brief A grade para Navegacao, derivada de Teclas().
     *
     * grade[coluna][linha] = indice em Teclas(). Nao ha layout escrito duas
     * vezes: mexer em Teclas() move a navegacao junto.
     */
    Navegacao::Grade Grade();

    /// @brief O indice da tecla que comeca selecionada. E o "1", canto superior
    /// esquerdo: e onde o olho cai, e e digito, que e o que quase todo mundo vem
    /// fazer aqui.
    size_t Inicial();
}
