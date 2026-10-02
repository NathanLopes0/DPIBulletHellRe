//
// A ponte entre a ficha do aluno e o disco.
//

#pragma once

#include <string>
#include <vector>

#include "Progresso.h"

/**
 * Separado de Ficha.cpp pelo mesmo motivo dos outros pares do projeto: aquele e
 * puro e entra no alvo de testes; este abre arquivo, escreve log e conhece a
 * pasta de saves, entao fica de fora.
 *
 * ONDE OS SAVES FICAM: <base>/Saves/<matricula>.json, um por aluno. A pasta e
 * criada por Caminhos::Save, ao lado de Assets.
 */
namespace FichaArquivo {

    /**
     * @brief Carrega o progresso de uma matricula.
     *
     * NUNCA FALHA do ponto de vista de quem chama: aluno sem arquivo, arquivo
     * ilegivel e arquivo corrompido devolvem todos um Progresso vazio, com o
     * motivo no log. Um save estragado nao pode impedir alguem de jogar - e no dia
     * da apresentacao isso seria um aluno parado na frente da maquina.
     *
     * Devolve o progresso por valor. Quem chama decide onde ele vive.
     */
    Progresso Carregar(const std::string& matricula);

    /**
     * @brief Grava o progresso de uma matricula. Devolve false se nao conseguiu.
     *
     * A gravacao e ATOMICA: escreve num arquivo temporario e so entao renomeia por
     * cima do bom. Sem isso, fechar o jogo no meio da escrita deixaria um save pela
     * metade - que e pior que save nenhum, porque o aluno so descobre depois.
     *
     * Recusa matricula que nao passa em Matricula::Validar. E a unica barreira
     * entre texto digitado e um nome de arquivo, entao ela e obrigatoria mesmo que
     * a tela ja valide: quem chama pode mudar.
     */
    bool Gravar(const std::string& matricula, const Progresso& progresso);

    /// @brief Se ja existe ficha para esta matricula.
    bool Existe(const std::string& matricula);

    /**
     * @brief Todas as matriculas com ficha gravada, em ordem.
     *
     * Para a exportacao do professor, e para o ranking em tela quando ele existir.
     * Varre a pasta de saves; e barato e acontece uma vez.
     */
    std::vector<std::string> ListarMatriculas();
}
