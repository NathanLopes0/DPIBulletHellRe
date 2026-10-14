//
// A ficha do aluno: a matricula mais o progresso dele, em texto.
//

#pragma once

#include <string>
#include <vector>

#include "Materias.h"
#include "Personagens.h"
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
    /// A versao 2 grava o CODIGO da materia ("INF213") no lugar da posicao dela na
    /// lista. A 1 gravava a posicao, e por isso reordenar as materias trocava as
    /// notas de dona em silencio - um save da 1 ainda abre, convertido pela ordem
    /// atual de materias.json.
    /// A 3 acrescenta "quando": a data da ultima vez que o aluno jogou aquela
    /// materia. O campo e opcional, entao um save da 2 abre sem conversao nenhuma.
    /// A 4 acrescenta "aparencia": as escolhas de pele, cabelo, camisa e calca.
    /// Tambem opcional - um save da 3 abre e cai na aparencia padrao.
    inline constexpr int kVersaoAtual = 4;

    struct Dados {
        std::string matricula;   ///< a forma CANONICA, ver Matricula.h
        Progresso progresso;

        /// As escolhas de aparencia, COMO ESTAVAM NO ARQUIVO.
        ///
        /// Nao sao conferidas contra o catalogo aqui, de proposito: esta camada
        /// transforma texto em dado e nada mais. Uma peca que saiu do catalogo e
        /// tratada por Personagens::Catalogo::Resolver, na hora de compor - que
        /// e onde o catalogo existe. Assim a ficha nao precisa recebe-lo, e
        /// trocar o catalogo nao invalida save nenhum.
        Personagens::Aparencia aparencia;
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
     *         { "materia": "INF213", "recorde": 72.5, "retomada": 68 }
     *       ]
     *     }
     *
     * As materias saem em ordem, e nao na ordem de um mapa interno: assim dois
     * saves do mesmo estado sao o MESMO texto, e dá para comparar arquivos num
     * diff quando algo parecer errado.
     */
    std::string Serializar(const Dados& dados, const Materias::Lista& materias);

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
    Lida Desserializar(const std::string& texto, const Materias::Lista& materias);
}
