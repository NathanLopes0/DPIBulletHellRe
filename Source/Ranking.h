//
// A ordem das melhores notas de uma materia.
//

#pragma once

#include <string>
#include <vector>

#include "Exportacao.h"

/**
 * CAMADA PURA. So a ordenacao; nao conhece disco, tela nem SDL.
 *
 * POR QUE ISTO EXISTE. A curva da nota foi feita inteira para que a nota ordene
 * um ranking de verdade - era o argumento contra todo mundo tirar 100. Mas ate
 * aqui o ranking so existia na planilha do professor, e quem joga no gabinete
 * nunca via onde estava.
 *
 * ELE LE AS MESMAS FICHAS QUE A PLANILHA, pelo mesmo tipo (Exportacao::FichaDeAluno),
 * entao nao ha duas ideias de "a nota de fulano" que possam discordar.
 */
namespace Ranking {

    struct Linha {
        int posicao = 1;            ///< 1 para o primeiro. Empate repete a posicao.
        std::string matricula;
        float nota = 0.0f;

        /// Quantas materias do curso o aluno jogou. SO TEM SENTIDO NO GERAL, e e
        /// o que torna a media legivel: 17,10 sozinho parece uma nota pessima,
        /// mas "2 de 11 jogadas, media 17,10" conta a historia certa. No ranking
        /// de uma materia vale 1 e nao e exibido.
        int materiasJogadas = 1;
    };

    /**
     * @brief As melhores notas de uma materia, da maior para a menor.
     *
     * QUEM NUNCA JOGOU A MATERIA NAO ENTRA. Uma linha com nota zero para cada
     * aluno que nunca abriu aquela fase encheria a tela de gente que nao
     * disputou - e empurraria para fora quem disputou.
     *
     * EMPATE REPETE A POSICAO e nao desempata por matricula: duas notas iguais
     * sao o mesmo resultado, e usar o numero da matricula como criterio faria o
     * aluno mais antigo ganhar de graca. A ordem entre empatados fica estavel
     * (pela matricula) so para a tela nao tremer entre duas aberturas.
     *
     * @param quantas Quantas linhas no maximo. Zero devolve todas.
     */
    std::vector<Linha> DaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                 int materia, size_t quantas = 0);

    /**
     * @brief A media do aluno sobre o CURSO INTEIRO, da maior para a menor.
     *
     * A media divide pelo numero total de materias do curso, e nao pelas que o
     * aluno jogou. A diferenca e o ranking inteiro:
     *
     *   Dividir pelas JOGADAS premiaria jogar pouco - quem fez uma materia e
     *   tirou 100 ficaria na frente de quem fez oito com 95 de media.
     *
     *   Dividir pelo CURSO faz o numero medir avanco: subir exige jogar mais
     *   materias E ir bem nelas. Quem esta no comeco aparece baixo, e isso e
     *   honesto - ele avancou pouco.
     *
     * O numero resultante NAO e uma nota, e nao deve ser lido como uma: com 11
     * materias, um aluno impecavel em 8 delas tem 72,7.
     *
     * Quem nao jogou nada nao entra, pela mesma razao de DaMateria.
     */
    std::vector<Linha> Geral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                             int quantasMaterias, size_t quantas = 0);

    /// @brief A posicao de uma matricula no ranking geral. Zero se ela nao jogou nada.
    int PosicaoNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                       int quantasMaterias, const std::string& matricula);

    /**
     * @brief Em que posicao esta uma matricula, na lista COMPLETA da materia.
     *
     * Devolve 0 quando ela nao jogou. Serve para mostrar "voce esta em 23o" a
     * quem nao aparece no topo - sem isso, o ranking so fala com quem ja esta
     * bem, que e justamente quem menos precisa de incentivo.
     */
    int PosicaoDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
                  int materia, const std::string& matricula);
}
