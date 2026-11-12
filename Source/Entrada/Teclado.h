//
// O teclado digital da tela de matricula.
//

#pragma once

#include <string>
#include <vector>

#include "Navegacao.h"

/// So o desenho do teclado e o significado de cada tecla; nao conhece SDL, fonte, cena nem arquivo.
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

    /// O teclado, na ordem em que as teclas sao criadas.
    const std::vector<Tecla>& Teclas();

    /// A grade para Navegacao, derivada de Teclas(). Grade[coluna][linha] = indice em Teclas().
    Navegacao::Grade Grade();

    /// O indice da tecla que comeca selecionada.
    size_t Inicial();
}
