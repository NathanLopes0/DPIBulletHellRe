//
// A ponte entre as opcoes do jogo e o disco.
//

#pragma once

#include "Configuracao.h"

/**
 * Separado de Configuracao.cpp pelo mesmo motivo dos outros pares *Arquivo do
 * projeto: aquele e puro e entra no alvo de testes; este abre arquivo e
 * escreve log, entao fica de fora.
 *
 * ONDE MORA: <base>/Saves/opcoes.cfg (Caminhos::Save).
 */
namespace ConfiguracaoArquivo {

    /// As opcoes gravadas, ou as padrao se nao houver arquivo. NUNCA FALHA:
    /// arquivo ilegivel e linha estragada vao para o log e viram o padrao.
    Configuracao::Dados Carregar();

    /// Grava as opcoes. A gravacao e ATOMICA, como a da ficha: escreve num
    /// temporario e so entao renomeia por cima do bom. Devolve false se nao
    /// conseguiu - a opcao continua valendo ate o jogo fechar.
    bool Gravar(const Configuracao::Dados& dados);
}
