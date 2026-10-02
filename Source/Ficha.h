//
// A ficha do aluno: a matricula mais o progresso dele, em texto.
//

#pragma once

#include <string>
#include <vector>

#include "Progresso.h"

/**
 * CAMADA PURA. Transforma ficha em texto e texto em ficha; nao abre arquivo, nao
 * sabe onde os saves moram e nao conhece SDL. Quem grava e Source/FichaArquivo.
 *
 * UM ARQUIVO POR ALUNO, e nao um arquivo com todos. Dois motivos:
 *   - um save corrompido perde UM aluno, nao a turma inteira;
 *   - gravar e so escrever o arquivo daquele aluno, sem ler, mesclar e reescrever
 *     o de todo mundo a cada fim de batalha.
 * O preco e que listar a turma (para o professor) vira uma varredura de pasta, o
 * que e barato e acontece uma vez.
 */
namespace Ficha {

    /// A versao do formato, gravada em todo arquivo.
    ///
    /// Custa um campo e compra a possibilidade de mudar o formato depois sem que
    /// o jogo quebre na cara de um aluno: a leitura pode olhar a versao e migrar.
    /// Um save sem este campo nunca existiu, entao a ausencia e erro.
    inline constexpr int kVersaoAtual = 1;

    struct Dados {
        std::string matricula;   ///< a forma CANONICA, ver Matricula.h
        Progresso progresso;
    };

    struct Lida {
        bool ok = false;
        Dados dados;

        /// Frases prontas dizendo o que esta errado. Vazio quando ok.
        std::vector<std::string> problemas;
    };

    /**
     * @brief A ficha como texto JSON.
     *
     * Formato estavel, para que um arquivo gravado hoje abra depois da defesa:
     *
     *     {
     *       "versao": 1,
     *       "matricula": "89384",
     *       "materias": [
     *         { "materia": 0, "recorde": 72.5, "retomada": 68 }
     *       ]
     *     }
     *
     * As materias saem em ordem, e nao na ordem de um mapa interno: assim dois
     * saves do mesmo estado sao o MESMO texto, e dá para comparar arquivos num
     * diff quando algo parecer errado.
     */
    std::string Serializar(const Dados& dados);

    /**
     * @brief Le o texto de uma ficha.
     *
     * Nunca lanca. Devolve ok = false com a lista de problemas quando o arquivo
     * nao serve - e nesse caso o jogo deve tratar o aluno como novo, nao parar:
     * um save estragado nao pode impedir alguem de jogar.
     *
     * Uma materia com problema e DESCARTADA e as outras continuam valendo. A nota
     * que vem fora de 0 a 100 nao e aceita calada: a batalha trava a nota nessa
     * faixa (Math::Clamp em Battle), entao um valor fora dela so pode vir de
     * arquivo editado a mao ou corrompido.
     */
    Lida Desserializar(const std::string& texto);
}
