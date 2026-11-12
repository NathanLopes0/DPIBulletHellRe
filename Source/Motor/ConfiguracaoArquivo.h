//
// A ponte entre as opcoes do jogo e o disco.
//

#pragma once

#include "Configuracao.h"

/// Separado de Configuracao.cpp pelo mesmo motivo dos outros pares *Arquivo do projeto: aquele e
/// puro e entra no alvo de testes.
namespace ConfiguracaoArquivo {

    /// As opcoes gravadas, ou as padrao se nao houver arquivo. Nunca falha: arquivo ilegivel e
    /// linha estragada vao para o log e viram o padrao.
    Configuracao::Dados Carregar();

    /// Grava as opcoes. A gravacao e atomica, como a da ficha: escreve num temporario e so entao
    /// renomeia por cima do bom.
    bool Gravar(const Configuracao::Dados& dados);
}
